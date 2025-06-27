// evt to root converter
#include <stdint.h>

#include <iostream>
#include <string>

#include "Converter.h"
#include <TFile.h>
#include <TTree.h>
#include <CRingItem.h>

using namespace ufmt;

void Converter::Initialize(std::string in, std::string fout)
{

  std::string rem = ".evt";
  std::string fin;
  fin = in + ".root";

  //strip out .evt from file name
  for(int i=fin.find(rem,0); i != std::string::npos; i=fin.find(rem,i)){
    fin.erase(i, rem.length());
  }

  std::cout << "file input  " << fin << std::endl;
  std::cout << "file output " << fout << std::endl; 

  m_fileout = new TFile(fout.c_str(),"RECREATE");

  // create tree and branch for ddas channel object
  m_treeout = new TTree("dchan","A time-ordered list of DDAS channel events");
  m_treeout->SetAutoSave(300000000); // autosave every 300 MBytes
  m_treeout->SetAutoFlush();
//  m_treeout->BranchRef();

}

void Converter::Close()
{
  // Write tree to output file
  m_fileout->cd();
  m_treeout->FlushBaskets();
  m_treeout->Write();

  m_fileout->Close();
  m_fileout=0;
  m_treeout=0;
}
