/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_root_gui_register_access.h                           */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 06.09.2021                                       */
/*  last modification:    06.09.2021                                       */
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

#ifndef _SIS3316_ROOT_GUI_REGISTER_ACCESS_
#define _SIS3316_ROOT_GUI_REGISTER_ACCESS_
#include "rootIncludes.h"
//#include "sis3316_class.h"


class sis3316_root_gui_register_access : public TGMainFrame
{
private:
    //void *class_sis3316_adc_device;
    TGVerticalFrame* fVF;
    TGHorizontalFrame* fHF;
    TGGroupFrame *fGF, *fGF_Tips;
    TGLabel* fLblDescription;
    TGNumberEntry* fNuEnRdAddr, * fNuEnRdValue, * fNuEnWrAddr, * fNuEnWrValue;
    TGTextButton* fButRead, * fButWrite, * fButExit;
    Bool_t* fBopen_window_flag;


public:
    uint32_t register_read_addr;
    uint32_t register_write_addr;


public:
    sis3316_root_gui_register_access(const TGWindow* p, void* sis3316_adc_device, UInt_t w = 300, UInt_t h = 600, Bool_t* open_window_flag = NULL);
    //sis3316_root_gui_register_access(const TGWindow* p,  UInt_t w = 300, UInt_t h = 600, Bool_t* open_window_flag = NULL);
    Bool_t ProcessMessage(Long_t a, Long_t b, Long_t c);
    virtual void CloseWindow();
    ~sis3316_root_gui_register_access(void);
};
#endif
