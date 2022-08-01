#include "assembler.hpp"

Assembler::Assembler(string options, string outputFile, string inputFile){
 this->options = options;
 this->outputFile = outputFile;
 this->inputFile = inputFile;
}

bool checkInputData(string options, string outputFile, string inputFile){
  
  if(options != "-o" || outputFile.substr(outputFile.find_last_of(".")+1) != "o" || 
    inputFile.substr(inputFile.find_last_of(".")+1) != "s"){
      return false;
  } else return true;

}

int main(int argc, char const *argv[]){
  
  try{

    if(!argv[1] || !argv[2] || !argv[3]) throw InputException();

    string options = argv[1];
    string outputFile = argv[2];
    string inputFile = argv[3];

    if(!checkInputData(options, outputFile, inputFile)){
      throw InputException();
    }

    Assembler assembler(options, outputFile, inputFile);
  }
  catch(const std::exception& e){
    std::cerr << e.what() << '\n';
  }

  return 0;
}