#include "assembler.hpp"
#include "exceptions.hpp"

int Assembler::sectionId = 0;
int Assembler::symbolId = 0;

/**
 * @brief Construct a new Assembler:: Assembler object
 * 
 * @param outputFile file where we output assembler pass
 * @param inputFile file that we are reading
 */
Assembler::Assembler(string outputFile, string inputFile) throw(){
  this->outputFileString = outputFile;
  this->inputFileString = inputFile;

  Section section;
  section.id = sectionId++;
  section.base = 0;
  section.length = 0;
  section.name = "UND";
  sectionTable.push_back(section);

  symbolTable.clear();
  Symbol symbol;
  symbol.id = symbolId++;
  symbol.name = "";
  symbol.offset = 0;
  symbol.sectionId = 0;
  symbol.value = 0;
  symbol.size = 0;
  symbol.type = NOTYP;
  symbol.bind = LOCAL;
  symbol.defined = false;
  symbol.type = NOTYP;
  symbolTable.push_back(symbol);

}

/**
 * @brief we open files assigned in constructor
 * 
 * @return true if input file exists, it is good
 * @return false if input file doesn't exist, exception
 */
bool Assembler::openFiles(){
  this->inputFile.open(inputFileString, ios::in);
  this->outputFile.open(outputFileString, ios::out|ios::trunc);

  if(!this->inputFile.is_open()){
    return false;
  } else {
    return true;
  }
}

/**
 * @brief we pass through the input file and remove comments, tabs, extra spaces, etc.
 * 
 */
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

/**
 * @brief One and only pass of assembler
 * 
 * @return int 0 - it is good, -1 - syntax error, -2 - some labels or code are not in section
 */
int Assembler::pass(){

  int locationCounter = 0;
  int locationCounterGlobal = 0;
  int globalBase = 0;
  Section currentSection;
  currentSection.name = "";

  int currentSectionId = -1;

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
      string labelName = m1.str(0);
      this->outputFile << "Label name: " << m1.str(0) << endl;      // we take label name and check if symbol exists, is it duplcate...
      s = m.suffix().str();                                         // remove label from the string

      if(currentSectionId == -1){
        return -2;
      }

      bool found = false;
      int i = 0;
      for(Symbol sym: symbolTable){
        if(sym.name == labelName){
          if(sym.defined){
            return -1;
          }

          sym.defined = true;
          sym.value = 0;
          sym.offset = locationCounter;
          sym.sectionId = currentSectionId;
          found = true;

          symbolTable.at(i) = sym;

          break;
        }
        i++;
      }

      if(!found){
        Symbol sym;
        sym.name = labelName;
        sym.value = 0;
        sym.defined = true;
        sym.bind = LOCAL;
        sym.offset = locationCounter;
        sym.type = NOTYP;
        sym.id = symbolId++;
        sym.sectionId = currentSectionId;
        symbolTable.push_back(sym);
      }

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
        string symbolName = m1.str(0);
        s1 = m1.suffix().str();

        bool found = false;
        for(Symbol sym: symbolTable){
          if(sym.name == symbolName){
            sym.bind = GLOBAL;
            found = true;
            break;
          }
        }

        if(!found){
          Symbol sym;
          sym.name = symbolName;
          sym.defined = false;
          sym.size = 0;
          sym.value = 0;
          sym.type = NOTYP;
          sym.bind = GLOBAL;
          sym.offset = locationCounter;
          sym.id = symbolId++;
          symbolTable.push_back(sym);
        }
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
        string symbolName = m1.str(0);
        s1 = m1.suffix().str();

        bool found = false;
        for(Symbol sym: symbolTable){
          if(sym.name == symbolName){
            sym.bind = GLOBAL;
            found = true;
            break;
          }
        }

        if(!found){
          Symbol sym;
          sym.name = symbolName;
          sym.defined = false;
          sym.size = 0;
          sym.value = 0;
          sym.type = NOTYP;
          sym.bind = GLOBAL;
          sym.offset = locationCounter;
          sym.id = symbolId++;
          symbolTable.push_back(sym);
        }
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

      bool found = false;
      for(Symbol sym: symbolTable){
        if(sym.name == s1){
          return -1;            // mislim da ne moze da postoje vise sekcija sa isitm imenom ili labela i sekcija sa istim imenom
        }
      }

      if(!found){
        Symbol sym;
        sym.name = s1;
        sym.defined = false;
        sym.size = 0;
        sym.value = 0;
        sym.type = SCTN;
        sym.bind = GLOBAL;
        currentSectionId = sym.sectionId = sym.id = symbolId++;
        sym.offset = locationCounter;
        symbolTable.push_back(sym);
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
      regex_search(s, m1, symbolOnlyRegex);
      string instruction = m1.str(0);

      this->outputFile << "Insturction: " << instruction << endl;

      if(instruction == "push" || instruction == "pop"){
        locationCounter+=3;
        locationCounterGlobal+=3;
      } else {
        if (instruction == "int" || instruction == "not"){
          locationCounter+=2;
          locationCounterGlobal+=2;
        }
      }

      regex_search(s, m1, registersRegex);
      this->outputFile << "Register found: " << m1.str(0) << endl;
    }

    if(regex_search(s, m, twoRegistersInstructions)){
      this->outputFile << "Found instruction with two registers as operands: " << m.str(0) << endl;

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolOnlyRegex);                // remove instruction name
      string instruction = m1.str(0);
      s1 = m1.suffix().str();

      regex_search(instruction, m1, symbolOnlyRegex);                // remove instruction name
      instruction = m1.str(0);

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

      regex_search(instruction, m1, symbolOnlyRegex);                // remove instruction name
      instruction = m1.str(0);

      this->outputFile << "Instruction: " << instruction << endl;
      this->outputFile << "Operand: " << s1 << endl;

      // register direct
      if(regex_search(s1, m1, registerDirectJumpRegex)){
        this->outputFile << "Jump Register direct value found!" << endl;
        locationCounter+=3;
        locationCounterGlobal+=3;
        continue;
      }

      // PC REL with symbol
      if(regex_search(s1, m1, pcRelSymbolJumpRegex)){
        this->outputFile << "Jump PC REL with symbol found!" << endl;
        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // register indirect with literal
      if(regex_search(s1, m1, registerIndirectLiteralJumpRegex)){
        this->outputFile << "Jump Register indirect with literal value found!" << endl;
        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // register indirect with symbol
      if(regex_search(s1, m1, registerIndirectSymbolJumpRegex)){
        this->outputFile << "Jump Register indirect with symbol value found!" << endl;
        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // register indirect
      if(regex_search(s1, m1, registerIndirectJumpRegex)){
        this->outputFile << "Jump Register indirect value found!" << endl;
        locationCounter+=3;
        locationCounterGlobal+=3;
        continue;
      }

      // memory value literal
      if(regex_search(s1, m1, valueMemLiteralJumpRegex)){
        this->outputFile << "Jump Memory literal value found!" << endl;
        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // memory value symbol
      if(regex_search(s1, m1, valueMemSymbolJumpRegex)){
        this->outputFile << "Jump Memory symbol value found!" << endl;
        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // literal value
      if(regex_search(s1, m1, literalRegex)){
        this->outputFile << "Jump literal value found!" << endl;
        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // symbol value
      if(regex_search(s1, m1, symbolOnlyRegex)){
        this->outputFile << "Jump symbol value found!" << endl;
        locationCounter+=5;
        locationCounterGlobal+=5;
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

      regex_search(instruction, m1, symbolOnlyRegex);                // remove instruction name
      instruction = m1.str(0);

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
        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // memory value literal
      if(regex_search(s1, m1, valueLiteralDataRegex)){
        this->outputFile << "Literal value found!" << endl;
        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // memory value symbol
      if(regex_search(s1, m1, valueSymbolDataRegex)){
        this->outputFile << "Symbol value found!" << endl;
        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // register direct
      if(regex_search(s1, m1, registersRegex)){
        this->outputFile << "Register direct value found!" << endl;
        s1 = m1.suffix().str();
        locationCounter+=3;
        locationCounterGlobal+=3;
        continue;
      }

      // register indirect
      if(regex_search(s1, m1, registerIndirectDataRegex)){
        this->outputFile << "Register indirect value found!" << endl;
        locationCounter+=3;
        locationCounterGlobal+=3;
        continue;
      }

      // register indirect with literal
      if(regex_search(s1, m1, registerIndirectLiteralDataRegex)){
        this->outputFile << "Register indirect with literal value found!" << endl;
        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // register indirect with symbol
      if(regex_search(s1, m1, registerIndirectSymbolDataRegex)){
        this->outputFile << "Register indirect with literal value found!" << endl;
        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // literal value
      if(regex_search(s1, m1, literalRegex)){
        this->outputFile << "Memory literal value found!" << endl;
        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // symbol value
      if(regex_search(s1, m1, symbolRegex)){
        if(!regex_search(s1, m1, registersRegex)){
          locationCounter+=5;
          locationCounterGlobal+=5;
          this->outputFile << "Memory symbol value found!" << endl;
        }
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

  this->outputFile << endl;
  this->outputFile << endl;
  this->outputFile << endl;
  this->outputFile << endl;

  this->outputFile << "SYMBOL TABLE\n";
  this->outputFile << "Num\tValue\tSize\tType\tBind\tNdx\tName\n";
  for(Symbol sym: symbolTable){
    this->outputFile << sym.id << "\t" << std::hex << sym.value << "\t" << std::dec << sym.size << "\t"; 

    switch(sym.type){

      case NOTYP:
        this->outputFile << "NOTYP\t";
        break;
      
      case SCTN:
        this->outputFile << "SCTN\t";
        break;
    }

    switch(sym.bind){
      case GLOBAL:
        this->outputFile << "GLOB\t";
        break;

      case LOCAL:
        this->outputFile << "LOC\t";
        break;

      case UND:
        this->outputFile << "UND\t";
        break;
    }

    switch(sym.sectionId){

      case 0:
        this->outputFile << "UND\t";
        break;

      default:
        this->outputFile << sym.sectionId << "\t";
        break;
    }

    this->outputFile << sym.name << "\n";
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
    int ret = assembler.pass();
    if(ret == -1){
      throw BadSyntaxException();
    }
    if(ret == -2){
      throw NoSectionException();
    }

    
  }
  catch(const std::exception& e){
    std::cerr << e.what() << '\n';
  }

  return 0;
}