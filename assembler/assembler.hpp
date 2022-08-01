#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <regex>

#include "exceptions.hpp"

using namespace std;

// helper strings
string registers = "r[0-7]|sp|psw";
string literal = "-?[1-9][0-9]*|0x[0-9a-fA-F]+";
string symbol = "[a-zA-Z][a-zA-z0-9_]";
string listSymbols = "(" + symbol + "(, " + symbol + ")*)$";
string listSymbolsAndLiterals = "((" + symbol + "|" + literal +")((, " + symbol + "|" + literal + "))*)$";
// string operandsForData = "\$" + literal + "|\$" + symbol + "|" + literal + "|" + symbol + "|%" + symbol + "|" + registers + "|[" + registers + " \+ " + literal + "]|[" + registers + " \+ " + literal + "]|[" + registers + "]";
// string operandsForJumps = literal + "|" + symbol + "|%" + symbol + "|\*" + literal + "|\*" + symbol + "|\*" + registers + "|\*[" + registers + " \+ " + literal + "]|\*[" + registers + " \+ " + literal + "]|\*[" + registers + "]";

// register, symbols, labels regex
regex registersRegex(registers);
regex labelRegex("^" + symbol + ":");

// assembler directives regex
regex globalRegex("^\\.global " + listSymbols);
regex externRegex("^\\.extern " + listSymbols);
regex sectionRegex("^\\.section" + symbol);
regex wordRegex("^\\.word " + listSymbolsAndLiterals);
regex skipRegex("^\\.skip " + literal);
regex endRegex("^\\.end");

// assembler instructions regex
regex noOperandsInstructions("^(halt|iret|ret)$");
regex oneRegisterInsturctions("^(int|push|pop|not) " + registers + "$");
regex oneOperandInstructions("^(call|jmp|jeq|jne|jgt) (.)+$");
regex twoRegistersInstructions("^(xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr) " + registers + ", " + registers + "$");

// helper regex
regex tabsRegex("\t");
regex extraSpacesRegex("[ ]+");
regex commentsRegex("#.*");

class Assembler{

public:

  Assembler(string options, string outputFile, string inputFile);

private:

  string options, outputFile, inputFile;

};