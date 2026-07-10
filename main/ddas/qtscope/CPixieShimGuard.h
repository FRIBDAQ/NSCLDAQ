/**
 * @file CPixieShimGuard.h
 * @brief Catch-all guards for the extern "C" ctypes shims. Nothing may
 * throw across an extern "C" frame into ctypes: expected failures are
 * handled inside the class methods; these handle anything unexpected.
 */

#ifndef CPIXIESHIMGUARD_H
#define CPIXIESHIMGUARD_H

#include <iostream>

/**
 * @addtogroup utilities libPixieUtilities.so
 * @{
 */

/** Returned by int shims when on unexpected errors. QtScope */
constexpr int SHIM_UNEXPECTED_ERROR = -1999;

/**
 * @brief Run fn() inside the boundary net; return fallback on any exception.
 * T must provide SetLastErrorMessage().
 * @param utils Pointer to the utility class instance.
 * @param where String indicating the location of the error.
 * @param fallback Value to return on failure.
 * @param fn Function to execute within the guard.
 * @return Result of the function or fallback value.
 */
template <typename T, typename R, typename Fn>
R shimGuard(T *utils, const char *where, R fallback, Fn fn) {
  try {
    utils->SetLastErrorMessage(""); // Clear any previous error message
    return fn();
  } catch (const std::exception &e) {
    utils->SetLastErrorMessage(e.what());
    std::cerr << where << " unexpected std::exception: " << e.what()
              << std::endl;
    return fallback;
  } catch (...) {
    utils->SetLastErrorMessage("unknown exception");
    std::cerr << where << " unknown exception" << std::endl;
    return fallback;
  }
}

/**
 * @brief Same net for void shims.
 * @param utils Pointer to the utility class instance.
 * @param where String indicating the location of the error.
 * @param fn Function to execute within the guard.
 */
template <typename T, typename Fn>
void shimGuardVoid(T *utils, const char *where, Fn fn) {
  try {
    utils->SetLastErrorMessage(""); // Clear any previous error message
    fn();
  } catch (const std::exception &e) {
    utils->SetLastErrorMessage(e.what());
    std::cerr << where << " unexpected std::exception: " << e.what()
              << std::endl;
  } catch (...) {
    utils->SetLastErrorMessage("unknown exception");
    std::cerr << where << " unknown exception" << std::endl;
  }
}

/**
 * @brief Net for the constructors: no object exists yet to store a message in,
 * so log and return nullptr (the Python __init__ must check).
 * @param where String indicating the location of the error.
 * @param fn Function to execute within the guard.
 * @return Result of the function or nullptr.
 */
template <typename Fn>
auto shimGuardNew(const char *where, Fn fn) -> decltype(fn()) {
  try {
    return fn();
  } catch (const std::exception &e) {
    std::cerr << where
              << " construction failed: unhandled std::exception: " << e.what()
              << std::endl;
    return nullptr;
  } catch (...) {
    std::cerr << where << " construction failed: unknown exception"
              << std::endl;
    return nullptr;
  }
}

/** @} */

#endif // CPIXIESHIMGUARD_H