/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_root_gui_udp_properties.h                            */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 06.09.2021                                       */
/*  last modification:    29.10.2021                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*  Fax  +49 (0)40 60 87 305 20                                            */
/*                                                                         */
/*  https://www.struck.de                                                  */
/*                                                                         */
/*  © 2021                                                                 */
/*                                                                         */
/***************************************************************************/

#ifndef _SIS3316_ROOT_GUI_UDP_PROPERTIES_
#define _SIS3316_ROOT_GUI_UDP_PROPERTIES_
#include "rootIncludes.h"

#define SIS3316UdpDialog_kCM_COMBOBOX_IRQ_NO_30			30   
#define SIS3316UdpDialog_kCM_BUTTON_IRQ_NO_30			30   


const char* const entry_Select_DHCP_Mode[3] = {
   "Clear DHCP option value (DHCP mode depends on Switch SW80-4)",  //
   "Enable DHCP",  //
   "Disable DHCP"   //
};

class sis3316_udp_properties : public TGMainFrame
{
private:
	TGVerticalFrame *fVert1;
	TGGroupFrame *fGrpDHCP;

	TGHorizontalFrame *fHor1, * fHor2, *fHor3 ;

	TGComboBox* fCombo_SetSelect_DHCP_Mode;

	TGGroupFrame *fGrpReboot;
	TGTextButton *fButReboot;
	TGLabel *fLabel_reboot_note ;

	Bool_t *fBSetup;

	Bool_t reboot_prevent_flag ;
	Pixel_t green;
	Pixel_t red;
	unsigned int dhcp_option;


public:
	sis3316_udp_properties(const TGWindow *p, UInt_t w = 300, UInt_t h = 600, Bool_t *b = NULL);
	Bool_t ProcessMessage(Long_t a, Long_t b, Long_t c);
	~sis3316_udp_properties(void);
};
#endif
