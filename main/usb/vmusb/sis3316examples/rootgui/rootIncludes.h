/***************************************************************************/
/*                                                                         */
/*  Filename: rootIncludes.h                                               */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 27.03.2015                                       */
/*  last modification:    05.07.2024                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  ROOT library (root_v6.32.02)                                           */
/*      Linux: used root libraries                                         */
/*             LIBS : = -lfftw3 -lCore -lHist -lGraf -lGui                 */
/*                      -lGpad -lMatrix -lMathCore -lRIO                   */
/*                      -lImt -ltbb -lThread                               */
/*                                                                         */
/*      Window: used root libraries                                        */
/*                   #pragma comment (lib, "libGui")                       */
/*                   #pragma comment (lib, "libCore")                      */
/*           //#pragma comment (lib, "libCint") remove with root_v6.xx.xx  */
/*                   #pragma comment (lib, "libRIO")                       */
/*                   #pragma comment (lib, "libNet")                       */
/*                   #pragma comment (lib, "libHist")                      */
/*                   #pragma comment (lib, "libGraf")                      */
/*                   #pragma comment (lib, "libGraf3d")                    */
/*                   #pragma comment (lib, "libGpad")                      */
/*                   #pragma comment (lib, "libTree")                      */
/*                   #pragma comment (lib, "libRint")                      */
/*                   #pragma comment (lib, "libPostscript")                */
/*                   #pragma comment (lib, "libMatrix")                    */
/*                   #pragma comment (lib, "libPhysics")                   */
/*                   #pragma comment (lib, "libMathCore")                  */
/*                   #pragma comment (lib, "libThread")                    */
/*                                                                         */
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
/*  © 2024                                                                 */
/*                                                                         */
/***************************************************************************/

#ifndef _SIS_ROOT_INCLUDES_
#define _SIS_ROOT_INCLUDES_

#include <TApplication.h>
#include <TGClient.h>

#include <TThread.h>
#include <TROOT.h>
#include <TVirtualX.h>
#include <TVirtualPadEditor.h>
#include <TGResourcePool.h>
#include <TGListBox.h>
#include <TGListTree.h>
#include <TGFSContainer.h>
#include <TGFrame.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGTextView.h>
#include <TGTextViewStream.h>
#include <TGNumberEntry.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TGCanvas.h>
#include <TGComboBox.h>
#include <TGTab.h>
#include <TGSlider.h>
#include <TGDoubleSlider.h>
#include <TGFileDialog.h>
#include <TGTextEdit.h>
#include <TGShutter.h>
#include <TGProgressBar.h>
#include <TGColorSelect.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TH1.h>
#include <TH2.h>
#include <TRandom.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TEnv.h>
#include <TFile.h>
#include <TKey.h>
#include <TGDockableFrame.h>
#include <TGFontDialog.h>
#include <TGenericClassInfo.h>
#include <TF1.h>
#include <TGXYLayout.h>
#include <TGTable.h>
#include <TGStatusBar.h>
#include "TGButtonGroup.h"
#include <TTree.h>
#include <TGTab.h>
#include <TGraph.h>
#include "TMultiGraph.h"
#include "TGTripleSlider.h"
#include <TStyle.h>
#include "TGLayout.h"
#include "TMath.h"

#include <TPave.h>
#include <TPaveText.h>

#include "TLatex.h"

#include <RQ_OBJECT.h> 


#endif
