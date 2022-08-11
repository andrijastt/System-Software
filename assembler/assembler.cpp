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
 * @brief adds machine code to section
 * 
 * @param value         what value we add
 * @param sectionName   to what section it is added
 * @param machineCodes  to what code
 */

// vector<MachineCode> Assembler::addToCode(string value, string sectionName, vector<MachineCode> machineCodes){
 
// }

/**
 * @brief  prints assembler output
 * 
 */
void Assembler::printOutput(){
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
  this->outputFile << "Num\tValue\tSize\tType\tBind\tNdx\tName\tDefined\n";
  for(Symbol sym: symbolTable){
    this->outputFile << sym.id << "\t" << std::setfill('0') << std::setw(4) << std::hex << sym.value << "\t" << std::dec << sym.size << "\t"; 

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

    this->outputFile << sym.name << "\t";

    switch(sym.defined){

      case true:
        this->outputFile << "DEF\n";
        break;

      case false:
        this->outputFile << "UND\n";
        break;
    }
  }

  this->outputFile << endl;
  this->outputFile << endl;
  this->outputFile << endl;
  this->outputFile << endl;

  for(Section sec: sectionTable){
    this->outputFile << "Machine code <" << sec.name << ">\n";

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
        i++;
      }

    }
    this->outputFile << endl;
  }

}

/**
 * @brief One and only pass of assembler
 * 
 * @return int 0 - it is good, -1 - syntax error, -2 - some labels or code are not in section
 */
int Assembler::pass(){

  // size_t lastindex = outputFileString.find_last_of(".");
  // string outputFileHelpString = outputFileString.substr(0, lastindex) + "Helper.o";

  int locationCounter = 0;
  int locationCounterGlobal = 0;
  int globalBase = 0;
  Section currentSection;
  currentSection.name = "";

  int currentSectionId = -1;
  vector<MachineCode> currentSectionMachineCode;

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
        this->machineCode.push_back(currentSectionMachineCode);
        currentSectionMachineCode.clear();
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
        sym.bind = LOCAL;
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
        string val = m1.str(0);
        s1 = m1.suffix().str();
         currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
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
      for(int i = 0; i < num; i++){
         currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
      }

    }

    // end directive
    if(regex_search(s, m, endRegex)){
      this->outputFile << "Found end directive: " << m.str(0) << endl;
      break;
    }

    // no operand isntruction
    if(regex_search(s, m, noOperandsInstructions)){
      this->outputFile << "Found instruction with no operands: " << m.str(0) << endl;
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
      this->outputFile << "Found instruction with one register as operand: " << m.str(0) << endl;
      
      smatch m1;
      regex_search(s, m1, symbolOnlyRegex);
      string instruction = m1.str(0);

      this->outputFile << "Insturction: " << instruction << endl;

      regex_search(s, m1, registersRegex);
      this->outputFile << "Register found: " << m1.str(0) << endl;
      string reg = m1.str(0);
      regex_search(reg, m1, literalRegex);
      string num = m1.str(0);

      if(instruction == "push" || instruction == "pop"){

        if(instruction == "push"){
           currentSectionMachineCode = addToCode("B0", currentSection.name, currentSectionMachineCode);

          if(reg == "sp"){
             currentSectionMachineCode = addToCode("66", currentSection.name, currentSectionMachineCode);
          } else {
            if(reg == "psw"){
               currentSectionMachineCode = addToCode("86", currentSection.name, currentSectionMachineCode);
            } else {
               currentSectionMachineCode = addToCode(num +"6", currentSection.name, currentSectionMachineCode);              
            }
          }

           currentSectionMachineCode = addToCode("11", currentSection.name, currentSectionMachineCode);

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
          currentSectionMachineCode = addToCode("21", currentSection.name, currentSectionMachineCode);
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
        this->outputFile << "Jump Register direct value found!" << endl;

        regex_search(s1, m1, registersRegex);
        string reg = m1.str(0);
        string num;

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
        
        currentSectionMachineCode = addToCode("F" + num, currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("01", currentSection.name, currentSectionMachineCode);

        locationCounter+=3;
        locationCounterGlobal+=3;
        continue;
      }

      // PC REL with symbol
      if(regex_search(s1, m1, pcRelSymbolJumpRegex)){
        this->outputFile << "Jump PC REL with symbol found!" << endl;

        regex_search(s1, m1, symbolOnlyRegex);
        string symb = m1.str(0);

        currentSectionMachineCode = addToCode("F7", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("03", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // register indirect with literal
      if(regex_search(s1, m1, registerIndirectLiteralJumpRegex)){
        this->outputFile << "Jump Register indirect with literal value found!" << endl;

        regex_search(s1, m1, registersRegex);
        string reg = m1.str(0);
        s1 = m1.suffix().str();
        string num;

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

        regex_search(s1, m1, literalRegex);
        string lit = m1.str(0);

        currentSectionMachineCode = addToCode("F" + num, currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("03", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("01", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("01", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // register indirect with symbol
      if(regex_search(s1, m1, registerIndirectSymbolJumpRegex)){
        this->outputFile << "Jump Register indirect with symbol value found!" << endl;

        regex_search(s1, m1, registersRegex);
        string reg = m1.str(0);
        s1 = m1.suffix().str();
        string num;

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

        regex_search(s1, m1, symbolOnlyRegex);
        string symb = m1.str(0);

        currentSectionMachineCode = addToCode("F" + num, currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("03", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("02", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("02", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // register indirect
      if(regex_search(s1, m1, registerIndirectJumpRegex)){
        this->outputFile << "Jump Register indirect value found!" << endl;

        regex_search(s1, m1, registersRegex);
        string reg = m1.str(0);
        string num;

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
        
        currentSectionMachineCode = addToCode("F" + num, currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("02", currentSection.name, currentSectionMachineCode);

        locationCounter+=3;
        locationCounterGlobal+=3;
        continue;
      }

      // memory value literal
      if(regex_search(s1, m1, valueMemLiteralJumpRegex)){
        this->outputFile << "Jump Memory literal value found!" << endl;

        regex_search(s1, m1, symbolOnlyRegex);
        string symb = m1.str(0);

        currentSectionMachineCode = addToCode("F0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("04", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("03", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("03", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // memory value symbol
      if(regex_search(s1, m1, valueMemSymbolJumpRegex)){
        this->outputFile << "Jump Memory symbol value found!" << endl;

        regex_search(s1, m1, literalRegex);
        string lit = m1.str(0);

        currentSectionMachineCode = addToCode("F0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("04", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("04", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("04", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // literal value
      if(regex_search(s1, m1, literalRegex)){
        this->outputFile << "Jump literal value found!" << endl;

        string lit = m1.str(0);

        currentSectionMachineCode = addToCode("F0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("05", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("05", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // symbol value
      if(regex_search(s1, m1, symbolOnlyRegex)){
        this->outputFile << "Jump symbol value found!" << endl;

        string symb = m1.str(0);

        currentSectionMachineCode = addToCode("F0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("06", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("06", currentSection.name, currentSectionMachineCode);

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
      string reg = m1.str(0);
      s1 = m1.suffix().str();

      this->outputFile << "Register found: " << reg << endl;

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
        this->outputFile << "PC REL with symbol found!" << endl;

        regex_search(s1, m1, symbolOnlyRegex);
        string symb = m1.str(0);

        currentSectionMachineCode = addToCode(num + "0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("03", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("07", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("07", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // memory value literal
      if(regex_search(s1, m1, valueLiteralDataRegex)){
        this->outputFile << "Literal value found!" << endl;

        regex_search(s1, m1, literalRegex);
        string lit = m1.str(0);

        currentSectionMachineCode = addToCode(num + "0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("08", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("08", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // memory value symbol
      if(regex_search(s1, m1, valueSymbolDataRegex)){
        this->outputFile << "Symbol value found!" << endl;

        regex_search(s1, m1, symbolOnlyRegex);
        string symb = m1.str(0);

        currentSectionMachineCode = addToCode(num + "0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("00", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("09", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("09", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // register direct
      if(regex_search(s1, m1, registersRegex)){
        this->outputFile << "Register direct value found!" << endl;
        string reg2 = m1.str(0);
        s1 = m1.suffix().str();

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
        continue;
      }

      // register indirect
      if(regex_search(s1, m1, registerIndirectDataRegex)){
        this->outputFile << "Register indirect value found!" << endl;

        regex_search(s1, m1, registersRegex);
        string reg2 = m1.str(0);

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
        continue;
      }

      // register indirect with literal
      if(regex_search(s1, m1, registerIndirectLiteralDataRegex)){
        this->outputFile << "Register indirect with literal value found!" << endl;

        regex_search(s1, m1, registersRegex);
        string reg2 = m1.str(0);
        s1 = m1.suffix().str();
        regex_search(s1, m1, literalRegex);
        string lit = m1.str(0);
        s1 = m1.suffix().str();

        if(reg2 == "sp") num += "6";
        else if(reg2 == "psw") num += "8";
          else {
            if(regex_search(reg2, m1, literalRegex))
              num += m1.str(0);
          }

        currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("03", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("0A", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("0A", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // register indirect with symbol
      if(regex_search(s1, m1, registerIndirectSymbolDataRegex)){
        this->outputFile << "Register indirect with literal value found!" << endl;

         regex_search(s1, m1, registersRegex);
        string reg2 = m1.str(0);
        s1 = m1.suffix().str();
        regex_search(s1, m1, symbolOnlyRegex);
        string symb = m1.str(0);
        s1 = m1.suffix().str();

        if(reg2 == "sp") num += "6";
        else if(reg2 == "psw") num += "8";
          else {
            if(regex_search(reg2, m1, literalRegex))
              num += m1.str(0);
          }

        currentSectionMachineCode = addToCode(num, currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("03", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("0B", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("0B", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // literal value
      if(regex_search(s1, m1, literalRegex)){
        this->outputFile << "Memory literal value found!" << endl;

        string lit = m1.str(0);
        s1 = m1.suffix().str();

        currentSectionMachineCode = addToCode(num + "0", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("04", currentSection.name, currentSectionMachineCode);
        // TODO
        currentSectionMachineCode = addToCode("0C", currentSection.name, currentSectionMachineCode);
        currentSectionMachineCode = addToCode("0C", currentSection.name, currentSectionMachineCode);

        locationCounter+=5;
        locationCounterGlobal+=5;
        continue;
      }

      // symbol value
      if(regex_search(s1, m1, symbolRegex)){
        if(!regex_search(s1, m1, registersRegex)){

          regex_search(s1, m1, symbolOnlyRegex);
          string symb = m1.str(0);
          s1 = m1.suffix().str();

          currentSectionMachineCode = addToCode(num + "0", currentSection.name, currentSectionMachineCode);
          currentSectionMachineCode = addToCode("04", currentSection.name, currentSectionMachineCode);
          // TODO
          currentSectionMachineCode = addToCode("0D", currentSection.name, currentSectionMachineCode);
          currentSectionMachineCode = addToCode("0D", currentSection.name, currentSectionMachineCode);

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
  machineCode.push_back(currentSectionMachineCode);

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