/**
 * @file CPixieErrors.h
 * @brief Defines error codes for the ctypes interface used by the *Utilities
 * classes to report errors which are handled internally. As of XIA API 5.1.0,
 * XIA error codes are > -1000, so QtScope error codes are < -1000 to avoid
 * conflicts.
 * @note The error code for unexpected errors in the ctype shims is included in
 * its own header file CPixieShimGuard.h.
 */

#ifndef CPIXIEERRORS_H
#define CPIXIEERRORS_H

/**
 * @addtogroup utilities libPixieUtilities.so
 * @{
 */

/** Invalid configuration: ConfigurationParser cannot generate valid config. */
constexpr int CPIXIEERROR_INVALID_CONFIG = -1000;
/** System not booted: Attempted to perform an operation that requires the
 * system to be booted. */
constexpr int CPIXIEERROR_NOT_BOOTED = -1001;
/** Invalid module number: Attempted to perform an operation on an invalid
 * module. */
constexpr int CPIXIEERROR_INVALID_MODULE = -1002;
/** Error during trace acquisition: trace length cannot be read from module,
 * trace is empty, or generator returns empty trace. */
constexpr int CPIXIEERROR_TRACE_ACQUIRE = -1003;

/** @} */

#endif // CPIXIEERRORS_H