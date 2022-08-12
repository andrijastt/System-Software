#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <regex>
#include <fstream>
#include <iomanip>

using namespace std;

// helper strings
string registers = "r[0-7]|sp|psw";
string literal = "[-]?[0-9][0-9]*|0x[0-9a-fA-F]+";
string symbol = "[a-zA-Z][a-zA-z0-9_]*";

string symbolOrLiteral = literal + "|" + symbol;
string listSymbols = "(" + symbol + "(,[ ]* " + symbol + ")*)$";
string listSymbolsAndLiterals = "((" + symbol + "|" + literal +")((, " + symbol + "|" + literal + "))*)$";

string operandsForData = "\\$(" + literal + ")|\\$" + symbol + "|" + literal + "|" + symbol + "|%" + symbol 
+ "|" + registers + "|\\[" + registers + " \\+ " + literal + "\\]|\\[" + registers + " \\+ " + symbol + "\\]|\\[" 
+ registers + "\\]";

string operandsForJumps = literal + "|" + symbol + "|%" + symbol + "|\\*(" + literal + ")|\\*" + symbol + "|\\*" 
  + registers + "|\\*\\[" + registers + " \\+ " + literal + "\\]|\\*\\[" + registers + " \\+ " + literal 
  + "\\]|\\*\\[" + registers + "\\]";

// register, symbols, labels regex, literal
regex registersRegex(registers);
regex labelRegex("^" + symbol + ":[ ]*");

regex symbolRegex(symbol + "[ ]*");
regex symbolOnlyRegex(symbol);
regex symbolNoBracketsRegex(symbol + "[^\\[\\]\\(\\) +-]");
regex literalRegex(literal);
regex symbolOrLiteralRegex(symbolOrLiteral);
regex operandsForJumpsRegex(operandsForJumps);

// assembler directives regex
regex globalRegex("^\\.global " + listSymbols + "$");
regex externRegex("^\\.extern " + listSymbols + "$");
regex sectionRegex("^\\.section " + symbol + "$");
regex wordRegex("^\\.word " + listSymbolsAndLiterals + "$");
// regex skipRegex("^\\.skip " + literal + "$");  // ovo gore nece nzm zasto ali neka ostane tako
regex skipRegex("^\\.skip");
regex endRegex("^\\.end$");

// assembler instructions regex
regex noOperandsInstructions("^(halt|iret|ret)$");
regex oneRegisterInsturctions("^(int|push|pop|not) " + registers + "$");
regex oneOperandInstructions("^(call|jmp|jeq|jne|jgt) (.)+$");
regex oneOperandOneRegisterInstructions("^(ldr|str) (.)+$");
regex twoRegistersInstructions("^(xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr) (" + registers + "),[ ]*(" + registers + ")$");

// helper regex
regex tabsRegex("\t");
regex extraSpacesRegex("[ ]+");
regex startSpacesRegex("^[ ]+");
regex commentsRegex("#.*");
regex spacesRegex("[ ]*");
regex endSpacesRegex("[ ]*$");
regex commaRegex(", ");
regex bracketsRegex("[\\[\\]\\(\\)]");

// regex for operands for jumps
regex pcRelSymbolJumpRegex("%" + symbol);
regex valueMemLiteralJumpRegex("\\*("  + literal + ")");
regex valueMemSymbolJumpRegex("\\*("  + symbol + ")");
regex registerDirectJumpRegex("\\*" + registers);
regex registerIndirectJumpRegex("\\*(\\[" + registers + "\\])");
regex registerIndirectLiteralJumpRegex("\\*\\[(" + registers + ") \\+ (" + literal + ")\\]");
regex registerIndirectSymbolJumpRegex("\\*\\[(" + registers + ") \\+ (" + symbol + ")\\]");

// regex for operands for data
regex valueLiteralDataRegex("\\$(" + literal + ")");
regex valueSymbolDataRegex("\\$(" + symbol + ")");
regex pcRelSymbolDataRegex("\\%" + symbol);
regex registerIndirectDataRegex("\\[" + registers + "\\]");
regex registerIndirectLiteralDataRegex("\\[" + registers + " \\+ (" + literal + ")\\]");
regex registerIndirectSymbolDataRegex("\\[" + registers + " \\+ (" + symbol + ")\\]");

class Assembler{

public:

  Assembler(string outputFile, string inputFile) throw();
  bool openFiles();
  void setGoodLines();
  int pass();

private:

  void printOutput();
  int searchSymbol(string symbolName);

  string outputFileString, inputFileString;
  ifstream inputFile;
  ofstream outputFile;
  vector<string> goodLines;

  static int sectionId;
  struct Section{
    string name;
    int base;
    int length;
    int id;
  };
  vector<Section> sectionTable;

  enum RelocationTypes{};
  enum Binds{GLOBAL, LOCAL, UND};
  enum SymbolType{NOTYP, SCTN};
  enum ForwardingType{TEXT, RELO, DATA};

  struct Forwarding{
    ForwardingType type;
    int sectionID;
    int patch;                        // address that needs to be patched
    int addend;
  };

  static int symbolId;
  struct Symbol{
    int id;
    string name;
    int offset;
    int sectionId;
    int value;
    int size;
    Binds bind;
    SymbolType type;
    bool defined;
    vector<Forwarding> forwardingTable;
  };
  vector<Symbol> symbolTable;

  Symbol findSymbol();

  struct MachineCode{
    // string address;
    string value;
    string sectionName;
  };
  vector<vector<MachineCode>> machineCode;

  vector<MachineCode> addToCode(string value, string sectionName, vector<MachineCode> machineCodes){
    MachineCode mc;
    mc.value = value;
    mc.sectionName = sectionName;
    machineCodes.push_back(mc);
    return machineCodes;
  };

  void addSymbolOrForwardElement(int ret, string symName, int currentSectionId, int locationCounter, 
    Section currentSection){
    if(ret == -1){
      Symbol symb;
      symb.name = symName;
      symb.bind = UND;
      symb.defined = false;
      symb.id = symbolId++;
      symb.offset = -1;       // unknown
      symb.size = 0;
      symb.type = NOTYP;
      symb.value = 0;
      symb.sectionId = currentSectionId;

      Forwarding fwd;
      fwd.type = TEXT;
      fwd.addend = -2;
      fwd.patch = locationCounter;
      fwd.sectionID = currentSection.id;

      symb.forwardingTable.push_back(fwd);
      symbolTable.push_back(symb);
    } else {                                        // there is symbol at table

      Symbol symb = symbolTable.at(ret);
      if(symb.defined){
        //TODO
      } else {  
        Forwarding fwd;
        fwd.type = TEXT;
        fwd.addend = -2;
        fwd.patch = locationCounter;
        fwd.sectionID = currentSection.id;

        symb.forwardingTable.push_back(fwd);
        symbolTable.at(ret) = symb;
      }
      
    }
  }
};