
#include <string>
#include <CXLMControls.h>
#include <CXLMControlsCreator.h>

namespace XLM
{

///////////////////////////////////////////////////////////////////////////////
/////////////////////// CXLMControlsCreator ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CXLMControls* CXLMControlsCreator::operator()()
{
  return new CXLMControls();
}


}
