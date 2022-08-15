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

  Symbol symbol;
  symbol.id = symbolId++;
  symbol.name = "UND";
  symbol.offset = 0;
  symbol.sectionId = 0;
  symbol.value = 0;
  symbol.size = 0;
  symbol.type = NOTYP;
  symbol.bind = NOBIND;
  symbol.defined = false;
  symbol.type = NOTYP;
  symbolTable.push_back(symbol);

}

/**
 * @brief turns hex number to machine code
 * 
 * @param num the number that need to be changed
 * @return vector<string> helper machine code
 */
vector<string> Assembler::hexToCode(string num){

  vector<string> ret;
  if(num.size() == 4){
    ret.push_back(num.substr(0,2));
    ret.push_back(num.substr(2,2));
  } else {
    if(num.size() == 3){
      ret.push_back("0" + num.substr(0,1));
      ret.push_back(num.substr(2,2));
    } else {
      if(num.size() == 2){
        ret.push_back("00");
        ret.push_back(num.substr(0,2));
      } else {
        ret.push_back("00");
        ret.push_back("0" + num);
      }
    }
  }
  return ret;
}

/**
 * @brief turns dec number to machine code
 * 
 * @param num the number that need to be changed
 * @return vector<string> helper machine code
 */
vector<string> Assembler::decToCode(string num){
  vector<string> ret;
  
  int n = stoi(num);
  char help[8];
  sprintf(help, "%X", n);
  string help1 = (string)help;

  if(help1.size() == 4){
    ret.push_back(help1.substr(0,2));
    ret.push_back(help1.substr(2,2));
  } else {
    if(help1.size() == 3){
      ret.push_back("0" + help1.substr(0,1));
      ret.push_back(help1.substr(2,2));
    } else {
      if(help1.size() == 2){
        ret.push_back("00");
        ret.push_back(help1.substr(0,2));
      } else {
        if(help1.size() == 1){
          ret.push_back("00");
          ret.push_back("0" + help1);
        } else {  // here it means it is negative number
          ret.push_back(help1.substr(4,2));
          ret.push_back(help1.substr(6,2));
        }        
      }
    }
  }

  return ret;
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
 * @brief search for symbol and search if symbol is defined
 * 
 * @param symbolName symbol's name
 * @return int -1 - no symbol, num - symbol found
 */
int Assembler::searchSymbol(string symbolName){

  int cnt = 0;
  for(Symbol sym: symbolTable){
    if(sym.name == symbolName){
      return cnt;
    }
    cnt++;
  }
  return -1;
}

/**
 * @brief  prints assembler output
 * 
 */
void Assembler::printOutput(){

  size_t lastindex = outputFileString.find_last_of(".");
  string outputLinker = outputFileString.substr(0, lastindex) + "Linker.o";
  ofstream outputLinkerStream;
  outputLinkerStream.open(outputLinker, ios::out|ios::trunc);

  this->outputFile << "SECTION TABLE\n";
  this->outputFile << "ID" << "\t" << "LENGTH" << "\t" << "NAME" << "\n";
  outputLinkerStream << "SECTIONS\n";
  for(Section s: sectionTable){
    this->outputFile << s.id << "\t" << s.length << "\t" << s.name << "\n";
    outputLinkerStream << s.id << "\t" << s.length << "\t" << s.name << "\n";
  }

  this->outputFile << endl;
  outputLinkerStream << endl;

  this->outputFile << "SYMBOL TABLE\n";
  this->outputFile << "Num\tValue\tType\tBind\tNdx\tName\tDefined\n";
  outputLinkerStream << "SYMBOLS\n";
  for(Symbol sym: symbolTable){
    this->outputFile << sym.id << "\t" << std::setfill('0') << std::setw(4) << std::hex << sym.offset << std::dec << "\t"; 
    outputLinkerStream << sym.id << "\t" << std::setfill('0') << std::setw(4) << std::hex << sym.offset << std::dec << "\t"; 

    switch(sym.type){

      case NOTYP:
        this->outputFile << "NOTYP\t";
        outputLinkerStream << "NOTYP\t";
        break;
      
      case SCTN:
        this->outputFile << "SCTN\t";
        outputLinkerStream << "SCTN\t";
        break;
    }

    switch(sym.bind){
      case GLOBAL:
        this->outputFile << "GLOB\t";
        outputLinkerStream << "GLOB\t";
        break;

      case LOCAL:
        this->outputFile << "LOC\t";
        outputLinkerStream << "LOC\t";
        break;

      case NOBIND:
        this->outputFile << "NOBIND\t";
        outputLinkerStream << "NOBIND\t";
        break;
    }

    switch(sym.sectionId){

      case 0:
        this->outputFile << "UND\t";
        outputLinkerStream << "UND\t";
        break;

      default:
        this->outputFile << sym.sectionId << "\t";
        outputLinkerStream << sym.sectionId << "\t";
        break;
    }

    this->outputFile << sym.name << "\t";
    outputLinkerStream << sym.name << "\t";

    switch(sym.defined){

      case true:
        this->outputFile << "DEF\n";
        outputLinkerStream << "DEF\n";
        break;

      case false:
        this->outputFile << "UND\n";
        outputLinkerStream << "UND\n";
        break;
    }
  }

  this->outputFile << endl;
  outputLinkerStream << endl;

  outputLinkerStream << "RELOCATIONS\nUND\n";
  for(Section sec: sectionTable){
    this->outputFile << "Relocation table <" << sec.name << ">\n";
    this->outputFile << "Offset\tType\tSymbol ID\tAddend\n";

    for(vector<Relocation> rel: relocationTable){
      if(rel.size() == 0) continue;
      if(rel.at(0).sectionId != sec.id){
        continue;
      }

      outputLinkerStream << sectionTable.at(rel.at(0).sectionId).name << endl;
      for(Relocation relocations: rel){
        this->outputFile << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << relocations.offset << std::dec << "\t";
        outputLinkerStream << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << relocations.offset << std::dec << "\t";

        switch(relocations.type){
          case R_16:
            this->outputFile << "R_16\t";
            outputLinkerStream << "R_16\t";
            break;
          case R_PC16:
            this->outputFile << "R_PC16\t";
            outputLinkerStream << "R_PC16\t";
            break;
        }
        this->outputFile << relocations.symbolId << "\t";
        outputLinkerStream << relocations.symbolId << "\t";

        if(relocations.addend >= 0){
          this->outputFile << std::setfill('0') << ::setw(4) << std::hex << relocations.addend << std::dec <<"\n";
          outputLinkerStream << std::setfill('0') << ::setw(4) << std::hex << relocations.addend << std::dec <<"\n";
        } else {
          string help = to_string(relocations.addend);
          vector<string> help1 = decToCode(help);
          for(string s: help1){
            this->outputFile << s;
            outputLinkerStream << s;
          }
          this->outputFile << endl;
          outputLinkerStream << endl;
        }
      }

    }
    this->outputFile << endl;
    outputLinkerStream << endl;
  }

  this->outputFile << endl;
  outputLinkerStream << endl;

  outputLinkerStream << "MACHINE CODE\n";
  for(Section sec: sectionTable){
    this->outputFile << "Machine code <" << sec.name << ">\n";
    outputLinkerStream << sec.name << "\n";

    for(vector<MachineCode> mcodes: machineCode){
      if(mcodes.at(0).sectionName != sec.name || sec.name == "UND" || mcodes.size() == 0){
        continue;
      }

      int i = 0;
      for(MachineCode mcode: mcodes){

        if(i % 8 == 0){
          this->outputFile << endl << std::setfill('0') << std::setw(4) << std::hex << i << std::dec << ": ";
        }

        this->outputFile << mcode.value << " ";
        outputLinkerStream << mcode.value << " ";
        i++;
      }

    }
    this->outputFile << endl;
    outputLinkerStream << endl;
  }

  this->outputFile << endl;
  outputLinkerStream << endl << "END";

  for(Symbol symb: symbolTable){
    this->outputFile << "Forward table <" << symb.name << ">\n";
    this->outputFile << "Forwarding_type\tSection ID\tOffset\tOffset Relocation\n" ;

    for(Forwarding fw: symb.forwardingTable){
      switch(fw.type){
        case TEXT:
          this->outputFile << "TEXT\t";
          break;

        case RELO:
          this->outputFile << "RELO\t";
          break;

        case DATA:
          this->outputFile << "DATA\t";
          break;
      }

      this->outputFile << fw.sectionID << "\t" << fw.offset << "\t" << fw.offsetRelo << endl;
    }

    this->outputFile << endl;
  }

}

void Assembler::backPatchingRelocation(Symbol sym){
  int j = 0;
  for(vector<Relocation> relTable: relocationTable){
    int i = 0;
    for(Relocation rel: relTable){
      if(rel.symbolId == sym.id){
        if(sym.bind == GLOBAL){
          rel.addend = 0;
        } else {
          rel.addend = sym.offset;
          rel.symbolId = sym.sectionId;
        }
        relTable.at(i) = rel;
      }
      i++;
    }
    relocationTable.at(j) = relTable;
    j++;
  }
}

/**
 * @brief One and only pass of assembler
 * 
 * @return int 0 - it is good, -1 - syntax error, -2 - some labels or code are not in section
 */
int Assembler::pass(){

  if(!openFiles()){
    return -3;
  }
  setGoodLines();

  size_t lastindex = outputFileString.find_last_of(".");
  string outputFileHelpString = outputFileString.substr(0, lastindex) + "Helper.o";
  ofstream outputHelp;
  outputHelp.open(outputFileHelpString, ios::out|ios::trunc);

  int locationCounter = 0;
  int locationCounterGlobal = 0;
  int globalBase = 0;
  Section currentSection;
  currentSection.name = "";

  int currentSectionId = -1;
  vector<MachineCode> currentSectionMachineCode;
  vector<Relocation> currentRelocationTable;

  for(string s: goodLines){
    smatch m;
    bool err = true;

    // line is emtpy
    if(s == ""){
      outputHelp << "Skipped line" << endl;
      continue;
    }

    // find label
    if(regex_search(s, m, labelRegex)){

      outputHelp << "Found label: " << m.str(0) << endl;
      
      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);
      string labelName = m1.str(0);
      outputHelp << "Label name: " << labelName << endl;      // we take label name and check if symbol exists, is it duplcate...
      s = m.suffix().str();                                   // remove label from the string

      if(m.prefix().str() != ""){
        return -1;
      }

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
          if(sym.bind == NOBIND) sym.bind = LOCAL;

          symbolTable.at(i) = sym;

          int j = 0;
          for(Relocation rel: currentRelocationTable){
            if(rel.symbolId == sym.id && rel.type == R_PC16){
              rel.type = R_16;
              currentRelocationTable.at(j) = rel;
              break;
            }
            j++;
          }

          currentSectionMachineCode = backPatching(sym, currentSection.id, locationCounter, currentSectionMachineCode);
          backPatchingRelocation(sym);

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

      outputHelp << "What is left after removing label: " << s << endl;

    }

    // global directive
    if(regex_search(s, m, globalRegex)){
      outputHelp << "Found global directive: " << m.str(0) << endl;

      if(m.prefix().str() != "" || m.suffix().str() != ""){
        return -1;
      }

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);              // remove global from symbols
      s1 = m1.suffix().str();

      while(regex_search(s1, m1, symbolRegex)){
        outputHelp << "Symbol name: " << m1.str(0) << endl;
        string symbolName = m1.str(0);
        s1 = m1.suffix().str();

        bool found = false;
        int i = 0;
        for(Symbol sym: symbolTable){
          if(sym.name == symbolName){
            sym.bind = GLOBAL;
            symbolTable.at(i) = sym;
            found = true;
            break;
          }
          i++;
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
      s = m.suffix().str();
      
    }

    // extern directive
    if(regex_search(s, m, externRegex)){
      outputHelp << "Found extern directive: " << m.str(0) << endl;

      if(m.prefix().str() != "" || m.suffix().str() != ""){
        return -1;
      }

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);              // remove extern from symbols
      s1 = m1.suffix().str();

      while(regex_search(s1, m1, symbolRegex)){
        outputHelp << "Symbol name: " << m1.str(0) << endl;
        string symbolName = m1.str(0);
        s1 = m1.suffix().str();

        bool found = false;
        int i = 0;
        for(Symbol sym: symbolTable){
          if(sym.name == symbolName){
            sym.bind = GLOBAL;
            symbolTable.at(i) = sym;
            found = true;
            break;
          }
          i++;
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
      s = m.suffix().str();
    }

    // section directive
    if(regex_search(s, m, sectionRegex)){
      outputHelp << "Found section directive: " << m.str(0) << endl;

      if(m.prefix().str() != "" || m.suffix().str() != ""){
        return -1;
      }

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);              // remove section from symbols
      s1 = m1.suffix().str();

      outputHelp << "Section name: " << s1 << endl;

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
        this->machineCode.push_back(currentSectionMachineCode);
        this->relocationTable.push_back(currentRelocationTable);
        currentSectionMachineCode.clear();
        currentRelocationTable.clear();
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
        sym.bind = NOBIND;
        currentSectionId = sym.sectionId = sym.id = symbolId++;
        sym.offset = locationCounter;
        symbolTable.push_back(sym);
      }
      s = m.suffix().str();
    }

    // word directive
    if(regex_search(s, m, wordRegex)){
      outputHelp << "Found word directive: " << m.str(0) << endl;

      if(currentSectionId == -1){
        return -2;
      }
      if(m.prefix().str() != "" || m.suffix().str() != ""){
        return -1;
      }

      smatch m1;
      string s1 = m.str(0);
      s = m.suffix().str();
      regex_search(s1, m1, symbolRegex);              // remove word from symbols
      s1 = m1.suffix().str();

      int cnt = 0;
      while(regex_search(s1, m1, symbolOrLiteralRegex)){
        locationCounter += 2;
        locationCounterGlobal += 2;
        outputHelp << "Symbol or Literal val: " << m1.str(0) << endl;
        string val = m1.str(0);
        s1 = m1.suffix().str();

        if(regex_search(val, m1, literalRegex)){  //

          if(regex_search(val, m1, hexRegex)){
            string num2 = m1.str(0);
            regex_search(num2, m1, hexRemoveRegex);
            num2 = m1.suffix().str();
            vector<string> help = hexToCode(num2);

            for(string s: help){
              currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
            }

          } else {
            string num2 = val;
            vector<string> help = decToCode(num2);

            for(string s: help){
              currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
            }
          }

        } else {
          if(regex_search(val, m1, symbolOnlyRegex)){       // add to forward if needed
            currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
            currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
            string symName = m1.str(0);
            int ret = searchSymbol(symName);
            int endSize = currentSectionMachineCode.size() - 1;
            int startSize = currentSectionMachineCode.size() - 2;
            int size = currentRelocationTable.size();
            currentRelocationTable = addSymbolOrForwardElement(ret, symName, currentSectionId, locationCounter, currentSection, 
            currentRelocationTable, false, startSize, endSize);

            if(size == currentRelocationTable.size() && ret != -1){
              int mov = locationCounter - symbolTable.at(ret).offset - 1;
              string mov1 = to_string(mov);
              vector<string> help = decToCode(mov1);
              bool done = false;
              for(string s: help){
                if(!done){
                  currentSectionMachineCode.at(startSize).value = s;
                  done = true;
                } else {
                  currentSectionMachineCode.at(endSize).value = s;
                }
              }
            }

          }
        } 

      }  
    }

    // wordOnly directive
    if(regex_search(s, m, wordOnlyRegex)){
      outputHelp << "Found word directive: " << m.str(0) << endl;

      if(currentSectionId == -1){
        return -2;
      }
      if(m.prefix().str() != ""){
        return -1;
      }

      smatch m1;
      s = m.suffix().str();

      int cnt = 0;
      while(regex_search(s, m1, symbolOrLiteralRegex)){
        locationCounter += 2;
        locationCounterGlobal += 2;
        outputHelp << "Symbol or Literal val: " << m1.str(0) << endl;
        string val = m1.str(0);
        s = m1.suffix().str();

        if(regex_search(val, m1, literalRegex)){  //

          if(regex_search(val, m1, hexRegex)){
            string num2 = m1.str(0);
            regex_search(num2, m1, hexRemoveRegex);
            num2 = m1.suffix().str();
            vector<string> help = hexToCode(num2);

            for(string s: help){
              currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
            }

          } else {
            string num2 = val;
            vector<string> help = decToCode(num2);

            for(string s: help){
              currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
            }
          }

        } else {
          if(regex_search(val, m1, symbolOnlyRegex)){       // add to forward if needed
            currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
            currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
            string symName = m1.str(0);
            int ret = searchSymbol(symName);
            int endSize = currentSectionMachineCode.size() - 1;
            int startSize = currentSectionMachineCode.size() - 2;
            int size = currentRelocationTable.size();
            currentRelocationTable = addSymbolOrForwardElement(ret, symName, currentSectionId, locationCounter, currentSection, 
            currentRelocationTable, false, startSize, endSize);

            if(size == currentRelocationTable.size() && ret != -1){
              int mov = locationCounter - symbolTable.at(ret).offset - 1;
              string mov1 = to_string(mov);
              vector<string> help = decToCode(mov1);
              bool done = false;
              for(string s: help){
                if(!done){
                  currentSectionMachineCode.at(startSize).value = s;
                  done = true;
                } else {
                  currentSectionMachineCode.at(endSize).value = s;
                }
              }
            }

          }
        }

      }  
    }

    // skip directive
    if(regex_search(s, m, skipRegex)){
      outputHelp << "Found skip directive: " << m.str(0) << endl;
      s = m.suffix().str();

      if(currentSectionId == -1){
        return -2;
      }
      if(m.prefix().str() != ""){
        return -1;
      }

      smatch m1;
      regex_search(s, m1, literalRegex);              
      string literal = m1.str(0);
      s = m1.suffix().str();
      outputHelp << "Literal: " << literal << endl;      

      int num = stoi(literal);
      locationCounter += num;
      locationCounterGlobal += num;
      for(int i = 0; i < num; i++){
         currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
      }

      if(s != "") return -1;

    }

    // end directive
    if(regex_search(s, m, endRegex)){
      outputHelp << "Found end directive: " << m.str(0) << endl;
      break;
    }

    // no operand isntruction
    if(regex_search(s, m, noOperandsInstructions)){
      outputHelp << "Found instruction with no operands: " << m.str(0) << endl;

      if(currentSectionId == -1){
        return -2;
      }
      if(m.prefix().str() != "" || m.suffix().str() != ""){
        return -1;
      }

      string instruction = m.str(0);
      s = m.suffix().str();    

      if(instruction == "halt"){
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
      } else {
        if(instruction == "iret"){
          currentSectionMachineCode = addToCode("20", currentSection.name, currentSectionMachineCode);
        } else {
          if(instruction == "ret"){
            currentSectionMachineCode = addToCode("40", currentSection.name, currentSectionMachineCode);
          }
        }
      }
      
      locationCounter++;
      locationCounterGlobal++;
    }

    // one register instruction
    if(regex_search(s, m, oneRegisterInsturctions)){
      outputHelp << "Found instruction with one register as operand: " << m.str(0) << endl;
      string helper = s;
      s = m.suffix().str();
      if(currentSectionId == -1){
        return -2;
      }
      if(m.prefix().str() != "" || m.suffix().str() != ""){
        return -1;
      }

      smatch m1;
      regex_search(helper, m1, symbolOnlyRegex);
      string instruction = m1.str(0);

      outputHelp << "Insturction: " << instruction << endl;

      regex_search(helper, m1, registersRegex);
      outputHelp << "Register found: " << m1.str(0) << endl;
      string reg = m1.str(0);
      
      if(instruction == "push" || instruction == "pop"){

        if(instruction == "push"){
          currentSectionMachineCode = addToCode("B0", currentSection.name, currentSectionMachineCode);

          if(reg == "sp"){
            currentSectionMachineCode = addToCode("66", currentSection.name, currentSectionMachineCode);
          } else {
            if(reg == "psw"){
              currentSectionMachineCode = addToCode("86", currentSection.name, currentSectionMachineCode);
            } else {
              regex_search(reg, m1, literalRegex);
              string num = m1.str(0);
              currentSectionMachineCode = addToCode(num +"6", currentSection.name, currentSectionMachineCode);              
            }
          }

           currentSectionMachineCode = addToCode("12", currentSection.name, currentSectionMachineCode);

        } else {  // instruction is pop
           currentSectionMachineCode = addToCode("A0", currentSection.name, currentSectionMachineCode);

          if(reg == "sp"){
             currentSectionMachineCode = addToCode("66", currentSection.name, currentSectionMachineCode);
          } else {
            if(reg == "psw"){
               currentSectionMachineCode = addToCode("86", currentSection.name, currentSectionMachineCode);
            } else {
              regex_search(reg, m1, literalRegex);
              string num = m1.str(0);
              currentSectionMachineCode = addToCode(num +"6", currentSection.name, currentSectionMachineCode);              
            }
          }
          currentSectionMachineCode = addToCode("42", currentSection.name, currentSectionMachineCode);
        }

        locationCounter+=3;
        locationCounterGlobal+=3;
      } else {
        if (instruction == "int" || instruction == "not"){

          if(instruction == "int"){
            currentSectionMachineCode = addToCode("10", currentSection.name, currentSectionMachineCode);
            if(reg == "sp"){
              currentSectionMachineCode = addToCode("6F", currentSection.name, currentSectionMachineCode);
            } else {
              if(reg == "psw"){
                currentSectionMachineCode = addToCode("8F", currentSection.name, currentSectionMachineCode);
              } else {
                regex_search(reg, m1, literalRegex);
                string num = m1.str(0);                
                currentSectionMachineCode = addToCode(num + "F", currentSection.name, currentSectionMachineCode);              
              }
            }
          } else {  // instruction is not
            currentSectionMachineCode = addToCode("80", currentSection.name, currentSectionMachineCode);

            if(reg == "sp"){
              currentSectionMachineCode = addToCode("66", currentSection.name, currentSectionMachineCode);
            } else {
              if(reg == "psw"){
                currentSectionMachineCode = addToCode("88", currentSection.name, currentSectionMachineCode);
              } else {
                regex_search(reg, m1, literalRegex);
                string num = m1.str(0) + m1.str(0);
                currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);              
              }
            }
          }

          locationCounter+=2;
          locationCounterGlobal+=2;          
        }
      }
      
    }

    if(regex_search(s, m, twoRegistersInstructions)){
      outputHelp << "Found instruction with two registers as operands: " << m.str(0) << endl;
      if(currentSectionId == -1){
        return -2;
      }
      if(m.prefix().str() != "" || m.suffix().str() != ""){
        return -1;
      }

      smatch m1;
      string s1 = m.str(0);
      string helper = s;
      s = m.suffix().str();
      regex_search(s1, m1, symbolOnlyRegex);                // remove instruction name
      string instruction = m1.str(0);
      s1 = m1.suffix().str();

      regex_search(instruction, m1, symbolOnlyRegex);                // remove instruction name
      instruction = m1.str(0);

      outputHelp << "Instruction: " << instruction << endl;

      regex_search(helper, m1, registersRegex);
      string r1 = m1.str(0);
      outputHelp << "Register found: " << m1.str(0) << endl;
      helper = m1.suffix().str();

      regex_search(helper, m1, registersRegex);
      string r2 = m1.str(0);
      outputHelp << "Register found: " << m1.str(0) << endl;

      string num;
      if(r1 == "sp") num = "6";
      else if(r1 == "psw") num = "8";
        else {
          if(regex_search(r1, m1, literalRegex))
            num = m1.str(0);
        }
      
      if(r2 == "sp") num += "6";
      else if(r2 == "psw") num += "8";
        else {
          if(regex_search(r2, m1, literalRegex))
            num += m1.str(0);
        }

      if(instruction == "xchg"){
        currentSectionMachineCode = addToCode("60", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);

      }
      else {  
        if(instruction == "add"){
          currentSectionMachineCode = addToCode("70", currentSection.name, currentSectionMachineCode);
          currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);

        }
        else {
          if(instruction == "sub"){
            currentSectionMachineCode = addToCode("71", currentSection.name, currentSectionMachineCode);
            currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);

          }
          else {
            if(instruction == "mul"){
              currentSectionMachineCode = addToCode("72", currentSection.name, currentSectionMachineCode);  
              currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);

            } 
            else {
              if(instruction == "div"){
                currentSectionMachineCode = addToCode("73", currentSection.name, currentSectionMachineCode);              
                currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);

              } 
              else {
                if(instruction == "cmp"){
                  currentSectionMachineCode = addToCode("74", currentSection.name, currentSectionMachineCode);
                  currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);

                }
                else {
                  if(instruction == "and"){
                    currentSectionMachineCode = addToCode("81", currentSection.name, currentSectionMachineCode);
                    currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);

                  }
                  else {
                    if(instruction == "or"){
                      currentSectionMachineCode = addToCode("82", currentSection.name, currentSectionMachineCode);
                      currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);

                    }
                    else {
                      if(instruction == "xor"){
                        currentSectionMachineCode = addToCode("83", currentSection.name, currentSectionMachineCode);                      
                        currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);

                      }
                      else {
                        if(instruction == "test"){
                          currentSectionMachineCode = addToCode("84", currentSection.name, currentSectionMachineCode);                       
                          currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);

                        }
                        else {
                          if(instruction == "shl"){
                            currentSectionMachineCode = addToCode("90", currentSection.name, currentSectionMachineCode);
                            currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);

                          }
                          else {
                            if(instruction == "shr"){
                              currentSectionMachineCode = addToCode("91", currentSection.name, currentSectionMachineCode);
                              currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);

                            } else { return -1; }
                          }
                        }
                      }
                    } 
                  }
                }
              }
            }
          }
        }
      }

      locationCounter+=2;
      locationCounterGlobal+=2;

    }

    if(regex_search(s, m, oneOperandInstructions)){
      outputHelp << "Found instruction with one operand: " << m.str(0) << endl;

      if(m.prefix().str() != "" || m.suffix().str() != ""){
        return -1;
      }

      smatch m1;
      string s1 = m.str(0);
      regex_search(s1, m1, symbolRegex);                // remove instruction name
      string instruction = m1.str(0);
      s1 = m1.suffix().str();

      regex_search(instruction, m1, symbolOnlyRegex);                // remove instruction name
      instruction = m1.str(0);

      outputHelp << "Instruction: " << instruction << endl;
      outputHelp << "Operand: " << s1 << endl;

      if(instruction == "call"){
        currentSectionMachineCode = addToCode("30", currentSection.name, currentSectionMachineCode);
      } else {
        if(instruction == "jmp"){
          currentSectionMachineCode = addToCode("50", currentSection.name, currentSectionMachineCode);
        } else {
          if(instruction == "jeq"){
            currentSectionMachineCode = addToCode("51", currentSection.name, currentSectionMachineCode);
          } else {
            if(instruction == "jne"){
              currentSectionMachineCode = addToCode("52", currentSection.name, currentSectionMachineCode);
            } else {
              if(instruction == "jgt"){
                currentSectionMachineCode = addToCode("53", currentSection.name, currentSectionMachineCode);
              } else { return -1; }
            }
          }
        }
      }

      // register direct
      if(regex_search(s1, m1, registerDirectJumpRegex)){
        outputHelp << "Jump Register direct value found!" << endl;
        string helper = s1;
        s1 = m1.suffix().str();

        regex_search(helper, m1, registersRegex);
        string reg = m1.str(0);
        string num;
        outputHelp << "Register found: " << reg << endl; 

        if(reg == "sp"){
          num = "6";
        } else {
          if(reg == "psw"){
            num = "6";
          } else {
            if(regex_search(helper, m1, literalRegex)){
              num = m1.str(0);
            } else {
              return -1;
            }
          }
        }
        
        currentSectionMachineCode = addToCode("F" + num, currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("01", currentSection.name, currentSectionMachineCode);

        locationCounter+=3;
        locationCounterGlobal+=3;
        
        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

      // PC REL with symbol
      if(regex_search(s1, m1, pcRelSymbolJumpRegex)){
        outputHelp << "Jump PC REL with symbol found!" << endl;
        string helper = s1;
        s1 = m1.suffix().str();
        
        locationCounter+=5;
        locationCounterGlobal+=5;

        currentSectionMachineCode = addToCode("F7", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("05", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);

        if(regex_search(helper, m1, symbolOnlyRegex)){       // add to forward if needed
          string symName = m1.str(0);
          outputHelp << "Symbol found: " << symName << endl;
          int ret = searchSymbol(symName);
          int endSize = currentSectionMachineCode.size() - 1;
          int startSize = currentSectionMachineCode.size() - 2;
          int size = currentRelocationTable.size();
          currentRelocationTable = addSymbolOrForwardElement(ret, symName, currentSectionId, locationCounter, currentSection, 
          currentRelocationTable, true, startSize, endSize);

          if(size == currentRelocationTable.size() && ret != -1){
            int mov = locationCounter - symbolTable.at(ret).offset - 1;
            string mov1 = to_string(mov);
            vector<string> help = decToCode(mov1);
            bool done = false;
            for(string s: help){
              if(!done){
                currentSectionMachineCode.at(startSize).value = s;
                done = true;
              } else {
                currentSectionMachineCode.at(endSize).value = s;
              }
            }
          }
        }

        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

      // register indirect with literal
      if(regex_search(s1, m1, registerIndirectLiteralJumpRegex)){
        outputHelp << "Jump Register indirect with literal value found!" << endl;
        string helper = s1;
        s1 = m1.suffix().str();

        regex_search(helper, m1, registersRegex);
        string reg = m1.str(0);
        helper = m1.suffix().str();
        string num;

        outputHelp << "Register found: " << reg << endl;

        if(reg == "sp"){
          num = "6";
        } else {
          if(reg == "psw"){
            num = "6";
          } else {
            if(regex_search(s1, m1, literalRegex)){
              num = m1.str(0);
            } else {
              return -1;
            }
          }
        }

        regex_search(helper, m1, literalRegex);
        string lit = m1.str(0);

        outputHelp << "Literal found: " << lit << endl;

        currentSectionMachineCode = addToCode("F" + num, currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("03", currentSection.name, currentSectionMachineCode);
        if(regex_search(lit, m1, hexRegex)){
          string num2 = m1.str(0);
          regex_search(num2, m1, hexRemoveRegex);
          num2 = m1.suffix().str();
          vector<string> help = hexToCode(num2);

          for(string s: help){
            currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
          }

        } else {
          string num2 = lit;
          vector<string> help = decToCode(num2);

          for(string s: help){
            currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
          }
        }

        locationCounter+=5;
        locationCounterGlobal+=5;
        
        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

      // register indirect with symbol
      if(regex_search(s1, m1, registerIndirectSymbolJumpRegex)){
        outputHelp << "Jump Register indirect with symbol value found!" << endl;
        string helper = s1;
        s1 = m1.suffix().str();

        regex_search(helper, m1, registersRegex);
        string reg = m1.str(0);
        helper = m1.suffix().str();
        string num;

        outputHelp << "Register found: " << reg << endl;

        if(reg == "sp"){
          num = "6";
        } else {
          if(reg == "psw"){
            num = "6";
          } else {
            if(regex_search(reg, m1, literalRegex)){
              num = m1.str(0);
            } else {
              return -1;
            }
          }
        }

        locationCounter+=5;
        locationCounterGlobal+=5;

        currentSectionMachineCode = addToCode("F" + num, currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("03", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);

        if(regex_search(helper, m1, symbolOnlyRegex)){       // add to forward if needed
          string symName = m1.str(0);
          helper = m1.suffix().str();
          regex_search(symName, m1, symbolNoBracketsRegex);
          symName = m1.str(0);
          outputHelp << "Symbol found: " << symName << endl;
          int ret = searchSymbol(symName);
          int endSize = currentSectionMachineCode.size() - 1;
          int startSize = currentSectionMachineCode.size() - 2;
          int size = currentRelocationTable.size();
          currentRelocationTable = addSymbolOrForwardElement(ret, symName, currentSectionId, locationCounter, currentSection, 
          currentRelocationTable, false, startSize, endSize);

          if(size == currentRelocationTable.size() && ret != -1){
            int mov = locationCounter - symbolTable.at(ret).offset - 1;
            string mov1 = to_string(mov);
            vector<string> help = decToCode(mov1);
            bool done = false;
            for(string s: help){
              if(!done){
                currentSectionMachineCode.at(startSize).value = s;
                done = true;
              } else {
                currentSectionMachineCode.at(endSize).value = s;
              }
            }
          }
        }

        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

      // register indirect
      if(regex_search(s1, m1, registerIndirectJumpRegex)){
        outputHelp << "Jump Register indirect value found!" << endl;
        string helper = s1;
        s1 = m1.suffix().str();
        if(regex_search(s1, m1, endBracketRegex)){
          s1 = m1.suffix().str();
        }

        regex_search(helper, m1, registersRegex);
        string reg = m1.str(0);
        string num;

        outputHelp << "Register found:" << reg << endl;

        if(reg == "sp"){
          num = "6";
        } else {
          if(reg == "psw"){
            num = "6";
          } else {
            if(regex_search(helper, m1, literalRegex)){
              num = m1.str(0);
            } else {
              return -1;
            }
          }
        }
        
        currentSectionMachineCode = addToCode("F" + num, currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("02", currentSection.name, currentSectionMachineCode);

        locationCounter+=3;
        locationCounterGlobal+=3;
        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

      // memory value literal
      if(regex_search(s1, m1, valueMemLiteralJumpRegex)){
        outputHelp << "Jump Memory literal value found!" << endl;
        string helper = s1;
        s1 = m1.suffix().str();

        locationCounter+=5;
        locationCounterGlobal+=5;

        regex_search(helper, m1, literalRegex);
        string lit = m1.str(0);

        currentSectionMachineCode = addToCode("F0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("04", currentSection.name, currentSectionMachineCode);
        if(regex_search(lit, m1, hexRegex)){
          string num2 = m1.str(0);
          regex_search(num2, m1, hexRemoveRegex);
          num2 = m1.suffix().str();
          vector<string> help = hexToCode(num2);

          for(string s: help){
            currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
          }

        } else {
          string num2 = lit;
          vector<string> help = decToCode(num2);

          for(string s: help){
            currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
          }
        }

        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

      // memory value symbol
      if(regex_search(s1, m1, valueMemSymbolJumpRegex)){
        outputHelp << "Jump Memory symbol value found!" << endl;
        string helper = s1;
        s1 = m1.suffix().str();

        regex_search(helper, m1, literalRegex);
        string lit = m1.str(0);
        outputHelp << "Literal found: " << lit << endl;

        currentSectionMachineCode = addToCode("F0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("04", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

      // literal value
      if(regex_search(s1, m1, literalRegex)){
        outputHelp << "Jump literal value found!" << endl;
        
        string lit = m1.str(0);
        string helper = s1;
        s1 = m1.suffix().str();
        outputHelp << "Literal found: " << lit << endl;

        currentSectionMachineCode = addToCode("F0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        if(regex_search(lit, m1, hexRegex)){
          string num2 = m1.str(0);
          regex_search(num2, m1, hexRemoveRegex);
          num2 = m1.suffix().str();
          vector<string> help = hexToCode(num2);

          for(string s: help){
            currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
          }

        } else {
          string num2 = lit;
          vector<string> help = decToCode(num2);

          for(string s: help){
            currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
          }
        }

        locationCounter+=5;
        locationCounterGlobal+=5;
        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

      // symbol value
      if(regex_search(s1, m1, symbolOnlyRegex)){
        outputHelp << "Jump symbol value found!" << endl;

        string symName = m1.str(0);
        string helper = s1;
        s1 = m1.suffix().str();
        int ret = searchSymbol(symName);
        outputHelp << "Symbol found: " << symName << endl;

        locationCounter+=5;
        locationCounterGlobal+=5;
        currentSectionMachineCode = addToCode("F0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);

        int endSize = currentSectionMachineCode.size() - 1;
        int startSize = currentSectionMachineCode.size() - 2;
        int size = currentRelocationTable.size();
        currentRelocationTable = addSymbolOrForwardElement(ret, symName, currentSectionId, locationCounter, currentSection, 
        currentRelocationTable, false, startSize, endSize);

        if(size == currentRelocationTable.size() && ret != -1){
          int mov = locationCounter - symbolTable.at(ret).offset - 1;
          string mov1 = to_string(mov);
          vector<string> help = decToCode(mov1);
          bool done = false;
          for(string s: help){
            if(!done){
              currentSectionMachineCode.at(startSize).value = s;
              done = true;
            } else {
              currentSectionMachineCode.at(endSize).value = s;
            }
          }
        }

        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

    }

    if(regex_search(s, m, oneOperandOneRegisterInstructions)){
      outputHelp << "Found instruction with one operand and one register: " << m.str(0) << endl;

      if(m.prefix().str() != "" || m.suffix().str() != ""){
        return -1;
      }

      smatch m1;
      string s1 = m.str(0);

      regex_search(s1, m1, symbolOnlyRegex);            
      string instruction = m1.str(0);

      regex_search(s1, m1, symbolRegex);                // remove instruction name
      instruction = m1.str(0);
      s1 = m1.suffix().str();

      regex_search(instruction, m1, symbolOnlyRegex);                // remove instruction name
      instruction = m1.str(0);

      outputHelp << "Instruction: " << instruction << endl;
      outputHelp << "Operands: " << s1 << endl;

      regex_search(s1, m1, registersRegex);
      string reg = m1.str(0);
      s1 = m1.suffix().str();

      outputHelp << "Register found: " << reg << endl;

      if(regex_search(s1, m1, commaRegex)){
        s1 = m1.suffix().str();
      }

      if(instruction == "ldr"){
        currentSectionMachineCode = addToCode("A0", currentSection.name, currentSectionMachineCode);
      } else {
        if(instruction == "str"){
        currentSectionMachineCode = addToCode("B0", currentSection.name, currentSectionMachineCode);
        } else { return -1; }
      }

      string num;
      regex_search(reg, m1, literalRegex);

      if(reg == "sp") num = "6";
        else if(reg == "psw") num = "8";
          else {
            if(regex_search(reg, m1, literalRegex))
              num = m1.str(0);
          }

      // PC REL with symbol
      if(regex_search(s1, m1, pcRelSymbolDataRegex)){
        outputHelp << "PC REL with symbol found!" << endl;
        string helper = s1;
        s1 = m1.suffix().str();

        locationCounter+=5;
        locationCounterGlobal+=5;

        currentSectionMachineCode = addToCode(num + "0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("05", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);

        if(regex_search(helper, m1, symbolOnlyRegex)){       // add to forward if needed
          string symName = m1.str(0);
          int ret = searchSymbol(symName);
          outputHelp << "Symbol found: " << symName << endl;
          int endSize = currentSectionMachineCode.size() - 1;
          int startSize = currentSectionMachineCode.size() - 2;
          int size = currentRelocationTable.size();
          currentRelocationTable = addSymbolOrForwardElement(ret, symName, currentSectionId, locationCounter, currentSection, 
          currentRelocationTable, true, startSize, endSize);

          if(size == currentRelocationTable.size() && ret != -1){
            int mov = locationCounter - symbolTable.at(ret).offset - 1;
            string mov1 = to_string(mov);
            vector<string> help = decToCode(mov1);
            bool done = false;
            for(string s: help){
              if(!done){
                currentSectionMachineCode.at(startSize).value = s;
                done = true;
              } else {
                currentSectionMachineCode.at(endSize).value = s;
              }
            }
          }
        }

        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

      // memory value literal
      if(regex_search(s1, m1, valueLiteralDataRegex)){
        outputHelp << "Literal value found!" << endl;
        string helper = s1;
        s1 = m1.suffix().str();

        regex_search(helper, m1, literalRegex);
        string lit = m1.str(0);
        outputHelp << "Literal found: " << lit << endl;
        s1 = m1.suffix().str();

        currentSectionMachineCode = addToCode(num + "0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        if(regex_search(lit, m1, hexRegex)){
          string num2 = m1.str(0);
          regex_search(num2, m1, hexRemoveRegex);
          num2 = m1.suffix().str();
          vector<string> help = hexToCode(num2);

          for(string s: help){
            currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
          }

        } else {
          string num2 = lit;
          vector<string> help = decToCode(num2);

          for(string s: help){
            currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
          }
        }

        locationCounter+=5;
        locationCounterGlobal+=5;

        if(s1 != ""){
          return -1;
        } else {
          continue;
        }

      }

      // memory value symbol
      if(regex_search(s1, m1, valueSymbolDataRegex)){
        outputHelp << "Symbol value found!" << endl;
        string helper = s1;
        s1 = m1.suffix().str();

        locationCounter+=5;
        locationCounterGlobal+=5;
        currentSectionMachineCode = addToCode(num + "0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);

        if(regex_search(helper, m1, symbolOnlyRegex)){       // add to forward if needed
          string symName = m1.str(0);
          int ret = searchSymbol(symName);
          outputHelp << "Symbol found: " << symName << endl;
          int endSize = currentSectionMachineCode.size() - 1;
          int startSize = currentSectionMachineCode.size() - 2;
          int size = currentRelocationTable.size();
          currentRelocationTable = addSymbolOrForwardElement(ret, symName, currentSectionId, locationCounter, currentSection, 
          currentRelocationTable, false, startSize, endSize);

          if(size == currentRelocationTable.size() && ret != -1){
            int mov = locationCounter - symbolTable.at(ret).offset - 1;
            string mov1 = to_string(mov);
            vector<string> help = decToCode(mov1);
            bool done = false;
            for(string s: help){
              if(!done){
                currentSectionMachineCode.at(startSize).value = s;
                done = true;
              } else {
                currentSectionMachineCode.at(endSize).value = s;
              }
            }
          }
        }

        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

      // register indirect
      if(regex_search(s1, m1, registerIndirectDataRegex)){
        if(m1.suffix().str() == "]"){
          outputHelp << "Register indirect value found!" << endl;
          string helper = s1;
          s1 = m1.suffix().str();
          regex_search(s1, m1, endBracketRegex);
          s1 = m1.suffix().str();

          regex_search(helper, m1, registersRegex);
          string reg2 = m1.str(0);
          outputHelp << "Register found: " << reg2 << endl;

          if(reg2 == "sp") num += "6";
          else if(reg2 == "psw") num += "8";
            else {
              if(regex_search(reg2, m1, literalRegex))
                num += m1.str(0);
            }

          currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);
          currentSectionMachineCode = addToCode("02", currentSection.name, currentSectionMachineCode);

          locationCounter+=3;
          locationCounterGlobal+=3;
          if(s1 != ""){
            return -1;
          } else {
            continue;
          }
        }

      }

      // register indirect with symbol
      if(regex_search(s1, m1, registerIndirectSymbolDataRegex)){
        string help1 = m1.suffix().str();
        regex_search(help1, m1, helperSymbolRegex);
        if(m1.suffix().str() == ""){
          outputHelp << "Register indirect with symbol value found!" << endl;
          string helper = s1;
          s1 = m1.suffix().str();

          regex_search(s1, m1, endBracketRegex);
          s1 = m1.suffix().str();

          locationCounter+=5;
          locationCounterGlobal+=5;

          regex_search(helper, m1, registersRegex);
          string reg2 = m1.str(0);
          outputHelp << "Register found: " << reg2 << endl;
          helper = m1.suffix().str();

          if(reg2 == "sp") num += "6";
          else if(reg2 == "psw") num += "8";
            else {
              if(regex_search(reg2, m1, literalRegex))
                num += m1.str(0);
            }

          currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);
          currentSectionMachineCode = addToCode("03", currentSection.name, currentSectionMachineCode);
          currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
          currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);

          if(regex_search(helper, m1, symbolOnlyRegex)){       // add to forward if needed
            string symName = m1.str(0);
            regex_search(symName, m1, endBracketRegex);
            symName = m1.prefix().str();
            outputHelp << "Symbol found: " << symName << endl;
            int ret = searchSymbol(symName);
            int endSize = currentSectionMachineCode.size() - 1;
            int startSize = currentSectionMachineCode.size() - 2;
            int size = currentRelocationTable.size();
            currentRelocationTable = addSymbolOrForwardElement(ret, symName, currentSectionId, locationCounter, currentSection, 
            currentRelocationTable, false, startSize, endSize);

            if(size == currentRelocationTable.size() && ret != -1){
              int mov = locationCounter - symbolTable.at(ret).offset - 1;
              string mov1 = to_string(mov);
              vector<string> help = decToCode(mov1);
              bool done = false;
              for(string s: help){
                if(!done){
                  currentSectionMachineCode.at(startSize).value = s;
                  done = true;
                } else {
                  currentSectionMachineCode.at(endSize).value = s;
                }
              }
            }
          }

          if(s1 != ""){
            return -1;
          } else {
            continue;
          }
        }
      }

      // register indirect with literal
      if(regex_search(s1, m1, registerIndirectLiteralDataRegex)){
        outputHelp << "Register indirect with literal value found!" << endl;
        string helper = s1;
        s1 = m1.suffix().str();

        regex_search(s1, m1, endBracketRegex);
        s1 = m1.suffix().str();

        regex_search(helper, m1, registersRegex);
        string reg2 = m1.str(0);
        helper = m1.suffix().str();
        regex_search(helper, m1, literalRegex);
        string lit = m1.str(0);
        helper = m1.suffix().str();

        outputHelp << "Register found: " << reg2 << endl;
        outputHelp << "Literal found: " << lit << endl;

        if(reg2 == "sp") num += "6";
        else if(reg2 == "psw") num += "8";
          else {
            if(regex_search(reg2, m1, literalRegex))
              num += m1.str(0);
          }

        currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("03", currentSection.name, currentSectionMachineCode);
        if(regex_search(lit, m1, hexRegex)){
          string num2 = m1.str(0);
          regex_search(num2, m1, hexRemoveRegex);
          num2 = m1.suffix().str();
          vector<string> help = hexToCode(num2);

          for(string s: help){
            currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
          }

        } else {
          string num2 = lit;
          vector<string> help = decToCode(num2);

          for(string s: help){
            currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
          }
        }

        locationCounter+=5;
        locationCounterGlobal+=5;
        if(s1 != ""){
          return -1;
        } else {
          continue;
        }        
      }

      // register direct
      if(regex_search(s1, m1, registersRegex)){
        outputHelp << "Register direct value found!" << endl;
        string reg2 = m1.str(0);
        s1 = m1.suffix().str();
        if(regex_search(s1, m1, endBracketRegex)){
          s1 = m1.suffix().str();
        }

        outputHelp << "Register found: " << reg2 << endl;

        if(reg2 == "sp") num += "6";
        else if(reg2 == "psw") num += "8";
          else {
            if(regex_search(reg2, m1, literalRegex))
              num += m1.str(0);
          }

        currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("01", currentSection.name, currentSectionMachineCode);

        locationCounter+=3;
        locationCounterGlobal+=3;
        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

      // literal value
      if(regex_search(s1, m1, literalRegex)){
        outputHelp << "Memory literal value found!" << endl;

        string lit = m1.str(0);
        s1 = m1.suffix().str();

        outputHelp << "Literal found: " << lit << endl;

        currentSectionMachineCode = addToCode(num + "0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("04", currentSection.name, currentSectionMachineCode);
        if(regex_search(lit, m1, hexRegex)){
          string num2 = m1.str(0);
          regex_search(num2, m1, hexRemoveRegex);
          num2 = m1.suffix().str();
          vector<string> help = hexToCode(num2);

          for(string s: help){
            currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
          }

        } else {
          string num2 = lit;
          vector<string> help = decToCode(num2);

          for(string s: help){
            currentSectionMachineCode = addToCode(s, currentSection.name, currentSectionMachineCode);
          }
        }

        locationCounter+=5;
        locationCounterGlobal+=5;
        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }

      // symbol value
      if(regex_search(s1, m1, symbolRegex)){
        string helper = s1;
        s1 = m1.suffix().str();
        if(!regex_search(helper, m1, registersRegex)){

          locationCounter+=5;
          locationCounterGlobal+=5;

          currentSectionMachineCode = addToCode(num + "0", currentSection.name, currentSectionMachineCode);
          currentSectionMachineCode = addToCode("04", currentSection.name, currentSectionMachineCode);
          currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
          currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);

          if(regex_search(helper, m1, symbolOnlyRegex)){       // add to forward if needed
            string symName = m1.str(0);
            int ret = searchSymbol(symName);
            outputHelp << "Symbol found: " << symName << endl;
            int endSize = currentSectionMachineCode.size() - 1;
            int startSize = currentSectionMachineCode.size() - 2;
            int size = currentRelocationTable.size();
            currentRelocationTable = addSymbolOrForwardElement(ret, symName, currentSectionId, locationCounter, currentSection, 
            currentRelocationTable, false, startSize, endSize);

            if(size == currentRelocationTable.size() && ret != -1){
              int mov = locationCounter - symbolTable.at(ret).offset - 1;
              string mov1 = to_string(mov);
              vector<string> help = decToCode(mov1);
              bool done = false;
              for(string s: help){
                if(!done){
                  currentSectionMachineCode.at(startSize).value = s;
                  done = true;
                } else {
                  currentSectionMachineCode.at(endSize).value = s;
                }
              }
            }
          }

          outputHelp << "Memory symbol value found!" << endl;
        }
        if(s1 != ""){
          return -1;
        } else {
          continue;
        }
      }
      
    }

    if(s != "") {
      outputHelp << s << endl;
      return -1;
    }

  }

  currentSection.length = locationCounter;
  sectionTable.push_back(currentSection);
  machineCode.push_back(currentSectionMachineCode);
  relocationTable.push_back(currentRelocationTable);

  printOutput();

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

    int ret = assembler.pass();
    if(ret == -1){
      throw BadSyntaxException();
    }
    if(ret == -2){
      throw NoSectionException();
    }
    if(ret == -2){
      throw NonexistantInputFileException();
    }

    
  }
  catch(const std::exception& e){
    std::cerr << e.what() << '\n';
  }

  return 0;
}