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
string literal = "0x[0-9a-fA-F]+|[-]?[0-9][0-9]*";
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
regex wordOnlyRegex("^\\.word ");
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
regex endBracketRegex("\\]");
regex hexRegex("0x[0-9a-fA-F]+");
regex hexRemoveRegex("0x");
regex decRegex("[-]?[0-9][0-9]*");
regex helperSymbolRegex(symbol);

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
  int pass();

private:

  bool openFiles();
  void setGoodLines();
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

  enum RelocationType{R_16, R_PC16, R_WORD16};
  enum Binds{GLOBAL, LOCAL, NOBIND};
  enum SymbolType{NOTYP, SCTN};
  enum ForwardingType{TEXT, RELO, DATA};

  struct Forwarding{
    ForwardingType type;
    int sectionID;
    int patch;                        // address that needs to be patched
    int addend;
    int mcstart;
    int mcend;
    int offset;
    int offsetRelo;
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

  struct MachineCode{
    // string address;
    string value;
    string sectionName;
  };
  vector<vector<MachineCode>> machineCode;

  vector<MachineCode> backPatching(Symbol sym, int sectionId, int locationCounter, vector<MachineCode> code){
    for(Forwarding fw: sym.forwardingTable){
      if(fw.sectionID == sectionId){
        int mov = locationCounter - fw.mcend - 1;
        string mov1 = to_string(mov);
        vector<string> help = decToCode(mov1);
        bool done = false;

        if(relocationTable[sectionId][fw.offsetRelo].type == R_WORD16){
          code.at(fw.mcstart).value = help[1];
          code.at(fw.mcend).value = help[0];
        } else {
          if(relocationTable[sectionId][fw.offsetRelo].type == R_16){
            for(string s: help){
              if(!done){
                code.at(fw.mcstart).value = s;
                done = true;
              } else {
                code.at(fw.mcend).value = s;
              }
            }
          }
        }
      }
    }

    return code;
  }

  struct Relocation{
    int sectionId;
    int offset;
    RelocationType type;
    int addend;
    int symbolId;
  };
  vector<vector<Relocation>> relocationTable;

  void backPatchingRelocation(Symbol sym);

  vector<MachineCode> addToCode(string value, string sectionName, vector<MachineCode> machineCodes){
    MachineCode mc;
    mc.value = value;
    mc.sectionName = sectionName;
    machineCodes.push_back(mc);
    return machineCodes;
  };

  /**
   * @brief adds Symbol to symbol table if it doesn't exist or to forward if it does
   * 
   * @param ret               if symbol is in symbol table or not
   * @param symName           symbol name
   * @param currentSectionId  id of current section
   * @param locationCounter   current location counter
   * @param currentSection    current section
   */
  vector<Relocation> addSymbolOrForwardElement(int ret, string symName, int currentSectionId, int locationCounter, 
    Section currentSection, vector<Relocation> relocationTable, int pc, int startSize, int endSize){
    if(ret == -1){
      Symbol symb;
      symb.name = symName;
      symb.bind = NOBIND;
      symb.defined = false;
      symb.id = symbolId++;
      symb.offset = 0;       // unknown
      symb.size = 0;
      symb.type = NOTYP;
      symb.value = 0;
      symb.sectionId = currentSectionId;

      Forwarding fwd;
      fwd.type = RELO;
      fwd.addend = -2;
      fwd.patch = locationCounter;
      fwd.sectionID = currentSection.id;
      fwd.mcstart = startSize;
      fwd.mcend = endSize;
      fwd.offset = -1;
      fwd.offsetRelo = relocationTable.size();

      symb.forwardingTable.push_back(fwd);
      symbolTable.push_back(symb);
      relocationTable = addRelocation(relocationTable, locationCounter, currentSection.id, symb.id, pc);
    } else {                                        // there is symbol at table

      Symbol symb = symbolTable.at(ret);
      if(symb.defined){
        //TODO
      } else {  
        Forwarding fwd;
        fwd.type = RELO;
        fwd.addend = -2;
        fwd.patch = locationCounter;
        fwd.sectionID = currentSection.id;
        fwd.mcstart = startSize;
        fwd.mcend = endSize;
        fwd.offset = -1;
        fwd.offsetRelo = relocationTable.size();

        symb.forwardingTable.push_back(fwd);
        symbolTable.at(ret) = symb;
        relocationTable = addRelocation(relocationTable, locationCounter, currentSection.id, symb.id, pc);
      }
    
    }
    return relocationTable;
  }

  vector<Relocation> addRelocation(vector<Relocation> relocationTable, int locationCounter, int sectionId, int symbolId, int pc){
    Relocation rel;

    rel.offset = locationCounter - 2;
    rel.sectionId = sectionId;
    rel.symbolId = symbolId;

    if(pc == 0){
      rel.addend = -2;
      rel.type = R_PC16;
    }
    else{ 
      rel.addend = symbolTable.at(symbolId).offset;
      if(pc == 1){
        rel.type = R_16;
      } else {
        rel.type = R_WORD16;
      }
    }

    relocationTable.push_back(rel);
    return relocationTable;
  }

  vector<string> hexToCode(string num);
  vector<string> decToCode(string num);
};