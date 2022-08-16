#include "linker.hpp"
#include "exceptions.hpp"

using namespace std;

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

  this->inputFileStrings = inputFileStrings;
  this->outputFileString = outputFileString;
}

/**
 * @brief opens files and check if input files exist
 * 
 * @return true everyhing is okay, creates new output File and all input Files exist
 * @return false some inputFiles don't exist
 */
bool Linker::openFiles(){

  this->outputFile.open(outputFileString, ios::trunc | ios::out);

  for(string s: this->inputFileStrings){
    ifstream inputStream;
    inputStream.open(s, ios::in);

    if(inputStream.is_open()){
      this->inputFiles.push_back(inputStream);
    } else {
      return false;
    }
  
  }
  return true;
}

/**
 * @brief Linker links all input files
 * 
 * @return int 0 - everything is okay, -1 - wrong terminal input, -2 - some input files don't exist
 * 
 */
int Linker::link(){

  if(!openFiles()){
    return -2;
  }

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

  }
  catch(const exception& e){

    cerr << e.what() << '\n';
  }
  
  
  return 0;
}
