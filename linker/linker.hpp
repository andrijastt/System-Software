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
  void printHelpFile();
  bool checkForUNDSymbols();
  void setGoodCode();
  void setSymbolOffset();
  void doRelocations();
  vector<string> decToCode(string num);

  enum SymbolType{NOTYP, SCTN};
  enum SymbolBind{GLOBAL, LOCAL, NOBIND};
  enum RelocationType{R_16, R_PC16, R_WORD16};

  vector<string> inputFileStrings;
  string outputFileString;
  ifstream inputFile;
  ofstream outputFile;

  struct Section{
    int id;
    int size;
    string name;
  };

  vector<Section> Sections;
  int searchSection(Section sec);

  struct Symbol{
    int id;
    int offset;
    SymbolType type;
    SymbolBind bind;
    int sectionId;
    string symbolName;
    bool defined;
    string fileName;
  };

  vector<Symbol> Symbols;
  int searchSymbol(Symbol symb);

  struct Relocation{
    int offset;
    RelocationType type;
    int symbolId;
    int addend;
  };

  struct Relocations{
    string name;
    string fileName;
    vector<Relocation> relocations;
  };

  vector<Relocations> allRelocations;

  struct MachineCode{
    string sectionName;
    string fileName;
    vector<string> code;
    // int start;
  };

  vector<MachineCode> allMachineCode;
  vector<MachineCode> goodMachineCode;
};