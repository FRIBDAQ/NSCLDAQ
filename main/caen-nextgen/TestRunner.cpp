#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <string>
#include <iostream>
#include <cstdlib>

using namespace std;
void usage(const char* name)
{
  std::cerr<< "Usage:\n";
  std::cerr << "  " << name << "  pid-or-ip 0|1\n";
  std::cerr <<"Where:\n";
  std::cerr << "    pid-or-ip is either the USB PID or ip address to use\n";
  std::cerr << "    0 - for ethernt (IP address needed) or 1 - for USB (PID needed)\n";
  
}

std::string connection;
bool        isUsb;

int main(int argc, char** argv)
{
  // Pull out the connection parameters which are either
  //  PID 1
  // or
  //  ip.dotted.addr.ees 0
  //
  if (argc !=  3) {
    usage(argv[0]);
  }
  connection = argv[1];
  int usb    = std::atoi(argv[2]);
  isUsb      = usb ? true : false;
  
  std::cout << "Connecting via " << (isUsb ? "USB" : "ETHERNET") << " to " << connection
    << std::endl;
  
  CppUnit::TextTestRunner
               runner; // Control tests.
  CppUnit::TestFactoryRegistry&
               registry(CppUnit::TestFactoryRegistry::getRegistry());

  runner.addTest(registry.makeTest());

  bool wasSucessful;
  try {
    wasSucessful = runner.run("",false);
  }
  catch(string& rFailure) {
    cerr << "Caught a string exception from test suites.: \n";
    cerr << rFailure << endl;
    wasSucessful = false;
  }
  return !wasSucessful;
}
