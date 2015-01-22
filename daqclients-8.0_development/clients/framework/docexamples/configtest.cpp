//
// Tests the configuration system.
#include <spectrodaq.h>
#include <SpectroFramework.h>
#include <iostream.h>
#include <fstream.h>
#include <vector>
#include <string>
#include <stdlib.h>


class MyApp : public DAQROCNode
{
  // Single configuration variables.

  int    m_nSingle;
  double m_fSingle;
  bool   m_bSingle;
  char*  m_pSingle;

  // Array bindings

  int    m_nArray[100];
  double m_fArray[100];
  bool   m_bArray[100];
  char*  m_pArray[100];

  // pointers to associative array bindings:
  //
  CAssocArrayBinding<int>*    m_pnAssoc;
  CAssocArrayBinding<double>* m_pfAssoc;
  CAssocArrayBinding<bool>*   m_pbAssoc;
  CAssocArrayBinding<char*>*  m_ppAssoc;  
protected:
  int operator()(int argc, char** argv);
  void DumpConfig(CConfigurationManager& rMgr, const char* dumpfile);
};

MyApp theApplication;

int
MyApp::operator()(int argc, char** pargv)
{
  // Bindings for single variables:

  cout << "Binding single variables" << endl;
  CVariableBinding<int>    nSingle(m_nSingle, "Integer", 0);
  CVariableBinding<double> fSingle(m_fSingle,  "Float" , 3.14159);
  CVariableBinding<bool>   bSingle(m_bSingle,  "Bool"  , false);
  CVariableBinding<char*>  pSingle(m_pSingle,  string("Char"), 
				   (char*)NULL);

  // Bindings for arrays:

  cout << "Binding array slices" << endl;

  CArrayBinding<int>    nArray(m_nArray, 0, 99, "IArray");
  CArrayBinding<double> fArray(m_fArray, 50,99, "FArray");
  CArrayBinding<bool>   bArray(m_bArray, 0, 49, "BArray");
  CArrayBinding<char*>  cArray(m_pArray, 25,75, string("SArray"));

  // Build a bound associative array:

  cout << "Creating associative arrays" << endl;

  CAssocArrayBinding<int>    nAssoc("IAssoc");
  CAssocArrayBinding<double> fAssoc("FAssoc");
  CAssocArrayBinding<bool>   bAssoc("BAssoc");
  CAssocArrayBinding<char*>  cAssoc(string("CAssoc"));
  m_pnAssoc = &nAssoc;
  m_pfAssoc = &fAssoc;
  m_pbAssoc = &bAssoc;
  m_ppAssoc = &cAssoc;

  // Setup the configuration object:

  cout << "Creating configuration manager and loading in bindings:" << endl;
  CConfigurationManager configurer;

  configurer.AddBinding(nSingle);
  configurer.AddBinding(fSingle);
  configurer.AddBinding(bSingle);
  configurer.AddBinding(pSingle);

  list<CTypeFreeBinding*> bindlist;
  bindlist.push_back(&nArray);
  bindlist.push_back(&fArray);
  bindlist.push_back(&bArray);
  bindlist.push_back(&cArray);
  configurer.AddBinding(bindlist);
  configurer.AddBinding(nAssoc);
  configurer.AddBinding(fAssoc);
  configurer.AddBinding(bAssoc);
  configurer.AddBinding(cAssoc);

  //  Configure from a single file:

  cout << "Reading single config file" << endl;

  configurer.ReadConfigFile("config.tcl");
 
  DumpConfig(configurer, "SingleFile");

  // Configure from first file in path list... should get same result.

  cout << "Reading 1st config file in pathlist" << endl;

  vector<string> Pathlist;
  Pathlist.push_back(string("."));
  Pathlist.push_back(string(getenv("HOME")));
  Pathlist.push_back(string(".."));
  configurer.Read1stConfigFile(Pathlist, "config.tcl");
  DumpConfig(configurer, "FirstFile");

  // Configure from multiple files in path list

  cout << "Reading all config files in pathlist." << endl;

  configurer.ReadAllConfigFiles(Pathlist, "config.tcl");
  DumpConfig(configurer, "AllFiles");


}
/// dumps the configured variables in two ways:
/// As a reconfig file and as a simple human readable file.
//

void
MyApp::DumpConfig(CConfigurationManager& rMgr, const char* dumpfile)
{
  // Make the filenames:

  string dumpname(dumpfile);
  string listname(dumpfile);
  dumpname += ".tcl";
  listname += ".txt";
  
  // Do the config dump:

  rMgr.WriteConfigFile(dumpname);

  ofstream txtfile(listname.c_str());
  txtfile << "----------------------- single variables --------------\n";
  txtfile << "m_nSingle = " << m_nSingle;
  txtfile << " m_fSingel = " << m_fSingle;
  txtfile << " m_bSingle = " << m_bSingle;
  txtfile << " m_pSingle = " << (m_pSingle ? m_pSingle : "null") << endl;

  txtfile << "-------------------------- Array variables -------------\n";
  txtfile << "index  int     double   bool     string\n";
  for(int i = 0; i < 100; i++) {
    txtfile << i << m_nArray[i] << " " << m_fArray[i] << " " << m_bArray[i] << " " ;
    txtfile << (m_pArray[i] ? m_pArray[i] : "(null)") << endl;

  }
  txtfile << "----------------------- nAssoc (integer assoc array) ----\n";
  txtfile << "Index           Value\n";
  map<string,int>::iterator i = m_pnAssoc->begin();
  while(i != m_pnAssoc->end()) {
    txtfile << i->first  << " " << i->second << endl;
    i++;
  }
  txtfile << "----------------------fAssoc (double assoc array) ------\n";
  txtfile << "Index           Value\n";
  map<string,double>::iterator f = m_pfAssoc->begin();
  while(f != m_pfAssoc->end()) {
    txtfile << f->first << " " << f->second << endl;
    f++;
  }
  txtfile << "---------------------------bAssoc (Bool assoc array)-----\n";
  txtfile << "Index           Value\n";
  map<string,bool>::iterator b = m_pbAssoc->begin();
  while(b != m_pbAssoc->end()) {
    txtfile << b->first << " " << b->second << endl;
    b++;
  }
  txtfile << "---------------------------cAssoc (char* assoc array) ------\n";
  txtfile << "Index           Value\n";
  map<string,char*>::iterator c = m_ppAssoc->begin();
  while(c != m_ppAssoc->end()) {
    txtfile << c->first << " " << c->second << endl;
    c++;			
  }


  
  
}
