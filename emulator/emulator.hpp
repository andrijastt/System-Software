#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <regex>
#include <fstream>
#include <iomanip>

using namespace std;

class Emulator{

public:

  Emulator(string inputFileString);
  int emulate();
  
private:

  bool openFile();
  void loadMemory();

  ifstream inputFile;
  string inputFileName;

  vector<string> Memory;
  string psw = "0000000000000000";
  int reg[9];   // r[0-7] + psw
};
