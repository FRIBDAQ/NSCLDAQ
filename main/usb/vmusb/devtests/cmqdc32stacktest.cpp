
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <CLoggingReadoutList.h>
#include <CMockVMUSB.h>
#include <string>
#include <vector>
#include <iostream>
#include <iterator>

using namespace std;

#define private public
#define protected public
#include <CMQDC32StackBuilder.h>
#undef protected
#undef private


// Fool the linker
//namespace Globals {
//  ::CConfiguration* pConfig;
//}


class cmqdc32stacktest : public CppUnit::TestFixture {
  private:
    MQDC32::CMQDC32StackBuilder m_module;
  public:
  CPPUNIT_TEST_SUITE(cmqdc32stacktest);
  CPPUNIT_TEST( resetAll_0 );
  CPPUNIT_TEST( addWriteModuleID_0 );
  CPPUNIT_TEST( addWriteThreshold_0 );
  CPPUNIT_TEST( addWriteThresholds_0 );
  CPPUNIT_TEST( addWriteIgnoreThresholds_0 );
  CPPUNIT_TEST( addWriteMarkerType_0 );
  CPPUNIT_TEST( addWriteMemoryBankSeparation_0 );
  CPPUNIT_TEST( addWriteGateLimit0_0 );
  CPPUNIT_TEST( addWriteGateLimit1_0 );
  CPPUNIT_TEST( addWriteGateLimits_0 );
  CPPUNIT_TEST( addWriteExpTrigDelay0_0 );
  CPPUNIT_TEST( addWriteExpTrigDelay1_0 );
  CPPUNIT_TEST( addWriteExpTrigDelays_0 );
  CPPUNIT_TEST( addWriteBankOffsets_0 );
  CPPUNIT_TEST( addWritePulserState_0 );
  CPPUNIT_TEST( addWritePulserAmplitude_0 );
  CPPUNIT_TEST( addWriteInputCoupling_0 );
  CPPUNIT_TEST( addWriteTimeDivisor_0 );
  CPPUNIT_TEST( addResetTimestamps_0 );
  CPPUNIT_TEST( addWriteECLTermination_0 );
  CPPUNIT_TEST( addWriteECLGate1Input_0 );
  CPPUNIT_TEST( addWriteECLFCInput_0 );
  CPPUNIT_TEST( addWriteNIMGate1Input_0 );
  CPPUNIT_TEST( addWriteNIMFCInput_0 );
  CPPUNIT_TEST( addWriteNIMBusyOutput_0 );
  CPPUNIT_TEST( addWriteTimeBaseSource_0 );
  CPPUNIT_TEST( addWriteMultiEventMode_0 );
  CPPUNIT_TEST( addWriteTransferCount_0 );
  CPPUNIT_TEST( addDisableInterrupts_0 );
  CPPUNIT_TEST( addWriteWithdrawIrqOnEmpty_0 );
  CPPUNIT_TEST( addWriteIrqLevel_0 );
  CPPUNIT_TEST( addWriteIrqVector_0 );
  CPPUNIT_TEST( addWriteIrqThreshold_0 );
  CPPUNIT_TEST( addInitializeFifo_0 );
  CPPUNIT_TEST( addWriteLowerMultLimits_0 );
  CPPUNIT_TEST( addWriteUpperMultLimits_0 );
  CPPUNIT_TEST( addWriteCounterReset_0 );
  CPPUNIT_TEST( addFifoRead_0 );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
    m_module = MQDC32::CMQDC32StackBuilder();
    m_module.setBase(0x40000000);
  }
  void tearDown() {
  }

protected:
  void resetAll_0();
  void addWriteModuleID_0();
  void addWriteThreshold_0();
  void addWriteThresholds_0();
  void addWriteIgnoreThresholds_0();
  void addWriteMarkerType_0();
  void addWriteMemoryBankSeparation_0();
  void addWriteGateLimit0_0();
  void addWriteGateLimit1_0();
  void addWriteGateLimits_0();
  void addWriteExpTrigDelay0_0();
  void addWriteExpTrigDelay1_0();
  void addWriteExpTrigDelays_0();
  void addWriteBankOffsets_0();
  void addWritePulserState_0();
  void addWritePulserAmplitude_0();
  void addWriteInputCoupling_0();
  void addWriteTimeDivisor_0();
  void addResetTimestamps_0();
  void addWriteECLTermination_0();
  void addWriteECLGate1Input_0();
  void addWriteECLFCInput_0();
  void addWriteNIMGate1Input_0();
  void addWriteNIMFCInput_0();
  void addWriteNIMBusyOutput_0();
  void addWriteTimeBaseSource_0();
  void addWriteMultiEventMode_0();
  void addWriteTransferCount_0();
  void addDisableInterrupts_0();
  void addWriteWithdrawIrqOnEmpty_0();
  void addWriteIrqLevel_0();
  void addWriteIrqVector_0();
  void addWriteIrqThreshold_0();
  void addInitializeFifo_0();
  void addWriteLowerMultLimits_0();
  void addWriteUpperMultLimits_0();
  void addWriteCounterReset_0();
  void addFifoRead_0();
};

// Utility function to print two vectors 
template<class T>
void print_vectors(const vector<T>& expected, const vector<T>& actual) {
  cout.flags(ios::hex);

  copy(expected.begin(), expected.end(), ostream_iterator<T>(cout,"\n"));
  cout << "---" << endl;
  copy(actual.begin(), actual.end(), ostream_iterator<T>(cout,"\n"));

  cout.flags(ios::dec);
}

CPPUNIT_TEST_SUITE_REGISTRATION(cmqdc32stacktest);

 
void cmqdc32stacktest::resetAll_0()
{
  CMockVMUSB ctlr;
  m_module.resetAll(ctlr);

  vector<string> expected = {
    "executeList::begin",
    "addWrite16 40006008 09 1",
    "executeList::end",
    "executeList::begin",
    "addWrite16 4000603a 09 0",
    "addWrite16 40006034 09 0",
    "executeList::end"
  };

  auto record =  ctlr.getOperationRecord();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}
void cmqdc32stacktest::addWriteModuleID_0()
{
  CLoggingReadoutList list;
  m_module.addWriteModuleID(list, 3);

  vector<string> expected = {"addWrite16 40006004 09 3",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteThreshold_0()
{
  CLoggingReadoutList list;
  m_module.addWriteThreshold(list, 3, 200);

  vector<string> expected = {"addWrite16 40004006 09 200",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteThresholds_0()
{
  CLoggingReadoutList list;
  m_module.addWriteThresholds(list, vector<long int>(32,2));

  vector<string> expected = {
    "addWrite16 40004000 09 2",
    "addDelay 1",
    "addWrite16 40004002 09 2",
    "addDelay 1",
    "addWrite16 40004004 09 2",
    "addDelay 1",
    "addWrite16 40004006 09 2",
    "addDelay 1",
    "addWrite16 40004008 09 2",
    "addDelay 1",
    "addWrite16 4000400a 09 2",
    "addDelay 1",
    "addWrite16 4000400c 09 2",
    "addDelay 1",
    "addWrite16 4000400e 09 2",
    "addDelay 1",
    "addWrite16 40004010 09 2",
    "addDelay 1",
    "addWrite16 40004012 09 2",
    "addDelay 1",
    "addWrite16 40004014 09 2",
    "addDelay 1",
    "addWrite16 40004016 09 2",
    "addDelay 1",
    "addWrite16 40004018 09 2",
    "addDelay 1",
    "addWrite16 4000401a 09 2",
    "addDelay 1",
    "addWrite16 4000401c 09 2",
    "addDelay 1",
    "addWrite16 4000401e 09 2",
    "addDelay 1",
    "addWrite16 40004020 09 2",
    "addDelay 1",
    "addWrite16 40004022 09 2",
    "addDelay 1",
    "addWrite16 40004024 09 2",
    "addDelay 1",
    "addWrite16 40004026 09 2",
    "addDelay 1",
    "addWrite16 40004028 09 2",
    "addDelay 1",
    "addWrite16 4000402a 09 2",
    "addDelay 1",
    "addWrite16 4000402c 09 2",
    "addDelay 1",
    "addWrite16 4000402e 09 2",
    "addDelay 1",
    "addWrite16 40004030 09 2",
    "addDelay 1",
    "addWrite16 40004032 09 2",
    "addDelay 1",
    "addWrite16 40004034 09 2",
    "addDelay 1",
    "addWrite16 40004036 09 2",
    "addDelay 1",
    "addWrite16 40004038 09 2",
    "addDelay 1",
    "addWrite16 4000403a 09 2",
    "addDelay 1",
    "addWrite16 4000403c 09 2",
    "addDelay 1",
    "addWrite16 4000403e 09 2",
    "addDelay 1",
  };

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteIgnoreThresholds_0()
{
  CLoggingReadoutList list;
  m_module.addWriteIgnoreThresholds(list, 0);

  vector<string> expected = {"addWrite16 4000604c 09 0",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteMarkerType_0()
{
  CLoggingReadoutList list;
  m_module.addWriteMarkerType(list, 3);

  vector<string> expected = {"addWrite16 40006038 09 3",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteMemoryBankSeparation_0()
{
  CLoggingReadoutList list;
  m_module.addWriteMemoryBankSeparation(list, 4);

  vector<string> expected = {"addWrite16 40006040 09 4",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteGateLimit0_0() 
{
  CLoggingReadoutList list;
  m_module.addWriteGateLimit0(list, 21);

  vector<string> expected = {"addWrite16 40006050 09 21",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);

}


void cmqdc32stacktest::addWriteGateLimit1_0() 
{
  CLoggingReadoutList list;
  m_module.addWriteGateLimit1(list, 255);

  vector<string> expected = {"addWrite16 40006052 09 255",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);

}

void cmqdc32stacktest::addWriteGateLimits_0() 
{
  CLoggingReadoutList list;
  m_module.addWriteGateLimits(list, {234, 255});

  vector<string> expected = {"addWrite16 40006050 09 234",
  "addDelay 1",
  "addWrite16 40006052 09 255",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);

}

void cmqdc32stacktest::addWriteExpTrigDelay0_0() 
{
  CLoggingReadoutList list;
  m_module.addWriteExpTrigDelay0(list, 2000);

  vector<string> expected = {"addWrite16 40006054 09 2000",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);

}


void cmqdc32stacktest::addWriteExpTrigDelay1_0() 
{
  CLoggingReadoutList list;
  m_module.addWriteExpTrigDelay1(list, 2001);

  vector<string> expected = {"addWrite16 40006056 09 2001",
    "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);

}

void cmqdc32stacktest::addWriteExpTrigDelays_0() 
{
  CLoggingReadoutList list;
  m_module.addWriteExpTrigDelays(list, {3094, 5903});

  vector<string> expected = {"addWrite16 40006054 09 3094",
  "addDelay 1",
  "addWrite16 40006056 09 5903",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);

}

void cmqdc32stacktest::addWriteBankOffsets_0() 
{
  CLoggingReadoutList list;
  m_module.addWriteBankOffsets(list, {1, 234});

  vector<string> expected = {"addWrite16 40006044 09 1",
  "addDelay 1",
  "addWrite16 40006046 09 234",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);

}


void cmqdc32stacktest::addWritePulserState_0() {
  CLoggingReadoutList list;
  m_module.addWritePulserState(list, 5);

  vector<string> expected = {"addWrite16 40006070 09 5",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);

}

void cmqdc32stacktest::addWritePulserAmplitude_0() {
  CLoggingReadoutList list;
  m_module.addWritePulserAmplitude(list, 234);

  vector<string> expected = {"addWrite16 40006072 09 234",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);

}

void cmqdc32stacktest::addWriteInputCoupling_0() {
  CLoggingReadoutList list;
  m_module.addWriteInputCoupling(list, 7);

  vector<string> expected = {"addWrite16 40006060 09 7",
  "addDelay 1"};

  auto record =  list.getLog();
 // print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}


void cmqdc32stacktest::addWriteTimeDivisor_0() {
  CLoggingReadoutList list;
  m_module.addWriteTimeDivisor(list, 200);

  vector<string> expected = {"addWrite16 40006098 09 200",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addResetTimestamps_0() {
  CLoggingReadoutList list;
  m_module.addResetTimestamps(list);

  vector<string> expected = {"addWrite16 40006090 09 3",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteECLTermination_0() {
  CLoggingReadoutList list;
  m_module.addWriteECLTermination(list,0x4);

  vector<string> expected = {"addWrite16 40006062 09 4",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteECLGate1Input_0() {
  CLoggingReadoutList list;
  m_module.addWriteECLGate1Input(list,0);

  vector<string> expected = {"addWrite16 40006064 09 0",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteECLFCInput_0() {
  CLoggingReadoutList list;
  m_module.addWriteECLFCInput(list,2);

  vector<string> expected = {"addWrite16 40006066 09 2",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}


void cmqdc32stacktest::addWriteNIMGate1Input_0() {
  CLoggingReadoutList list;
  m_module.addWriteNIMGate1Input(list,0);

  vector<string> expected = {"addWrite16 4000606a 09 0",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteNIMFCInput_0() {
  CLoggingReadoutList list;
  m_module.addWriteNIMFCInput(list,2);

  vector<string> expected = {"addWrite16 4000606c 09 2",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteNIMBusyOutput_0() {
  CLoggingReadoutList list;
  m_module.addWriteNIMBusyOutput(list,3);

  vector<string> expected = {"addWrite16 4000606e 09 3",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteTimeBaseSource_0() {
  CLoggingReadoutList list;
  m_module.addWriteTimeBaseSource(list,2);

  vector<string> expected = {"addWrite16 40006096 09 2",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteMultiEventMode_0() {
  CLoggingReadoutList list;
  m_module.addWriteMultiEventMode(list,3);

  vector<string> expected = {"addWrite16 40006036 09 3",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteTransferCount_0() {
  CLoggingReadoutList list;
  m_module.addWriteTransferCount(list,123);

  vector<string> expected = {"addWrite16 4000601a 09 123",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addDisableInterrupts_0() {
  CLoggingReadoutList list;
  m_module.addDisableInterrupts(list);

  vector<string> expected = {"addWrite16 40006010 09 0",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteWithdrawIrqOnEmpty_0() {
  CLoggingReadoutList list;
  m_module.addWriteWithdrawIrqOnEmpty(list, true);

  vector<string> expected = {"addWrite16 4000601c 09 1",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteIrqLevel_0() {
  CLoggingReadoutList list;
  m_module.addWriteIrqLevel(list, 7);

  vector<string> expected = {"addWrite16 40006010 09 7",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteIrqVector_0() {
  CLoggingReadoutList list;
  m_module.addWriteIrqVector(list, 233);

  vector<string> expected = {"addWrite16 40006012 09 233",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteIrqThreshold_0() {
  CLoggingReadoutList list;
  m_module.addWriteIrqThreshold(list, 233);

  vector<string> expected = {"addWrite16 40006018 09 233",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addInitializeFifo_0() {
  CLoggingReadoutList list;
  m_module.addInitializeFifo(list);

  vector<string> expected = {"addWrite16 4000603c 09 1",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteLowerMultLimits_0() {
  CLoggingReadoutList list;
  m_module.addWriteLowerMultLimits(list, {22, 111});

  vector<string> expected = {"addWrite16 400060b2 09 22",
  "addDelay 1",
  "addWrite16 400060b6 09 111",
  "addDelay 1"};
  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteUpperMultLimits_0() {
  CLoggingReadoutList list;
  m_module.addWriteUpperMultLimits(list, {22, 111});

  vector<string> expected = {"addWrite16 400060b0 09 22",
  "addDelay 1",
  "addWrite16 400060b4 09 111",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

void cmqdc32stacktest::addWriteCounterReset_0() {
  CLoggingReadoutList list;
  m_module.addWriteCounterReset(list,2);

  vector<string> expected = {"addWrite16 40006090 09 2",
  "addDelay 1"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}


void cmqdc32stacktest::addFifoRead_0() {
  CLoggingReadoutList list;
  m_module.addFifoRead(list, 45);

  vector<string> expected = {"addFifoRead32 40000000 0b 45"};

  auto record =  list.getLog();
//  print_vectors(expected, record);

  CPPUNIT_ASSERT(expected == record);
}

