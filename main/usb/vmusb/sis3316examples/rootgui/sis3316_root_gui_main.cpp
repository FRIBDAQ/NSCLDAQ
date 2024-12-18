//
/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_root_gui_main.cpp                                    */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 27.03.2015                                       */
/*  last modification:    08.03.2024 (SIS3316-2 adaptation)                */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*                                                                         */
/*  https://www.struck.de                                                  */
/*                                                                         */
/*  � 2024                                                                 */
/*                                                                         */
/***************************************************************************/

#include "project_system_define.h"		//define LINUX or WINDOWS
#include "project_interface_define.h"   //define Interface (sis1100/sis310x, sis3150usb or Ethnernet UDP)

#include "sis3316_root_gui_monitor_size.h"

// default
#ifdef ETHERNET_UDP_INTERFACE
const char* default_ip_addr = "sis3316-0040";
//const char* default_ip_addr = "212.60.16.200";
#else
const char* default_ip_addr = "sis3153-0015";
#endif

#define FIRST_MODULE_BASE_ADDR						0x41000000


#define TestMainFrame_kCM_kCM_BUTTON_IRQ_NO_10		10
#define TestMainFrame_kCM_CHECKBUTTON_IRQ_NO_10		10
#define TestMainFrame_kCM_ENTRY_IRQ_NO_10		    10  
#define TestMainFrame_kCM_ENTRY_IRQ_NO_11		    11  


////#include "rootIncludes.h"
#include "sis3316_root_gui_test1.h"		 
#include "sis3316_root_gui_flash.h"
#include "sis3316_root_gui_register_access.h"
#include "sis3316_root_gui_udp_properties.h"
#include "sis3316.h"

#include "rootIncludes.h"
//#include "sis3316_root_gui_main.h"

using namespace std;



#ifdef WINDOWS
	#pragma comment (lib, "libGui")
	#pragma comment (lib, "libCore")
//	#pragma comment (lib, "libCint")   remove with root_v6.xx.xx
	#pragma comment (lib, "libRIO")
	#pragma comment (lib, "libNet")
	#pragma comment (lib, "libHist")
	#pragma comment (lib, "libGraf")
	#pragma comment (lib, "libGraf3d")
	#pragma comment (lib, "libGpad")
	#pragma comment (lib, "libTree")
	#pragma comment (lib, "libRint")
	#pragma comment (lib, "libPostscript")
	#pragma comment (lib, "libMatrix")
	#pragma comment (lib, "libPhysics")
	#pragma comment (lib, "libMathCore")
	#pragma comment (lib, "libThread")
//	#pragma comment (lib, "liblistDict")

/*****************************************************************************************************/

	#include <iostream>
	#include <iomanip>
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <winsock2.h>

	#include <stdlib.h>
	#include <string.h>
	#include <math.h>
	#include "wingetopt.h"

#endif



// choose Interface
#include "vme_interface_class.h"

#ifdef PCI_VME_INTERFACE
	#ifdef LINUX
		#include "sis1100linux_vme_class.h"
	#endif
	#ifdef WINDOWS
		#include "sis1100w_vme_class.h"
	#endif
sis1100 *gl_vme_crate ;
#endif

#ifdef USB_VME_INTERFACE
	#ifdef LINUX
		#include "sis315x_usb_inux_vme_class.h"
		sis315x *gl_vme_crate ;
	#endif
	#ifdef WINDOWS
		#include "sis3150w_vme_class.h"
		sis3150 *gl_vme_crate ;
	#endif
#endif


#ifdef USB3_VME_INTERFACE
	#include "sis3153w_vme_class.h"
	sis3153 *gl_vme_crate ;
#endif


#ifdef ETHERNET_UDP_INTERFACE
	#include "sis3316_ethernet_access_class.h"
	sis3316_eth *gl_vme_crate ;
	char  gl_sis3316_ip_addr_string[32] ;
	int gl_int_jumbo_frame_flag;
	unsigned int gl_uint_NofPackagesPerRequest;

	#ifdef LINUX
		#include <sys/types.h>
		#include <sys/socket.h>
	#endif

	#ifdef WINDOWS
	#include <winsock2.h>
	#pragma comment(lib, "ws2_32.lib")
	//#pragma comment(lib, "wsock32.lib")
	typedef int socklen_t;

	long WinsockStartup()
	{
	  long rc;

	  WORD wVersionRequested;
	  WSADATA wsaData;
	  wVersionRequested = MAKEWORD(2, 1);

	  rc = WSAStartup( wVersionRequested, &wsaData );
	  return rc;
	}
	#endif

#endif

#ifdef ETHERNET_VME_INTERFACE

	#define SIS31353_JUMBO_FRAME_ENABLE

	#include "sis3153ETH_vme_class.h"
	sis3153eth *gl_vme_crate;
//	char  gl_sis3153_ip_addr_string[32] = "212.60.16.206";
	char  gl_sis3153_ip_addr_string[32] = "192.168.1.11";

	#ifdef LINUX
		#include <sys/types.h>
		#include <sys/socket.h>
	#endif

	#ifdef WINDOWS
		#include <winsock2.h>
		#pragma comment(lib, "ws2_32.lib")
		typedef int socklen_t;
	#endif
#endif


		//unsigned int gl_module_base_addr = FIRST_MODULE_BASE_ADDR   ;

#include "sis3316_class.h"

#ifdef VMUSB_INTERFACE
#include <CVMUSB.h>
#include <CVMUSBFactory.h>
#include <string>
#include <sis_vmusb_interface.h>
namespace Globals {
        CVMUSB* pUSBController(0);
}
static vme_interface_class* vme_crate(0);
vme_interface_class* gl_vme_crate(0);    // *sigh*
// Create the VMUSB Interface and an sis_vme_interface

// instance based on it.  Return 0 on succdess.
static int createInterface() {
        try {
                Globals::pUSBController = CVMUSBFactory::createUSBController(
					CVMUSBFactory::local, nullptr
				);

                vme_crate = new sis_vmusb_interface;
                vme_crate->vmeopen();
        } catch (std::string msg) {
                std::cerr << "Failed to open the VUSB:" 
					<< msg << std::endl;
                return -1;
        }
        return 0;
}
// True if the module at the base address has a readable
// module id and the id indicates this is a 3316:
static bool isSIS3316(vme_interface_class& vme, uint32_t base) {
        UINT idreg;
        int status = vme.vme_A32D32_read(base+0x04, &idreg);

        // If not status == 0 then it's not a module.

        if (status) return false;

        idreg = (idreg & 0xffff0000) >> 16;
        return idreg == 0x3316;
}
#endif

sis3316_adc* gl_sis3316_adc1;





//#ifdef testraus

enum ETestCommandIdentifiers {
   M_FILE_EXIT,
   M_SIS3316TEST1_DLG,
   M_FPGA_PROG_MENUE,
   M_REGISTER_ACCESS,
   M_UDP_DHCP_MENUE,
   M_HELP_ABOUT,
};



class TestMainFrame  : public TGMainFrame {


private:
   TGCompositeFrame    *main_frameh1;
   TGGroupFrame        *main_frameh1_fGrp[5];
   
   TGHorizontalFrame *main_frameh1_fGrp4_fHor1, *main_frameh1_fGrp4_fHor2;
   TGHorizontalFrame *main_framehsub, *main_framehsub0;
   TGLabel *fLabel_main_frameh1[8] ;
   TGLabel *fLabel_main_frameh2[6] ;

   TGCompositeFrame   *fStatusFrame;
   TGCanvas           *fCanvasWindow;
   TGTextEntry        *fTestText;
   TGTextBuffer        *fTbmsg;

   TGButton           *fTestButton;
   TGColorSelect      *fColorSel;

   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuSIS3316Test, *fMenuFpgaProgram, *fMenuRegisterAccess, *fMenuUdpDhcp, *fMenuHelp;
   TGLayoutHints      *fMenuBarLayout, *fMenuBarItemLayout, *fMenuBarHelpLayout;

   TGNumberEntry      *fNumericEntriesModuleAddress;
   TGTextEntry       *fTextEntryModuleIpString;

	TGButton           *fValidateIpAddressButton;
	TGButton            *fChk_JumboFrame;
//	TGHorizontalFrame   *fTGHorizontalIpSubFrame1, *fTGHorizontalIpSubFrame2;
	TGNumberEntry       *fNumericEntries_MaxNofPacketsPerReadRequest;
	TGLabel             *fLabel_MaxNofPacketsPerReadRequest;

	TGTextBuffer      *fTextBufferModuleIpString;

	SIS3316TestDialog *fTestDialogWindow;
	sis3316_flash *fFlashWindow;
	sis3316_root_gui_register_access *fRegisterAccessWindow;
	sis3316_udp_properties *fUdpPropertiesWindow;



protected:
	Bool_t fB_openfMenuSIS3316TestWindowFlag; // shows if setup window is open
	Bool_t fB_openfMenuSIS3316TestRunFlag; // shows if test is running
	
	Bool_t fB_openProgramMenueWindowFlag; // shows if setup window is open
	Bool_t fB_openRegAccessMenueWindowFlag; // shows if setup window is open
	Bool_t fB_openUdpPropertiesMenueWindowFlag; // shows if setup window is open

public:

	char	char_main_ip_addr_string[32];
	unsigned int uint_main_vme_base_addr;
 	char char_main_config_file[512];

	sis3316_adc* main_sis3316_adc;


public:
	TestMainFrame(const TGWindow* p, UInt_t w, UInt_t h, char*  char_ip_addr_string, unsigned int uint_vme_base_addr, char* char_config_file);
	//TestMainFrame(const TGWindow* p, UInt_t w, UInt_t h);
	virtual ~TestMainFrame();

   virtual void CloseWindow();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t);
   int ShowModuleInformation();

  
};

/*************************************************************************************************************************/
/*************************************************************************************************************************/


/*************************************************************************************************************************/


TestMainFrame::TestMainFrame(const TGWindow* p, UInt_t w, UInt_t h, char* char_ip_addr_string, unsigned int uint_vme_base_addr, char* char_config_file)
      : TGMainFrame(p, w, h)
{
	CHAR char_messages[128];
	UINT nof_found_devices;
	UINT return_code=0, data=0;
	CHAR s[64];
//	sis3316_adc  *sis3316_adc1 ;


	if (char_config_file[0] == '\0') {
		this->char_main_config_file[0] = '\0';
	}
	else {
		strcpy(this->char_main_config_file, char_config_file); // 
	}

 
	this->uint_main_vme_base_addr = uint_vme_base_addr;


	// Create test main frame. A TGMainFrame is a top level window.
	// use hierarchical cleaning
	this->SetCleanup(kDeepCleanup);
    this->SetWindowName("SIS3316 Test Menu");
	this->SetIconPixmap("sis1_sis3316.png");


	// open menueWindows
	fB_openfMenuSIS3316TestWindowFlag = kFALSE; // Setup
	fB_openfMenuSIS3316TestRunFlag = kFALSE; // Setup 
	fB_openProgramMenueWindowFlag = kFALSE; // Setup
	fB_openRegAccessMenueWindowFlag = kFALSE; // Setup
	fB_openUdpPropertiesMenueWindowFlag = kFALSE; // Setup



	fMenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX);
	fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 10, 0, 0);
	fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);

	fMenuFile = new TGPopupMenu(fClient->GetRoot());
	fMenuFile->AddEntry("E&xit", M_FILE_EXIT);


// add SIS3316 Test
	fMenuSIS3316Test = new TGPopupMenu(gClient->GetDefaultRoot());
	fMenuSIS3316Test->AddLabel("SIS3316 Test");
	fMenuSIS3316Test->AddSeparator();
	fMenuSIS3316Test->AddEntry("&Test 1", M_SIS3316TEST1_DLG);
	fMenuSIS3316Test->DisableEntry(M_SIS3316TEST1_DLG);

	// add FPGA Programming
	fMenuFpgaProgram = new TGPopupMenu(gClient->GetDefaultRoot());
	fMenuFpgaProgram->AddEntry("FPGA Programming Menu", M_FPGA_PROG_MENUE);
	fMenuFpgaProgram->DisableEntry(M_FPGA_PROG_MENUE);

	// add Register ReadWrite  
	fMenuRegisterAccess = new TGPopupMenu(gClient->GetDefaultRoot());
	fMenuRegisterAccess->AddEntry("Register Access Menu", M_REGISTER_ACCESS);
	fMenuRegisterAccess->DisableEntry(M_REGISTER_ACCESS);

	// add UDP TGMainFrame
	fMenuUdpDhcp = new TGPopupMenu(gClient->GetDefaultRoot());
	fMenuUdpDhcp->AddEntry("UDP DHCP Menu", M_UDP_DHCP_MENUE);
	fMenuUdpDhcp->DisableEntry(M_UDP_DHCP_MENUE);


// fMenuHelp = new TGPopupMenu(fClient->GetRoot());
   fMenuHelp = new TGPopupMenu(gClient->GetDefaultRoot());
   //fMenuHelp->AddSeparator();
   fMenuHelp->AddEntry("&About", M_HELP_ABOUT);

   // Menu button messages are handled by the main frame (i.e. "this")
   // ProcessMessage() method.
   fMenuFile->Associate(this);



   fMenuSIS3316Test->Associate(this);
   fMenuFpgaProgram->Associate(this);
   fMenuRegisterAccess->Associate(this);
   fMenuUdpDhcp->Associate(this);

   fMenuBar = new TGMenuBar(this);
   fMenuBar->AddPopup("&File", fMenuFile, fMenuBarItemLayout);
   fMenuBar->AddPopup("SIS3316 Test", fMenuSIS3316Test, fMenuBarItemLayout);
   fMenuBar->AddPopup("FPGA Programming", fMenuFpgaProgram, fMenuBarItemLayout);
   fMenuBar->AddPopup("Register Access", fMenuRegisterAccess, fMenuBarItemLayout);
   fMenuBar->AddPopup("UDP Properties", fMenuUdpDhcp, fMenuBarItemLayout);
   fMenuBar->AddPopup("Help", fMenuHelp, fMenuBarHelpLayout);

	this->AddFrame(fMenuBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

	main_frameh1 = new TGCompositeFrame(this, kHorizontalFrame);
	//main_frameh1->SetBackgroundColor(0xee0000); //
	this->AddFrame(main_frameh1, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

	TGIcon *fIcon1341 = new TGIcon(main_frameh1, "sislogo.bmp");
	fIcon1341->SetName("fIcon1341");
	main_frameh1->AddFrame(fIcon1341, new TGLayoutHints(kLHintsLeft | kLHintsTop, 5, 5, 15, 15));


#ifdef ETHERNET_VME_INTERFACE
	main_frameh1_fGrp[4] = new TGGroupFrame(main_frameh1, "SIS3153 IP Address");
	main_frameh1->AddFrame(main_frameh1_fGrp[4], new TGLayoutHints(kLHintsExpandX, 5, 5, 25, 25)); // hints, left, right, top, bottom
	main_framehsub0 = new TGHorizontalFrame(main_frameh1_fGrp[4], 200, 30);
	main_frameh1_fGrp[4]->AddFrame(main_framehsub0, new TGLayoutHints(kLHintsExpandX, 2, 2, 12, 2));

//	strcpy(gl_sis3153_ip_addr_string,"212.60.16.200") ; // SIS3153 IP address
//	strcpy(gl_sis3153_ip_addr_string,"192.168.1.11") ; // SIS3153 IP address

	strcpy(gl_sis3153_ip_addr_string, char_ip_addr_string); // SIS3316 IP address


	fTextEntryModuleIpString = new TGTextEntry(main_framehsub0, new TGTextBuffer(32), 9);
	fTextEntryModuleIpString->SetMaxLength(32);
	fTextEntryModuleIpString->Resize(200, fTextEntryModuleIpString->GetDefaultHeight());
 	fTextEntryModuleIpString->SetAlignment(kTextLeft);
	fTextEntryModuleIpString->SetText(gl_sis3153_ip_addr_string);
	//fTextEntryModuleIpString->Resize(83,fTextEntryModuleIpString->GetDefaultHeight());
	fTextEntryModuleIpString->Associate(this);
	main_framehsub0->AddFrame(fTextEntryModuleIpString, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

#endif

	main_frameh1_fGrp[0] = new TGGroupFrame(main_frameh1, "Interface Information");
	main_frameh1->AddFrame(main_frameh1_fGrp[0], new TGLayoutHints(kLHintsExpandX, 5, 5, 25, 25)); // hints, left, right, top, bottom

	fLabel_main_frameh1[0] = new TGLabel(main_frameh1_fGrp[0]," ");
	fLabel_main_frameh1[0]->SetTextJustify(kTextLeft + kTextCenterX );
	fLabel_main_frameh1[0]->SetMargins(0,0,0,0);
	fLabel_main_frameh1[0]->SetWrapLength(-1);
	main_frameh1_fGrp[0]->AddFrame(fLabel_main_frameh1[0], new TGLayoutHints(kLHintsExpandX,2,2,15,2));

	fLabel_main_frameh1[1] = new TGLabel(main_frameh1_fGrp[0]);
	fLabel_main_frameh1[1]->SetTextJustify(kTextLeft + kTextCenterX);
	fLabel_main_frameh1[1]->SetMargins(0,0,0,0);
	fLabel_main_frameh1[1]->SetWrapLength(-1);
	main_frameh1_fGrp[0]->AddFrame(fLabel_main_frameh1[1], new TGLayoutHints(kLHintsExpandX,2,2,2,5));

	fLabel_main_frameh1[2] = new TGLabel(main_frameh1_fGrp[0]);
	fLabel_main_frameh1[2]->SetTextJustify(kTextLeft + kTextCenterX);
	fLabel_main_frameh1[2]->SetMargins(0,0,0,0);
	fLabel_main_frameh1[2]->SetWrapLength(-1);
	main_frameh1_fGrp[0]->AddFrame(fLabel_main_frameh1[2], new TGLayoutHints(kLHintsExpandX,2,2,5,2));

	fLabel_main_frameh1[3] = new TGLabel(main_frameh1_fGrp[0]);
	fLabel_main_frameh1[3]->SetTextJustify(kTextLeft + kTextCenterX);
	fLabel_main_frameh1[3]->SetMargins(0,0,0,0);
	fLabel_main_frameh1[3]->SetWrapLength(-1);
	main_frameh1_fGrp[0]->AddFrame(fLabel_main_frameh1[3], new TGLayoutHints(kLHintsExpandX,2,2,2,2));

	fLabel_main_frameh1[4] = new TGLabel(main_frameh1_fGrp[0]);
	fLabel_main_frameh1[4]->SetTextJustify(kTextLeft + kTextCenterX);
	fLabel_main_frameh1[4]->SetMargins(0,0,0,0);
	fLabel_main_frameh1[4]->SetWrapLength(-1);
	main_frameh1_fGrp[0]->AddFrame(fLabel_main_frameh1[4], new TGLayoutHints(kLHintsExpandX,2,2,2,2));

	fLabel_main_frameh1[5] = new TGLabel(main_frameh1_fGrp[0]);
	fLabel_main_frameh1[5]->SetTextJustify(kTextLeft + kTextCenterX);
	fLabel_main_frameh1[5]->SetMargins(0,0,0,0);
	fLabel_main_frameh1[5]->SetWrapLength(-1);
	main_frameh1_fGrp[0]->AddFrame(fLabel_main_frameh1[5], new TGLayoutHints(kLHintsExpandX,2,2,2,2));

	fLabel_main_frameh1[6] = new TGLabel(main_frameh1_fGrp[0]);
	fLabel_main_frameh1[6]->SetTextJustify(kTextLeft + kTextCenterX);
	fLabel_main_frameh1[6]->SetMargins(0,0,0,0);
	fLabel_main_frameh1[6]->SetWrapLength(-1);
	main_frameh1_fGrp[0]->AddFrame(fLabel_main_frameh1[6], new TGLayoutHints(kLHintsExpandX,2,2,2,2));

	fLabel_main_frameh1[7] = new TGLabel(main_frameh1_fGrp[0]);
	fLabel_main_frameh1[7]->SetTextJustify(kTextLeft + kTextCenterX);
	fLabel_main_frameh1[7]->SetMargins(0,0,0,0);
	fLabel_main_frameh1[7]->SetWrapLength(-1);
	main_frameh1_fGrp[0]->AddFrame(fLabel_main_frameh1[7], new TGLayoutHints(kLHintsExpandX,2,2,2,15)); // hints, left, right, top, bottom


#ifdef ETHERNET_UDP_INTERFACE
	main_frameh1_fGrp[2] = new TGGroupFrame(main_frameh1, "SIS3316 IP Address");
	main_frameh1->AddFrame(main_frameh1_fGrp[2], new TGLayoutHints(kLHintsExpandX, 5, 5, 25, 10)); // hints, left, right, top, bottom
	main_framehsub = new TGHorizontalFrame(main_frameh1_fGrp[2], 200, 30);
	main_frameh1_fGrp[2]->AddFrame(main_framehsub, new TGLayoutHints(kLHintsExpandX, 2, 2, 12, 2));

	//char  gl_sis3316_ip_addr_string[32] ;
	//strcpy(gl_sis3316_ip_addr_string,"192.168.1.100") ; // SIS3316 IP address
	//strcpy(gl_sis3316_ip_addr_string,"sis3316-0161") ; // SIS3316 IP address
	//strcpy(gl_sis3316_ip_addr_string, "212.60.16.202"); // SIS3316 IP address
	strcpy(gl_sis3316_ip_addr_string, char_ip_addr_string); // SIS3316 IP address
	
	gl_int_jumbo_frame_flag        = 1 ;
	gl_uint_NofPackagesPerRequest  = 20;

	fTextEntryModuleIpString = new TGTextEntry(main_framehsub, new TGTextBuffer(32), TestMainFrame_kCM_ENTRY_IRQ_NO_10);
	fTextEntryModuleIpString->SetMaxLength(32);
	fTextEntryModuleIpString->Resize(150, fTextEntryModuleIpString->GetDefaultHeight());
 	fTextEntryModuleIpString->SetAlignment(kTextLeft);
	fTextEntryModuleIpString->SetText(gl_sis3316_ip_addr_string);
	//fTextEntryModuleIpString->Resize(83,fTextEntryModuleIpString->GetDefaultHeight());
	fTextEntryModuleIpString->Associate(this);
	main_framehsub->AddFrame(fTextEntryModuleIpString, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	fTextEntryModuleIpString->ChangeBackground(GetDefaultFrameBackground()); //

	fValidateIpAddressButton = new TGTextButton(main_framehsub, "  Validate IP address  ", TestMainFrame_kCM_kCM_BUTTON_IRQ_NO_10);
	fValidateIpAddressButton->ChangeBackground(GetDefaultFrameBackground()); //  
	fValidateIpAddressButton->Associate(this);
	main_framehsub->AddFrame(fValidateIpAddressButton, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 12, 2, 2, 2));


//******
	
	//**********************
	// UDP parameter section
	main_frameh1_fGrp[4] = new TGGroupFrame(main_frameh1, "UDP parameter");  // hints, left, right, top, bottom
	main_frameh1->AddFrame(main_frameh1_fGrp[4], new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 10));
	main_frameh1_fGrp4_fHor1 = new TGHorizontalFrame(main_frameh1_fGrp[4], 200, 30);
	main_frameh1_fGrp[4]->AddFrame(main_frameh1_fGrp4_fHor1, new TGLayoutHints(kLHintsExpandX, 2, 2, 12, 2));

	fChk_JumboFrame = new TGCheckButton(main_frameh1_fGrp4_fHor1, "Jumbo Frame enable ", TestMainFrame_kCM_CHECKBUTTON_IRQ_NO_10);
	main_frameh1_fGrp4_fHor1->AddFrame(fChk_JumboFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 15, 4, 2));// hints, left, right, top, bottom

	if (gl_int_jumbo_frame_flag == 0) {
		fChk_JumboFrame->SetState(kButtonUp); // is OFF !
	}
	else {
		fChk_JumboFrame->SetState(kButtonDown); // is ON !
	}
	fChk_JumboFrame->Associate(this);



	//**********************
	main_frameh1_fGrp4_fHor2 = new TGHorizontalFrame(main_frameh1_fGrp[4], 200, 30);
	main_frameh1_fGrp[4]->AddFrame(main_frameh1_fGrp4_fHor2, new TGLayoutHints(kLHintsExpandX, 2, 2, 12, 2));

	fNumericEntries_MaxNofPacketsPerReadRequest = new TGNumberEntry(main_frameh1_fGrp4_fHor2, 1 /* value */, 4 /* width */, TestMainFrame_kCM_ENTRY_IRQ_NO_11, (TGNumberFormat::kNESInteger)); //kNESHex
	main_frameh1_fGrp4_fHor2->AddFrame(fNumericEntries_MaxNofPacketsPerReadRequest, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	fLabel_MaxNofPacketsPerReadRequest = new TGLabel(main_frameh1_fGrp4_fHor2, "max Number of Packets per Read Request");
	main_frameh1_fGrp4_fHor2->AddFrame(fLabel_MaxNofPacketsPerReadRequest, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 5, 12));
	fNumericEntries_MaxNofPacketsPerReadRequest->SetLimits((TGNumberFormat::kNELLimitMinMax), 1, 32);

	fNumericEntries_MaxNofPacketsPerReadRequest->SetIntNumber(gl_uint_NofPackagesPerRequest); // 
	fNumericEntries_MaxNofPacketsPerReadRequest->Associate(this);



#else

	main_frameh1_fGrp[2] = new TGGroupFrame(main_frameh1, "SIS3316 VME Base Address");
	main_frameh1->AddFrame(main_frameh1_fGrp[2], new TGLayoutHints(kLHintsExpandX, 5, 5, 25, 25)); // hints, left, right, top, bottom
	main_framehsub = new TGHorizontalFrame(main_frameh1_fGrp[2], 200, 30);
	main_frameh1_fGrp[2]->AddFrame(main_framehsub, new TGLayoutHints(kLHintsExpandX, 2, 2, 12, 2));
	fNumericEntriesModuleAddress = new TGNumberEntry(main_framehsub, this->uint_main_vme_base_addr /* value */, 18 /* width */, 10 /* irq */ , (TGNumberFormat::kNESHex) ) ; //kNESHex
	fNumericEntriesModuleAddress->Associate(this);
	main_framehsub->AddFrame(fNumericEntriesModuleAddress, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

#endif
 
	main_frameh1_fGrp[1] = new TGGroupFrame(main_frameh1, "SIS3316 Information");
	main_frameh1->AddFrame(main_frameh1_fGrp[1], new TGLayoutHints(kLHintsExpandX, 5, 5, 25, 25)); // hints, left, right, top, bottom

	fLabel_main_frameh2[0] = new TGLabel(main_frameh1_fGrp[1]," ");
	fLabel_main_frameh2[0]->SetTextJustify(kTextLeft + kTextCenterX );
	fLabel_main_frameh2[0]->SetMargins(0,0,0,0);
	fLabel_main_frameh2[0]->SetWrapLength(-1);
	main_frameh1_fGrp[1]->AddFrame(fLabel_main_frameh2[0], new TGLayoutHints(kLHintsExpandX,2,2,15,2));

	fLabel_main_frameh2[1] = new TGLabel(main_frameh1_fGrp[1]);
	fLabel_main_frameh2[1]->SetTextJustify(kTextLeft + kTextCenterX);
	fLabel_main_frameh2[1]->SetMargins(0,0,0,0);
	fLabel_main_frameh2[1]->SetWrapLength(-1);
	main_frameh1_fGrp[1]->AddFrame(fLabel_main_frameh2[1], new TGLayoutHints(kLHintsExpandX,2,2,2,2));

	fLabel_main_frameh2[2] = new TGLabel(main_frameh1_fGrp[1]);
	fLabel_main_frameh2[2]->SetTextJustify(kTextLeft + kTextCenterX);
	fLabel_main_frameh2[2]->SetMargins(0,0,0,0);
	fLabel_main_frameh2[2]->SetWrapLength(-1);
	main_frameh1_fGrp[1]->AddFrame(fLabel_main_frameh2[2], new TGLayoutHints(kLHintsExpandX,2,2,2,2));

	fLabel_main_frameh2[3] = new TGLabel(main_frameh1_fGrp[1]);
	fLabel_main_frameh2[3]->SetTextJustify(kTextLeft + kTextCenterX);
	fLabel_main_frameh2[3]->SetMargins(0,0,0,0);
	fLabel_main_frameh2[3]->SetWrapLength(-1);
	main_frameh1_fGrp[1]->AddFrame(fLabel_main_frameh2[3], new TGLayoutHints(kLHintsExpandX,2,2,2,2));

	fLabel_main_frameh2[4] = new TGLabel(main_frameh1_fGrp[1]);
	fLabel_main_frameh2[4]->SetTextJustify(kTextLeft + kTextCenterX);
	fLabel_main_frameh2[4]->SetMargins(0,0,0,0);
	fLabel_main_frameh2[4]->SetWrapLength(-1);
	main_frameh1_fGrp[1]->AddFrame(fLabel_main_frameh2[4], new TGLayoutHints(kLHintsExpandX,2,2,2,15));

	fLabel_main_frameh2[5] = new TGLabel(main_frameh1_fGrp[1]);
	fLabel_main_frameh2[5]->SetTextJustify(kTextLeft + kTextCenterX);
	fLabel_main_frameh2[5]->SetMargins(0,0,0,0);
	fLabel_main_frameh2[5]->SetWrapLength(-1);
	main_frameh1_fGrp[1]->AddFrame(fLabel_main_frameh2[5], new TGLayoutHints(kLHintsExpandX,2,2,2,15));



/******************************************************************************************************************************/
/* VME Master Create, Open and Setup                                                                                          */
/******************************************************************************************************************************/


#ifdef PCI_VME_INTERFACE
	// create SIS1100/SIS310x vme interface device
	sis1100 *vme_crate = new sis1100(0);
#endif

#ifdef USB_VME_INTERFACE
	USHORT idVendor;
	USHORT idProduct;
	USHORT idSerNo;
	USHORT idFirmwareVersion;
	USHORT idDriverVersion;
	// create SIS3150USB vme interface device
	#ifdef LINUX
		sis315x *vme_crate = new sis315x(0);
	#endif
	#ifdef WINDOWS
		sis3150 *vme_crate = new sis3150(0);
	#endif
#endif


#ifdef USB3_VME_INTERFACE
	USHORT idVendor;
	USHORT idProduct;
	USHORT idSerNo;
	USHORT idDriverVersion;
	USHORT idFxFirmwareVersion;
	USHORT idFpgaFirmwareVersion;
	// create SIS3153USB vme interface device
	sis3153 *vme_crate = new sis3153(0);
#endif


#ifdef ETHERNET_UDP_INTERFACE
	char  pc_ip_addr_string[32] ;
	char  sis3316_ip_addr_string[32] ;
	strcpy(sis3316_ip_addr_string, gl_sis3316_ip_addr_string) ; // SIS3316 IP address
	//strcpy(sis3316_ip_addr_string,"212.60.16.200") ; // SIS3316 IP address
	//int return_code ;
	#ifdef WINDOWS
    //return_code = WSAStartup();
    return_code = WinsockStartup();
	#endif
	sis3316_eth *vme_crate = new sis3316_eth;
	// increase read_buffer size
	// SUSE needs following command as su: >sysctl -w net.core.rmem_max=33554432
	int	sockbufsize = 335544432 ; // 0x2000000
	return_code = vme_crate->set_UdpSocketOptionBufSize(sockbufsize) ;

	//strcpy(pc_ip_addr_string,"212.60.16.49") ; // Window example: secocnd Lan interface IP address is 212.60.16.49
	strcpy(pc_ip_addr_string,"") ; // empty if default Lan interface (Window: use IP address to bind in case of 2. 3. 4. .. LAN Interface)
	return_code = vme_crate->set_UdpSocketBindMyOwnPort( pc_ip_addr_string);

	vme_crate->set_UdpSocketSIS3316_IpAddress( sis3316_ip_addr_string);
	vme_crate->udp_reset_cmd();


#endif

#ifdef ETHERNET_VME_INTERFACE
	sis3153eth *vme_crate;
	sis3153eth(&vme_crate, gl_sis3153_ip_addr_string);
#endif 

#ifdef VMUSB_INTERFACE
	if (createInterface()) {
		exit(EXIT_FAILURE);
	}
	
#endif	
	// open Vme Interface device
	return_code = vme_crate->vmeopen ();  // open Vme interface
	vme_crate->get_vmeopen_messages (char_messages, &nof_found_devices);  // open Vme interface

#ifdef ETHERNET_VME_INTERFACE
	return_code = vme_crate->udp_sis3153_register_read((UINT)1, &data); // Module Id. and firmware version register
	if (return_code != 0) {
		nof_found_devices = 0;
	}
#endif


	fLabel_main_frameh1[0]->SetText(char_messages);
	sprintf(s,"-- found %d vme interface device(s)",nof_found_devices);
	fLabel_main_frameh1[1]->SetText(s);

	//printf("\n%s    (found %d vme interface device[s])\n\n",char_messages, nof_found_devices);


/******************************************************************************************/
// additional Vme interface device informations
#ifdef USB_VME_INTERFACE

	#ifdef WINDOWS
		vme_crate->get_device_informations (&idVendor, &idProduct, &idSerNo, &idFirmwareVersion, &idDriverVersion);  //

		sprintf(s,"idVendor:           %04X",idVendor);
		fLabel_main_frameh1[2]->SetText(s);
		sprintf(s,"idProduct:          %04X",idProduct);
		fLabel_main_frameh1[3]->SetText(s);
		sprintf(s,"idSerNo:            %d",idSerNo);
		fLabel_main_frameh1[4]->SetText(s);
		sprintf(s,"idFirmwareVersion:  %04X",idFirmwareVersion);
		fLabel_main_frameh1[5]->SetText(s);
	#endif

#endif

#ifdef USB3_VME_INTERFACE
	vme_crate->get_device_informations (&idVendor, &idProduct, &idSerNo, &idDriverVersion, &idFxFirmwareVersion, &idFpgaFirmwareVersion);  //

	sprintf(s,"idVendor:               0x%04X",idVendor);
	fLabel_main_frameh1[2]->SetText(s);
	sprintf(s,"idProduct:              0x%04X",idProduct);
	fLabel_main_frameh1[3]->SetText(s);
	sprintf(s,"idSerNo:                %d",idSerNo);
	fLabel_main_frameh1[4]->SetText(s);
	sprintf(s,"idDriverVersion:        0x%04X",idDriverVersion);
	fLabel_main_frameh1[5]->SetText(s);
	sprintf(s,"idFxFirmwareVersion:    0x%04X",idFxFirmwareVersion);
	fLabel_main_frameh1[6]->SetText(s);
	sprintf(s,"idFpgaFirmwareVersion:  0x%04X",idFpgaFirmwareVersion);
	fLabel_main_frameh1[7]->SetText(s);

#endif

#ifdef ETHERNET_VME_INTERFACE
	return_code = vme_crate->udp_sis3153_register_read((UINT)1, &data); // Module Id. and firmware version register
	if (return_code == 0) {
		sprintf(s, "Module Identifier:               0x%04X", (data >> 16) & 0xffff);
		fLabel_main_frameh1[3]->SetText(s);
		sprintf(s, "Fpga Firmware Version:  0x%04X", data & 0xffff);
		fLabel_main_frameh1[4]->SetText(s);
	}
	return_code = vme_crate->udp_sis3153_register_read(2, &data); // Serial Number register
	if (return_code == 0) {
		sprintf(s, "SerNo:                                 %d", data & 0xffff);
		fLabel_main_frameh1[5]->SetText(s);
	}

#ifdef SIS31353_JUMBO_FRAME_ENABLE
	vme_crate->set_UdpSocketEnableJumboFrame();  // enable jumbo frame transfer length
	sprintf(s, "JumboFrame enabled   ");
	fLabel_main_frameh1[6]->SetText(s);
#else
	vme_crate->set_UdpSocketDisableJumboFrame(); // disable jumbo frame transfer length
	sprintf(s, "JumboFrame disabled   ");
	fLabel_main_frameh1[6]->SetText(s);
#endif



#endif






/******************************************************************************************/

	gl_vme_crate = vme_crate ;

	if(nof_found_devices == 0) {
		return_code = -1 ;
	}
	else {
		return_code = this->ShowModuleInformation();
	}

	if (return_code == 0) {
		// kill request and grant from vme interface
		gl_vme_crate->vme_A32D32_write(this->uint_main_vme_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000);
		// arbitrate
		gl_vme_crate->vme_A32D32_write(this->uint_main_vme_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 1);
		
		this->main_sis3316_adc = new sis3316_adc( gl_vme_crate, this->uint_main_vme_base_addr);
		gl_sis3316_adc1 = this->main_sis3316_adc;
		if (this->main_sis3316_adc->device_variant == SIS::ADC::SIS3316::TYPE_SIS3316) { // SIS3316
			if (this->main_sis3316_adc->adc_125MHz_flag == 1) {
				sprintf(s, "SIS3316  125MHz-16bit");
			}
			else {
				sprintf(s, "SIS3316  250MHz-14bit");
			}
		}
		else { // SIS3316-2
			if (this->main_sis3316_adc->adc_125MHz_flag == 1) {
				sprintf(s, "SIS3316-2  125MHz-16bit");
			}
			else {
				sprintf(s, "SIS3316-2  250MHz-14bit");
			}

		}
		fLabel_main_frameh2[5]->SetText(s);
		fMenuSIS3316Test->EnableEntry(M_SIS3316TEST1_DLG);
		fMenuFpgaProgram->EnableEntry(M_FPGA_PROG_MENUE);
		fMenuRegisterAccess->EnableEntry(M_REGISTER_ACCESS);
	

#ifdef ETHERNET_UDP_INTERFACE
		fMenuUdpDhcp->EnableEntry(M_UDP_DHCP_MENUE);
		if (gl_int_jumbo_frame_flag == 0) {
			gl_vme_crate->set_UdpSocketDisableJumboFrame();
		}
		else {
			gl_vme_crate->set_UdpSocketEnableJumboFrame();
		}
		gl_vme_crate->set_UdpSocketReceiveNofPackagesPerRequest(gl_uint_NofPackagesPerRequest);

#endif
	}
	else {
		this->main_sis3316_adc = NULL  ;
		//this->main_sis3316_adc = sis3316_adc1 ;
		fMenuSIS3316Test->DisableEntry(M_SIS3316TEST1_DLG);
		fMenuFpgaProgram->DisableEntry(M_FPGA_PROG_MENUE);
		fMenuRegisterAccess->DisableEntry(M_REGISTER_ACCESS);
		fMenuUdpDhcp->DisableEntry(M_UDP_DHCP_MENUE);

#ifdef RUN_WITHOUT_HARDWARE
		fMenuSIS3316Test->EnableEntry(M_SIS3316TEST1_DLG);
#endif
	}
 


/******************************************************************************************/

	this->SetWindowName("SIS3316 Test (08-March-2024)");
	this->Move(MAIN_WINDOW_POSTION_X, MAIN_WINDOW_POSTION_Y);
	this->MapSubwindows();
	this->SetWMPosition(MAIN_WINDOW_POSTION_X, MAIN_WINDOW_POSTION_Y);
	this->Resize(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HIGH);   // resize to default size
	this->MapWindow();

}




/******************************************************************************************/
/******************************************************************************************/



TestMainFrame::~TestMainFrame()
{
   // Delete all created widgets.
   delete fMenuFile;
   delete fMenuSIS3316Test;
   delete fMenuHelp;
   //delete gl_vme_crate;
}

void TestMainFrame::CloseWindow()
{
   // Got close message for this MainFrame. Terminate the application
   // or returns from the TApplication event loop (depending on the
   // argument specified in TApplication::Run()).

   gApplication->Terminate(0);
}

/*******************************************************************************************************************************/

int TestMainFrame::ShowModuleInformation()
{
unsigned int return_code, data ;
float modTemp;
char s[64];

	return_code = gl_vme_crate->vme_A32D32_read (this->uint_main_vme_base_addr + SIS3316_MODID, &data);
	//printf("vme_A32D32_read: data = 0x%08x     return_code = 0x%08x\n", data, return_code);

	if (return_code == 0) {
		if ((data &0xffff0000) == 0x33160000) {
			sprintf(s,"VME base address = 0x%08x", this->uint_main_vme_base_addr);
			fLabel_main_frameh2[0]->SetText(s);
			fLabel_main_frameh2[0]->SetBackgroundColor(0x99FF99); // light green
			sprintf(s,"VME FPGA firmware version  = 0x%08x", data);
			fLabel_main_frameh2[1]->SetText(s);

			// kill request and grant from vme interface
			gl_vme_crate->vme_A32D32_write(this->uint_main_vme_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000);
			// arbitrate
			gl_vme_crate->vme_A32D32_write(this->uint_main_vme_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 1);


			gl_vme_crate->vme_A32D32_read (this->uint_main_vme_base_addr + SIS3316_ADC_CH1_4_FIRMWARE_REG, &data);
			sprintf(s,"ADC FPGA firmware version = 0x%08x", data);
			fLabel_main_frameh2[2]->SetText(s);

			gl_vme_crate->vme_A32D32_read (this->uint_main_vme_base_addr + SIS3316_INTERNAL_TEMPERATURE_REG, &data);
			modTemp = (float)(data & 0x3FF);
			modTemp /= 4;
 			sprintf(s,"Temperature   = %.1f C", modTemp);
			fLabel_main_frameh2[3]->SetText(s);

			gl_vme_crate->vme_A32D32_read (this->uint_main_vme_base_addr + SIS3316_SERIAL_NUMBER_REG, &data);
			sprintf(s,"Serial Number = %d", data & 0xffff);
			fLabel_main_frameh2[4]->SetText(s);

		}
		else {
			sprintf(s,"No SIS3316");
			fLabel_main_frameh2[0]->SetText(s);
			fLabel_main_frameh2[0]->SetBackgroundColor(0xFF9999); // light red
			sprintf(s,"address = 0x%08x", this->uint_main_vme_base_addr);
			fLabel_main_frameh2[1]->SetText(s);
			sprintf(s,"version register = 0x%08x", data);
			fLabel_main_frameh2[2]->SetText(s);
			sprintf(s," ");
			fLabel_main_frameh2[3]->SetText(s);
			fLabel_main_frameh2[4]->SetText(s);
 			fLabel_main_frameh2[5]->SetText(s);
 			return_code = 0x900 ;
		}

	}
	else {
		sprintf(s,"No SIS3316");
		fLabel_main_frameh2[0]->SetText(s);
		fLabel_main_frameh2[0]->SetBackgroundColor(0xFF9999); // light red
		sprintf(s,"address = 0x%08x", this->uint_main_vme_base_addr);
		fLabel_main_frameh2[1]->SetText(s);
		sprintf(s,"return_code = 0x%08x", return_code);
		fLabel_main_frameh2[2]->SetText(s);
		sprintf(s," ");
		fLabel_main_frameh2[3]->SetText(s);
		fLabel_main_frameh2[4]->SetText(s);
		fLabel_main_frameh2[5]->SetText(s);
	}
	return return_code ;
}

/*******************************************************************************************************************************/



Bool_t TestMainFrame::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
   // Handle messages send to the TestMainFrame object. E.g. all menu button
   // messages.
	int return_code;
	unsigned int data;
	char s[64];
	TString tstring_temp = "                                 ";
	//sis3316_adc  *sis3316_adc1 ;
#ifdef ETHERNET_VME_INTERFACE
	CHAR char_messages[128];
	UINT nof_found_devices;
#endif
    switch (GET_MSG(msg)) {
       //printf("switch (GET_MSG(msg) %d\n", GET_MSG(msg));


      case kC_COMMAND:
         //printf("kC_COMMAND\n");
         switch (GET_SUBMSG(msg)) {

            case kCM_MENUSELECT:
               //printf("kCM_MENUSELECT id=%ld\n", parm1);
               break;

            case kCM_MENU:
               //printf("kCM_MENU id=%ld\n", parm1);
               switch (parm1) {

                  case M_FILE_EXIT:
                     CloseWindow();   // this also terminates theApp
                     break;

				  case M_SIS3316TEST1_DLG:
						if(fB_openfMenuSIS3316TestWindowFlag == kFALSE){
							 //printf("M_SIS3316TEST1_DLG\n");
							fTestDialogWindow = new SIS3316TestDialog(fClient->GetRoot(), this, 800, 200, &fB_openfMenuSIS3316TestWindowFlag, &fB_openfMenuSIS3316TestRunFlag, this->uint_main_vme_base_addr, this->char_main_config_file);
							fB_openfMenuSIS3316TestWindowFlag = kTRUE;
						}
						else {
							fTestDialogWindow->RequestFocus();
						}

						break;
						 
						
				  case M_FPGA_PROG_MENUE:
					  //printf("M_FPGA_PROG_MENU\n");
					  if (fB_openProgramMenueWindowFlag == kFALSE) {
						  fB_openProgramMenueWindowFlag = kTRUE;
						  fFlashWindow = new sis3316_flash(gClient->GetRoot(), 360, 700, &fB_openProgramMenueWindowFlag);
					  }
					  else {
						  fFlashWindow->RequestFocus();
					  }
					  break;


				  case M_REGISTER_ACCESS:
					  if (fB_openRegAccessMenueWindowFlag == kFALSE) {
						  fB_openRegAccessMenueWindowFlag = kTRUE;
						  fRegisterAccessWindow = new sis3316_root_gui_register_access(gClient->GetRoot(), this->main_sis3316_adc, 360, 700, &fB_openRegAccessMenueWindowFlag);
						  //fRegisterAccessWindow = new sis3316_root_gui_register_access(gClient->GetRoot(),  360, 700, &fB_openRegAccessMenueWindowFlag);
					  }
					  else {
						  fRegisterAccessWindow->RequestFocus();
					  }
					  break; // M_REGISTER_ACCESS



				  case M_UDP_DHCP_MENUE:
					  //printf("M_UDP_DHCP_MENU\n");
					  if (fB_openUdpPropertiesMenueWindowFlag == kFALSE) {
						  fB_openUdpPropertiesMenueWindowFlag = kTRUE;
						  fUdpPropertiesWindow = new sis3316_udp_properties(gClient->GetRoot(), 500, 320, &fB_openUdpPropertiesMenueWindowFlag);
					  }
					  else {
						  fUdpPropertiesWindow->RequestFocus();
					  }
					  break;

                  default:
                     break;
               } // kCM_MENU switch (parm1)
               break;

/********************************************/
		#ifdef ETHERNET_UDP_INTERFACE
			case kCM_BUTTON:
				switch(parm1) {
					case TestMainFrame_kCM_kCM_BUTTON_IRQ_NO_10:  // validate IP address

						if (fB_openfMenuSIS3316TestWindowFlag == kTRUE) {
							//printf("Deactivate_Buttons\n");
							fTestDialogWindow->Deactivate_Buttons();
						}

						fValidateIpAddressButton->ChangeBackground(GetDefaultFrameBackground()); // 
						fTextEntryModuleIpString->ChangeBackground(GetDefaultFrameBackground());  
						tstring_temp = fTextEntryModuleIpString->GetText();
						strcpy(gl_sis3316_ip_addr_string,tstring_temp) ; // SIS3316 IP address

						return_code = gl_vme_crate->set_UdpSocketSIS3316_IpAddress( gl_sis3316_ip_addr_string);
						gl_vme_crate->udp_reset_cmd();
						this->uint_main_vme_base_addr = 0x0 ;

						fMenuSIS3316Test->DisableEntry(M_SIS3316TEST1_DLG);
						fMenuFpgaProgram->DisableEntry(M_FPGA_PROG_MENUE);
						fMenuRegisterAccess->DisableEntry(M_REGISTER_ACCESS);
						fMenuUdpDhcp->DisableEntry(M_UDP_DHCP_MENUE);

						return_code = this->ShowModuleInformation();
						if (return_code == 0) {
							// kill request and grant from vme interface
							gl_vme_crate->vme_A32D32_write(this->uint_main_vme_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000);
							// arbitrate
							gl_vme_crate->vme_A32D32_write(this->uint_main_vme_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 1);
							if (this->main_sis3316_adc != NULL) {
								delete this->main_sis3316_adc;
							}
							this->main_sis3316_adc = new sis3316_adc( gl_vme_crate, this->uint_main_vme_base_addr);
							gl_sis3316_adc1 = this->main_sis3316_adc;
							if (this->main_sis3316_adc->device_variant == SIS::ADC::SIS3316::TYPE_SIS3316) { // SIS3316
								if (this->main_sis3316_adc->adc_125MHz_flag == 1) {
									sprintf(s, "SIS3316  125MHz-16bit");
								}
								else {
									sprintf(s, "SIS3316  250MHz-14bit");
								}
							}
							else { // SIS3316-2
								if (this->main_sis3316_adc->adc_125MHz_flag == 1) {
									sprintf(s, "SIS3316-2  125MHz-16bit");
								}
								else {
									sprintf(s, "SIS3316-2  250MHz-14bit");
								}

							}
							fLabel_main_frameh2[5]->SetText(s);
							fMenuSIS3316Test->EnableEntry(M_SIS3316TEST1_DLG);
							fMenuFpgaProgram->EnableEntry(M_FPGA_PROG_MENUE);
							fMenuRegisterAccess->EnableEntry(M_REGISTER_ACCESS);
						#ifdef ETHERNET_UDP_INTERFACE
							fMenuUdpDhcp->EnableEntry(M_UDP_DHCP_MENUE);
							//fChk_JumboFrame->SetEnabled(kTRUE); //
							//fNumericEntries_MaxNofPacketsPerReadRequest->SetState(kTRUE); //

							if (gl_int_jumbo_frame_flag == 0) {
								gl_vme_crate->set_UdpSocketDisableJumboFrame();
								//printf("\nset_UdpSocketDisableJumboFrame\n");
							}
							else {
								gl_vme_crate->set_UdpSocketEnableJumboFrame();
								//printf("\nset_UdpSocketEnableJumboFrame\n");
							}
							gl_vme_crate->set_UdpSocketReceiveNofPackagesPerRequest(gl_uint_NofPackagesPerRequest);

							//printf("\nset_UdpSocketReceiveNofPackagesPerRequest = %d\n", gl_uint_NofPackagesPerRequest);
#endif

							if (fB_openfMenuSIS3316TestWindowFlag == kTRUE) {
								//printf("Activate_CLK_Configuration_Button\n");
								fTestDialogWindow->Activate_CLK_Configuration_Button();
							}

						};
	
						break;

					default:
						break;

				} //switch(parm1)
			break; //case kCM_BUTTON

 
			case kCM_CHECKBUTTON:
				//printf("TestMainFrame::ProcessMessage:case kC_COMMAND;kCM_CHECKBUTTON parm1 = %d \n", parm1);
				switch (parm1) {

				case TestMainFrame_kCM_CHECKBUTTON_IRQ_NO_10:  //  
					//printf("TestMainFrame_kCM_CHECKBUTTON_IRQ_NO_10 \n");
					if (fB_openfMenuSIS3316TestRunFlag == kTRUE) { // is running
						if (gl_int_jumbo_frame_flag == 0) {     // restore value
							fChk_JumboFrame->SetState(kButtonUp); // is OFF !
						}
						else {
							fChk_JumboFrame->SetState(kButtonDown); // is ON !
						}
					}
					else {
						if (fChk_JumboFrame->IsOn() == kTRUE) {
							gl_int_jumbo_frame_flag = 1;
						}
						else {
							gl_int_jumbo_frame_flag = 0;
						}
						if (gl_int_jumbo_frame_flag == 0) {
							gl_vme_crate->set_UdpSocketDisableJumboFrame();
							//printf("\nset_UdpSocketDisableJumboFrame\n");
						}
						else {
							gl_vme_crate->set_UdpSocketEnableJumboFrame();
							//printf("\nset_UdpSocketEnableJumboFrame\n");
						}
					}
					break;

				default:
					break;
				}
				break; // kCM_CHECKBUTTON


			case kCM_COMBOBOX:
				switch (parm1) {
				default:
					break;
				}
				break;//kCM_COMBOBOX


#endif // ETHERNET_UDP_INTERFACE
 /********************************************/

            default:
               break;
         } // kC_COMMAND switch (GET_SUBMSG(msg))
         break;

	  case kC_TEXTENTRY:
			//printf("kC_TEXTENTRY item %ld activated\n", parm1);
              switch (parm1) {
				 case 9:
					#ifdef ETHERNET_VME_INTERFACE						
					 if (fB_openfMenuSIS3316TestRunFlag == kTRUE) { // is running
						 fTextEntryModuleIpString->SetText(gl_sis3153_ip_addr_string);
					 }
					 else {
						 tstring_temp = fTextEntryModuleIpString->GetText();
						 strcpy(gl_sis3153_ip_addr_string, tstring_temp); // SIS3153 IP address

						 sis3153eth(&gl_vme_crate, gl_sis3153_ip_addr_string);

						 // open Vme Interface device
						 return_code = gl_vme_crate->vmeopen();  // open Vme interface
						 gl_vme_crate->get_vmeopen_messages(char_messages, &nof_found_devices);  // open Vme interface

 
						 return_code = gl_vme_crate->udp_sis3153_register_read((UINT)1,  &data); // Module Id. and firmware version register
						 if (return_code != 0) {
							 nof_found_devices = 0;
							 sprintf(s, " ");
							 fLabel_main_frameh1[3]->SetText(s);
							 fLabel_main_frameh1[4]->SetText(s);
							 fLabel_main_frameh1[5]->SetText(s);
							 fLabel_main_frameh1[6]->SetText(s);
						 }
 
						 if (nof_found_devices != 0) {

							 fLabel_main_frameh1[0]->SetText(char_messages);
							 sprintf(s, "-- found %d vme interface device(s)", nof_found_devices);
							 fLabel_main_frameh1[1]->SetText(s);

							 return_code = gl_vme_crate->udp_sis3153_register_read((UINT)1, &data); // Module Id. and firmware version register
							 if (return_code == 0) {
								 sprintf(s, "Module Identifier:               0x%04X", (data >> 16) & 0xffff);
								 fLabel_main_frameh1[3]->SetText(s);
								 sprintf(s, "Fpga Firmware Version:  0x%04X", data & 0xffff);
								 fLabel_main_frameh1[4]->SetText(s);
							 }
							 return_code = gl_vme_crate->udp_sis3153_register_read((UINT)2, &data); // Serial Number register
							 if (return_code == 0) {
								 sprintf(s, "SerNo:                                 %d", data & 0xffff);
								 fLabel_main_frameh1[5]->SetText(s);
							 }

	#ifdef SIS31353_JUMBO_FRAME_ENABLE
							 gl_vme_crate->set_UdpSocketEnableJumboFrame();  // enable jumbo frame transfer length
							 sprintf(s, "JumboFrame enabled   ");
							 fLabel_main_frameh1[6]->SetText(s);
	#else
							 gl_vme_crate->set_UdpSocketDisableJumboFrame(); // disable jumbo frame transfer length
							 sprintf(s, "JumboFrame disabled   ");
							 fLabel_main_frameh1[6]->SetText(s);			
	#endif
						 }





						 if (fB_openfMenuSIS3316TestWindowFlag == kTRUE) {
							 printf("Deactivate_Buttons\n");
							 fTestDialogWindow->Deactivate_Buttons();
						 }
						 fMenuSIS3316Test->DisableEntry(M_SIS3316TEST1_DLG);

						 return_code = this->ShowModuleInformation();
						 if (return_code == 0) {
							 // kill request and grant from vme interface
							 gl_vme_crate->vme_A32D32_write(this->uint_main_vme_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000);
							 // arbitrate
							 gl_vme_crate->vme_A32D32_write(this->uint_main_vme_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 1);
							 if (this->main_sis3316_adc != NULL) {
								 delete this->main_sis3316_adc;
							 }
							 this->main_sis3316_adc = new sis3316_adc(gl_vme_crate, this->uint_main_vme_base_addr);
							 gl_sis3316_adc1 = this->main_sis3316_adc;
							 if (this->main_sis3316_adc->device_variant == SIS::ADC::SIS3316::TYPE_SIS3316) { // SIS3316
								 if (this->main_sis3316_adc->adc_125MHz_flag == 1) {
									 sprintf(s, "SIS3316  125MHz-16bit");
								 }
								 else {
									 sprintf(s, "SIS3316  250MHz-14bit");
								 }
							 }
							 else { // SIS3316-2
								 if (this->main_sis3316_adc->adc_125MHz_flag == 1) {
									 sprintf(s, "SIS3316-2  125MHz-16bit");
								 }
								 else {
									 sprintf(s, "SIS3316-2  250MHz-14bit");
								 }

							 }
							 fLabel_main_frameh2[5]->SetText(s);
							 fMenuSIS3316Test->EnableEntry(M_SIS3316TEST1_DLG);
							 fMenuFpgaProgram->EnableEntry(M_FPGA_PROG_MENUE);
							 fMenuRegisterAccess->EnableEntry(M_REGISTER_ACCESS);

							 if (fB_openfMenuSIS3316TestWindowFlag == kTRUE) {
								 printf("Activate_CLK_Configuration_Button\n");
								 fTestDialogWindow->Activate_CLK_Configuration_Button();
							 }
						 };
					 }
					#endif


					 break;

				 case TestMainFrame_kCM_ENTRY_IRQ_NO_10: // Module address
					#ifdef ETHERNET_UDP_INTERFACE
					 if (fB_openfMenuSIS3316TestRunFlag == kTRUE) { // is running
						 fTextEntryModuleIpString->SetText(gl_sis3316_ip_addr_string);
					 }
					 else {
 						 fTextEntryModuleIpString->ChangeBackground(0xffff00); //light yellow
						 fValidateIpAddressButton->ChangeBackground(0x99FF99); // light green
					 }
					#else
						if (fB_openfMenuSIS3316TestRunFlag == kTRUE) { // is running
							fNumericEntriesModuleAddress->SetIntNumber(this->uint_main_vme_base_addr);
						}  
						else {

							if (fB_openfMenuSIS3316TestWindowFlag == kTRUE) {
								//printf("Deactivate_Buttons\n");
								fTestDialogWindow->Deactivate_Buttons();
							}

							this->uint_main_vme_base_addr = fNumericEntriesModuleAddress->GetIntNumber();
							fMenuSIS3316Test->DisableEntry(M_SIS3316TEST1_DLG);

							return_code = this->ShowModuleInformation();
							if (return_code == 0) {
								// kill request and grant from vme interface
								gl_vme_crate->vme_A32D32_write(this->uint_main_vme_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000);
								// arbitrate
								gl_vme_crate->vme_A32D32_write(this->uint_main_vme_base_addr + SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 1);
								if (this->main_sis3316_adc != NULL) {
									delete this->main_sis3316_adc;
								}
								this->main_sis3316_adc = new sis3316_adc(gl_vme_crate, this->uint_main_vme_base_addr);
								gl_sis3316_adc1 = this->main_sis3316_adc;

								//device_variant = reg & 0x80 ? SIS::ADC::SIS3316::TYPE_SIS3316_2 : SIS::ADC::SIS3316::TYPE_SIS3316;
								if (this->main_sis3316_adc->device_variant == SIS::ADC::SIS3316::TYPE_SIS3316) { // SIS3316
									if (this->main_sis3316_adc->adc_125MHz_flag == 1) {
										sprintf(s, "SIS3316  125MHz-16bit");
									}
									else {
										sprintf(s, "SIS3316  250MHz-14bit");
									}
								}
								else { // SIS3316-2
									if (this->main_sis3316_adc->adc_125MHz_flag == 1) {
										sprintf(s, "SIS3316-2  125MHz-16bit");
									}
									else {
										sprintf(s, "SIS3316-2  250MHz-14bit");
									}

								}
								
								fLabel_main_frameh2[5]->SetText(s);
								fMenuSIS3316Test->EnableEntry(M_SIS3316TEST1_DLG);
								fMenuFpgaProgram->EnableEntry(M_FPGA_PROG_MENUE);
								fMenuRegisterAccess->EnableEntry(M_REGISTER_ACCESS);


								if (fB_openfMenuSIS3316TestWindowFlag == kTRUE) {
									//printf("Activate_CLK_Configuration_Button\n");
									fTestDialogWindow->Activate_CLK_Configuration_Button();
								}

							};
						}
					#endif

						break;

				 case TestMainFrame_kCM_ENTRY_IRQ_NO_11: // max. number of packets
					#ifdef ETHERNET_UDP_INTERFACE
					 //printf("\n TestMainFrame_kCM_ENTRY_IRQ_NO_11\n");
					 if (fB_openfMenuSIS3316TestRunFlag == kTRUE) { // is running
						 fNumericEntries_MaxNofPacketsPerReadRequest->SetIntNumber(gl_uint_NofPackagesPerRequest); //  restore --> not changed
					 }
					 else {
						 gl_uint_NofPackagesPerRequest = fNumericEntries_MaxNofPacketsPerReadRequest->GetIntNumber(); //read
						 if (this->main_sis3316_adc != NULL) {
							 gl_vme_crate->set_UdpSocketReceiveNofPackagesPerRequest(gl_uint_NofPackagesPerRequest); //  
							 printf("\nset_UdpSocketReceiveNofPackagesPerRequest = %d\n", gl_uint_NofPackagesPerRequest);
						 }
					 }

					 break;


#endif

				 default:
                     break;
               } // kC_TEXTENTRY switch (parm1)
         break;

 

	  default:
         break;
   } // switch (GET_MSG(msg)
   return kTRUE;
}








//---- Main program ------------------------------------------------------------

int main(int argc, char **argv)
{

	int argcROOT = 1;
	//argcROOT = argc;
	char* argvROOT = argv[0]; // needs the app`s name only

	int int_arg_ch;
	//CHAR char_arg_messages[128];

	char  char_ip_addr_string[32];
	unsigned int uint_vme_base_addr;
	char char_config_file[512];




	//TApplication theApp("App", &argc, argv);
	//TApplication theApp("App", &argcROOT, &argvROOT);
	TApplication theApp("App", &argcROOT, &argvROOT);
#ifdef not_used
	if (gROOT->IsBatch()) {
      fprintf(stderr, "%s: cannot run in batch mode\n", argvROOT);
      //return 1;
	}
#endif
	/***********************************************************************************************************************************************/
	uint_vme_base_addr = FIRST_MODULE_BASE_ADDR; // Default
	strcpy(char_ip_addr_string, default_ip_addr); // SIS3316 or sis3153 default IP address
	strcpy(char_config_file, "\0"); //set to Empty string


	// get app parameter
	if (argc > 1) {
	   while ((int_arg_ch = getopt(argc, argv, "?lhI:X:C:")) != -1) {
		   switch (int_arg_ch) {
			   //printf("ch %c    \n", int_arg_ch);

			   case 'I':
				   //printf("case 'I' \n");
				   sscanf(optarg, "%s", char_ip_addr_string);
				   //printf("-I %s    \n", ch_string );
				   //strcpy(gl_cmd_ip_string, ch_string);
				   break;

			   case 'X':
				   //printf("case 'X' \n");
				   sscanf(optarg, "%X", &uint_vme_base_addr);
				   break;


			   case 'C':
				   //printf("case 'C' \n" );
				   sscanf(optarg, "%s", char_config_file);
				   //printf("char_config_file %s    \n", char_config_file );

			   break;



		   case '?':
		   case 'h':
		   default:
			   printf("   \n");
			   printf("Usage: %s  [-?h] [-I ip] [-X Vme Base Address] [-C config_filename.ini] ", argv[0]);
			   printf("   \n");
			   printf("   \n");
			   printf("   -X num      ......  SIS3316 VME Base Address           Default = 0x%08x\n", uint_vme_base_addr);
			   printf("   -I string   ......  SIS3316eth/SIS3153eth IP Address   Default = %s\n", char_ip_addr_string);
			   printf("   \n");
			   printf("   -C filename.ini ..  configuration file name; for example sis3316_running_parameter.ini \n");
			   printf("   \n");
			   printf("   -h     ..........   print this message only\n");
			   printf("   \n");
			   printf("   \n");
			   printf("   Examples: \n");
			   printf("   sis3316_root_gui -I 192.168.1.10 -X 0x30000000 -C config_internal_trigger.ini \n");
			   printf("   sis3316_root_gui -I sis3316-0450  -C config_external_trigger.ini \n");
			   printf("   \n");
			   printf("   date:               07. February 2024 \n");
			   printf("   \n");
			   printf("   \n");
			   exit(1);
		   }
	   }
   } // if (argc > 1)

	/***********************************************************************************************************************************************/
 
   new TestMainFrame(gClient->GetRoot(), 400, 220, char_ip_addr_string, uint_vme_base_addr, char_config_file);

   theApp.Run();

   return 0;
}


/***********************************************************************************************************************************************/




/***********************************************************************************************************************************************/
