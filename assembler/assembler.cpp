#include "assembler.hpp"

Assembler::Assembler(string outputFile, string inputFile) throw(){
  this->outputFileString = outputFile;
  this->inputFileString = inputFile;
}

bool Assembler::openFiles(){
  this->inputFile.open(inputFileString, ios::in);
  this->outputFile.open(outputFileString, ios::out|ios::trunc);

  if(!this->inputFile.is_open()){
    return false;
  } else {
    return true;
  }
}

void Assembler::setGoodLines(){

  string line;

  while(getline(this->inputFile, line)){

    string newLine;

    newLine = regex_replace(line, commentsRegex, "");
    newLine = regex_replace(newLine, tabsRegex, " ");
    newLine = regex_replace(newLine, extraSpacesRegex, " ");
    newLine = regex_replace(newLine, startSpacesRegex, "");

    goodLines.push_back(newLine);

  }

  for(string s: goodLines){
    this->outputFile << s << endl;
  }

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

    Assembler assembler(outputFile, inputFile);

    if(!assembler.openFiles()){
      throw NonexistantInputFileException();
    }

    assembler.setGoodLines();
    
  }
  catch(const std::exception& e){
    std::cerr << e.what() << '\n';
  }

  return 0;
}