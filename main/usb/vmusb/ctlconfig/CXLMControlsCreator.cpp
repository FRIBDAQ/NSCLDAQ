
#include <string>
#include <CXLMControls.h>
#include <CXLMControlsCreator.h>
#include <memory>

namespace XLM
{

///////////////////////////////////////////////////////////////////////////////
/////////////////////// CXLMControlsCreator ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


  CControlHardware* CXLMControlsCreator::operator()()
  {
    return (new CXLMControls);
  }


}
