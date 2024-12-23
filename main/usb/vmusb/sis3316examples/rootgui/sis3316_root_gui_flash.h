/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_root_gui_flash.h                                     */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 27.03.2015                                       */
/*  last modification:    27.03.2015                                       */
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
/*  http://www.struck.de                                                   */
/*                                                                         */
/*  © 2015                                                                 */
/*                                                                         */
/***************************************************************************/

#ifndef _SIS3316_ROOT_GUI_FLASH_
#define _SIS3316_ROOT_GUI_FLASH_
#include "rootIncludes.h"

class sis3316_flash : public TGMainFrame
{
private:
	TGVerticalFrame *fVert1;
	TGHorizontalFrame *fHor1;
	TGHorizontalFrame *fHorADC1 ;
	//TGHorizontalFrame *fHorADC1, *fHorADC2, *fHorADC3, *fHorADC4;
	TGGroupFrame *fGrpMod, *fGrpVME, *fGrpADC, *fGrpBarProg, *fGrpBarVerify, *fGrpReboot;
	TGTextButton *fButVMEProg, *fButADCProg, *fButReboot;
	TGLabel *fLabel_reboot_note ;

	TGHProgressBar *fBar_prog, *fBar_verify;
	TGLabel *fLblVMEImg;
	//TGLabel *fLblADC1Img, *fLblADC2Img, *fLblADC3Img, *fLblADC4Img;
	TGLabel *fLblADC1Img;
	TGTextEntry *fVMEImgPath;
	TGTextEntry *fADC1ImgPath;
	//TGTextEntry *fADC1ImgPath, *fADC2ImgPath, *fADC3ImgPath, *fADC4ImgPath;
	TGTextButton *fButVMEBrowse;
	TGTextButton *fButADC1Browse;
	//TGTextButton *fButADC1Browse, *fButADC2Browse, *fButADC3Browse, *fButADC4Browse;
	//TGCheckButton *fChkADCImgSelect;
	TGComboBox *fCBox;


	TGFileInfo fiVME;
	TGFileInfo fiADC1, fiADC2, fiADC3, fiADC4;
	Bool_t *fBSetup;

	Bool_t reboot_prevent_flag ;
	Pixel_t green;
	Pixel_t red;

public:
	sis3316_flash(const TGWindow *p, UInt_t w = 300, UInt_t h = 600, Bool_t *b = NULL);
	Bool_t ProcessMessage(Long_t a, Long_t b, Long_t c);
	static void progressProgCallback(int percent);
	static void progressVerifyCallback(int percent);
	void updateBar(int percent);
	~sis3316_flash(void);
};
#endif
