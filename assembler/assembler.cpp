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

  // for(string s: goodLines){
  //   this->outputFile << s << "\n";
  // }

}

int Assembler::pass(){

  for(string s: goodLines){
    smatch m;
    bool err = true;

    // line is emtpy
    if(s == ""){
      this->outputFile << "Skipped line" << endl;
      continue;
    }

    // find label
    if(regex_search(s, m, labelRegex)){

      this->outputFile << "Found label: " << m.str(0) << endl;
      
      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);
      this->outputFile << "Label name: " << m1.str(0) << endl;     // we take label name and check if symbol exists, is it duplcate...

      s = m.suffix().str();                            // remove label from the string
    }

    if(regex_search(s, m, globalRegex)){
      this->outputFile << "Found global directive: " << m.str(0) << endl;

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);              // remove global from symbols
      s1 = m1.suffix().str();

      while(regex_search(s1, m1, symbolRegex)){
        this->outputFile << "Symbol name: " << m1.str(0) << endl;
        s1 = m1.suffix().str();
      }
      
    }

    if(regex_search(s, m, externRegex)){
      this->outputFile << "Found extern directive: " << m.str(0) << endl;

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);              // remove extern from symbols
      s1 = m1.suffix().str();

      while(regex_search(s1, m1, symbolRegex)){
        this->outputFile << "Symbol name: " << m1.str(0) << endl;
        s1 = m1.suffix().str();
      }
    }

    if(regex_search(s, m, sectionRegex)){
      this->outputFile << "Found section directive: " << m.str(0) << endl;

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);              // remove section from symbols
      s1 = m1.suffix().str();

      this->outputFile << "Section name: " << s1 << endl;
    }

    if(regex_search(s, m, wordRegex)){
      this->outputFile << "Found word directive: " << m.str(0) << endl;

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);              // remove word from symbols
      s1 = m1.suffix().str();

      while(regex_search(s1, m1, symbolOrLiteralRegex)){
        this->outputFile << "Symbol or Literal val: " << m1.str(0) << endl;
        s1 = m1.suffix().str();
      }

    }

    // ovo nesto ne radi lepo
    if(regex_search(s, m, skipRegex)){
      this->outputFile << "Found skip directive: " << m.str(0) << endl;

      smatch m1;
      regex_search(s, m1, literalRegex);              

      this->outputFile << "Literal: " << m1.str(0) << endl;              
    }

    if(regex_search(s, m, endRegex)){
      this->outputFile << "Found end directive: " << m.str(0) << endl;
    }

  }

  return 0;
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
    assembler.pass();
    
  }
  catch(const std::exception& e){
    std::cerr << e.what() << '\n';
  }

  return 0;
}