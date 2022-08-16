#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <regex>
#include <fstream>
#include <iomanip>

using namespace std;

class Linker{

public:

  Linker(vector<string> inputFileStrings, string outputFileString);
  int link();

private:

  bool openFiles();

  enum RelocationType{R_16, R_PC16};
  enum Binds{GLOBAL, LOCAL, NOBIND};
  enum SymbolType{NOTYP, SCTN};
  enum ForwardingType{TEXT, RELO, DATA};

  vector<string> inputFileStrings;
  string outputFileString;
  ifstream inputFile;
  ofstream outputFile;

};