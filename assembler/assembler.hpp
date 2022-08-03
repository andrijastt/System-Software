#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <regex>
#include <fstream>

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
// regex labelOnlyRegex("^" + symbol + ":[ ]*$");
regex symbolRegex(symbol + "[ ]*");
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

class Assembler{

public:

  Assembler(string outputFile, string inputFile) throw();
  bool openFiles();
  void setGoodLines();
  int pass();

private:

  string outputFileString, inputFileString;
  ifstream inputFile;
  ofstream outputFile;
  vector<string> goodLines;


};