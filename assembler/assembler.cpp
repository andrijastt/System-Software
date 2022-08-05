#include "assembler.hpp"
#include "exceptions.hpp"

int Assembler::sectionId = 0;
int Assembler::symbolId = 0;

Assembler::Assembler(string outputFile, string inputFile) throw(){
  this->outputFileString = outputFile;
  this->inputFileString = inputFile;

  Section section;
  section.id = sectionId++;
  section.base = 0;
  section.length = 0;
  section.name = "UND";
  sectionTable.push_back(section);
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
    newLine = regex_replace(newLine, endSpacesRegex, "");

    goodLines.push_back(newLine);

  }

}

int Assembler::pass(){

  int locationCounter = 0;
  int locationCounterGlobal = 0;
  int globalBase = 0;
  Section currentSection;
  currentSection.name = "";

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

    // global directive
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

    // extern directive
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

    // section directive
    if(regex_search(s, m, sectionRegex)){
      this->outputFile << "Found section directive: " << m.str(0) << endl;

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);              // remove section from symbols
      s1 = m1.suffix().str();

      this->outputFile << "Section name: " << s1 << endl;

      Section section;
      section.id = sectionId++;
      section.name = s1;
      section.base = locationCounterGlobal;

      if(currentSection.name == ""){
        currentSection = section;
      } else {
        currentSection.length = locationCounter;
        sectionTable.push_back(currentSection);
        currentSection = section;
        locationCounter = 0;
      }
    }

    // word directive
    if(regex_search(s, m, wordRegex)){
      this->outputFile << "Found word directive: " << m.str(0) << endl;

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);              // remove word from symbols
      s1 = m1.suffix().str();


      int cnt = 0;
      while(regex_search(s1, m1, symbolOrLiteralRegex)){
        cnt++;
        this->outputFile << "Symbol or Literal val: " << m1.str(0) << endl;
        s1 = m1.suffix().str();
      }

      locationCounter += cnt*2;
      locationCounterGlobal += cnt*2;

    }

    // skip directive
    if(regex_search(s, m, skipRegex)){
      this->outputFile << "Found skip directive: " << m.str(0) << endl;

      smatch m1;
      regex_search(s, m1, literalRegex);              
      string literal = m1.str(0);

      this->outputFile << "Literal: " << literal << endl;      

      int num = stoi(literal);
      locationCounter += num;
      locationCounterGlobal += num;

    }

    // end directive
    if(regex_search(s, m, endRegex)){
      this->outputFile << "Found end directive: " << m.str(0) << endl;
    }

    // no operand isntruction
    if(regex_search(s, m, noOperandsInstructions)){
      this->outputFile << "Found instruction with no operands: " << m.str(0) << endl;

      locationCounter++;
      locationCounterGlobal++;
      
    }

    // one register instruction
    if(regex_search(s, m, oneRegisterInsturctions)){
      this->outputFile << "Found instruction with one register as operand: " << m.str(0) << endl;
      
      smatch m1;
      regex_search(s, m1, symbolRegex);
      string instruction = m1.str(0);

      this->outputFile << "Insturction: " << instruction << endl;

      if(instruction == "iret" || instruction == ""){
        locationCounter+=2;
        locationCounterGlobal+=2;
      }

      regex_search(s, m1, registersRegex);
      this->outputFile << "Register found: " << m1.str(0) << endl;
    }

    if(regex_search(s, m, twoRegistersInstructions)){
      this->outputFile << "Found instruction with two registers as operands: " << m.str(0) << endl;

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);                // remove instruction name
      string instruction = m1.str(0);
      s1 = m1.suffix().str();

      this->outputFile << "Instruction: " << instruction << endl;

      regex_search(s, m1, registersRegex);
      string r1 = m1.str(0);
      this->outputFile << "Register found: " << m1.str(0) << endl;
      s = m1.suffix().str();

      regex_search(s, m1, registersRegex);
      string r2 = m1.str(0);
      this->outputFile << "Register found: " << m1.str(0) << endl;
      s = m1.suffix().str();

      locationCounter+=2;
      locationCounterGlobal+=2;

    }

    if(regex_search(s, m, oneOperandInstructions)){
      this->outputFile << "Found instruction with one operand: " << m.str(0) << endl;

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);                // remove instruction name
      string instruction = m1.str(0);
      s1 = m1.suffix().str();
      this->outputFile << "Instruction: " << instruction << endl;
      this->outputFile << "Operand: " << s1 << endl;

      // register direct
      if(regex_search(s1, m1, registerDirectJumpRegex)){
        this->outputFile << "Jump Register direct value found!" << endl;
        continue;
      }

      // PC REL with symbol
      if(regex_search(s1, m1, pcRelSymbolJumpRegex)){
        this->outputFile << "Jump PC REL with symbol found!" << endl;
        continue;
      }

      // register indirect with literal
      if(regex_search(s1, m1, registerIndirectLiteralJumpRegex)){
        this->outputFile << "Jump Register indirect with literal value found!" << endl;
        continue;
      }

      // register indirect with symbol
      if(regex_search(s1, m1, registerIndirectSymbolJumpRegex)){
        this->outputFile << "Jump Register indirect with symbol value found!" << endl;
        continue;
      }

      // register indirect
      if(regex_search(s1, m1, registerIndirectJumpRegex)){
        this->outputFile << "Jump Register indirect value found!" << endl;
        continue;
      }

      // memory value literal
      if(regex_search(s1, m1, valueMemLiteralJumpRegex)){
        this->outputFile << "Jump Memory literal value found!" << endl;
        continue;
      }

      // memory value symbol
      if(regex_search(s1, m1, valueMemSymbolJumpRegex)){
        this->outputFile << "Jump Memory symbol value found!" << endl;
        continue;
      }

      // literal value
      if(regex_search(s1, m1, literalRegex)){
        this->outputFile << "Jump literal value found!" << endl;
        continue;
      }

      // symbol value
      if(regex_search(s1, m1, symbolOnlyRegex)){
        this->outputFile << "Jump symbol value found!" << endl;
        continue;
      }

    }

    if(regex_search(s, m, oneOperandOneRegisterInstructions)){
      this->outputFile << "Found instruction with one operand and one register: " << m.str(0) << endl;

      smatch m1;
      string s1 = m.str(0);

      regex_search(s1, m1, symbolOnlyRegex);            
      string instruction = m1.str(0);

      regex_search(s1, m1, symbolRegex);                // remove instruction name
      instruction = m1.str(0);
      s1 = m1.suffix().str();

      this->outputFile << "Instruction: " << instruction << endl;
      this->outputFile << "Operands: " << s1 << endl;

      regex_search(s1, m1, registersRegex);
      string registerFound = m1.str(0);
      s1 = m1.suffix().str();

      this->outputFile << "Register found: " << registerFound << endl;

      if(regex_search(s1, m1, commaRegex)){
        s1 = m1.suffix().str();
      }

      // PC REL with symbol
      if(regex_search(s1, m1, pcRelSymbolDataRegex)){
        this->outputFile << "PC REL with symbol found!" << endl;
        continue;
      }

      // memory value literal
      if(regex_search(s1, m1, valueLiteralDataRegex)){
        this->outputFile << "Literal value found!" << endl;
        continue;
      }

      // memory value symbol
      if(regex_search(s1, m1, valueSymbolDataRegex)){
        this->outputFile << "Symbol value found!" << endl;
        continue;
      }

      // register direct
      if(regex_search(s1, m1, registersRegex)){
        this->outputFile << "Register direct value found!" << endl;
        s1 = m1.suffix().str();
      }

      // register indirect
      if(regex_search(s1, m1, registerIndirectDataRegex)){
        this->outputFile << "Register indirect value found!" << endl;
        continue;
      }

      // register indirect with literal
      if(regex_search(s1, m1, registerIndirectLiteralDataRegex)){
        this->outputFile << "Register indirect with literal value found!" << endl;
        continue;
      }

      // register indirect with symbol
      if(regex_search(s1, m1, registerIndirectSymbolDataRegex)){
        this->outputFile << "Register indirect with literal value found!" << endl;
        continue;
      }

      // literal value
      if(regex_search(s1, m1, literalRegex)){
        this->outputFile << "Memory literal value found!" << endl;
        continue;
      }

      // symbol value
      if(regex_search(s1, m1, symbolRegex)){
        if(!regex_search(s1, m1, registersRegex))
          this->outputFile << "Memory symbol value found!" << endl;
        continue;
      }
      
    }

  }

  currentSection.length = locationCounter;
  sectionTable.push_back(currentSection);
  locationCounter = 0;

  this->outputFile << endl;
  this->outputFile << endl;
  this->outputFile << endl;
  this->outputFile << endl;

  this->outputFile << "SECTION TABLE\n";
  this->outputFile << "ID" << "\t" << "BASE" << "\t" << "LENGTH" << "\t" << "NAME" << "\n";
  for(Section s: sectionTable){
    this->outputFile << s.id << "\t" << s.base << "\t" << s.length << "\t" << s.name << "\n";
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
    if(assembler.pass() < 0){
      throw BadSyntaxException();
    }
    
  }
  catch(const std::exception& e){
    std::cerr << e.what() << '\n';
  }

  return 0;
}