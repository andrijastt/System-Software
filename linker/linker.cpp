#include "linker.hpp"
#include "exceptions.hpp"

/**
 * @brief checks input data
 * 
 * @param option1 option has tu be "-hex"
 * @param option2 option has to be "-o"
 * @param outputFile after option2, output File has to be .hex
 * @param inputFiles after output File, input files have to be .o
 * @return true everything is good
 * @return false something is bad
 */
bool checkInputData(string option1, string option2, string outputFile, vector<string> inputFiles){

  if(inputFiles.size() == 0) return false;

  if(option1 != "-hex" || option2 != "-o" || outputFile.substr(outputFile.find_last_of(".")+1) != "hex")
    return false;

  int i = 0;
  for(string s: inputFiles){
    if(s.substr(s.find_last_of(".")+1) != "o")
      return false;
  }

  return true;
}

/**
 * @brief Construct a new Linker:: Linker object
 * 
 * @param inputFileStrings sets all input File names
 * @param outputFileString sets output file name
 */
Linker::Linker(vector<string> inputFileStrings, string outputFileString){

  int i = 0;
  for(string s: inputFileStrings){
    inputFileStrings[i] = "linker." + s;
    i++;
  }

  this->inputFileStrings = inputFileStrings;
  this->outputFileString = outputFileString;
}

/**
 * @brief Linker links all input files
 * 
 * @return int 0 - everything is okay, -1 - wrong terminal input, -2 - some input files don't exist
 * 
 */
int Linker::link(){

  for(string s: this->inputFileStrings){
    inputFile.open(s, ios::in);

    if(!inputFile.is_open()) return -2;

    string line;
    while(getline(inputFile, line)){

    }

    inputFile.close();
  }

  return 0;

}

int main(int argc, char const *argv[]){

  try{

    if(!argv[1] || !argv[2] || !argv[3]) throw InputException();

    string option1 = argv[1];
    string option2 = argv[2];
    string outputFile = argv[3];
    vector<string> inputFiles;
    int i = 4;
    while(argv[i]){
      inputFiles.push_back(argv[i]);
      i++;
    }

    if(!checkInputData(option1, option2, outputFile, inputFiles)){
      throw InputException();
    }

    Linker linker(inputFiles, outputFile);
    int ret = linker.link();

    if(ret == - 2) throw InputException();

  }
  catch(const exception& e){

    cerr << e.what() << '\n';
  }
  
  
  return 0;
}
