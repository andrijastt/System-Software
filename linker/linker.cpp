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
 * @brief turns dec number to machine code
 * 
 * @param num the number that need to be changed
 * @return vector<string> helper machine code
 */
vector<string> Linker::decToCode(string num){
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
      ret.push_back(help1.substr(1,2));
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
 * @brief searches if there section already exists
 * 
 * @param sec section that we search for
 * @return int -1 section doen't exist, other num section already exists
 */
int Linker::searchSection(Section sec){
  if(Sections.size() == 0) return -1;

  int ret = 0;
  for(Section s:Sections){
    if(s.name == sec.name){
      return ret;
    }
    ret++;
  }

  return -1;
}

/**
 * @brief searches if there symbol already exists
 * 
 * @param symb symbol that we search for
 * @return int -1 symbol doen't exist, other num symbol already exists
 */
int Linker::searchSymbol(Symbol symb){
  if(Symbols.size() == 0) return -1;

  int ret = 0;
  for(Symbol s:Symbols){
    if(s.symbolName == symb.symbolName){
      return ret;
    }
    ret++;
  }

  return -1;
}

/**
 * @brief check if there are undefined symbols
 * 
 * @return true there is an undefined symbol
 * @return false all symbols are defined
 */
bool Linker::checkForUNDSymbols(){

  for(Symbol s: Symbols){
    if(!s.defined && s.type != SCTN)
      return true;
  }

  return false;

}

/**
 * @brief sets machine code in right order
 * 
 */
void Linker::setGoodCode(){

  int start = 0;
  for(Section s: Sections){
    for(MachineCode mc: allMachineCode){
      if(mc.sectionName == s.name){

        start += mc.code.size();
        goodMachineCode.push_back(mc);

      }
    }
  }
}

/**
 * @brief Set good offsets for Symbols
 * 
 */
void Linker::setSymbolOffset(){

  int i = 1;
  for(Symbol symb: Symbols){
    if(symb.symbolName == "UND") continue;

    int size = 0;
    for(MachineCode mc: goodMachineCode){
      if(mc.fileName == symb.fileName && mc.sectionName == Symbols[symb.sectionId].symbolName){
        Symbols[i].offset += size;
        size = 0;
        break;
      } else {
        size += mc.code.size();
      }
    }
    i++;
  }

  int sz = 0;
  for(Section sec: Sections){
    if(sec.name == "UND") continue;

    int j  = 0;
    for(Symbol symb:Symbols){
      if(symb.symbolName == sec.name){
        Symbols[j].offset = sz;
      }
      j++;
    }

    sz += sec.size;
  }  

}

/**
 * @brief changes Machine code at set locations
 * 
 */
void Linker::doRelocations(){

  for(Relocations relos: allRelocations){

    int i = 0, size = 0;
    for(MachineCode mc: goodMachineCode){
      if(mc.fileName == relos.fileName && mc.sectionName == relos.name){

        int sz = relos.relocations.size();
        for(int j = 0; j < sz; j++){
          
          if(relos.relocations[j].type == R_16){
            string symbolValue = to_string(Symbols[relos.relocations[j].symbolId].offset + relos.relocations[j].addend);
            vector<string> helper = decToCode(symbolValue);
            goodMachineCode[i].code[relos.relocations[j].offset] = helper[0];
            goodMachineCode[i].code[relos.relocations[j].offset + 1] = helper[1];
          } else {

            if(relos.relocations[j].type == R_WORD16){
              string symbolValue = to_string(Symbols[relos.relocations[j].symbolId].offset);
              vector<string> helper = decToCode(symbolValue);
              goodMachineCode[i].code[relos.relocations[j].offset + relos.relocations[j].addend] = helper[1];
              goodMachineCode[i].code[relos.relocations[j].offset + relos.relocations[j].addend + 1] = helper[0];
            } else {
              
              int offsetSymb0 = size + relos.relocations[j].offset + 2;
              int offsetSymb1 = Symbols[relos.relocations[j].symbolId].offset;
              string help = to_string(offsetSymb1 - offsetSymb0);
              vector<string> helper = decToCode(help);
              goodMachineCode[i].code[relos.relocations[j].offset] = helper[0];
              goodMachineCode[i].code[relos.relocations[j].offset + 1] = helper[1];
            }
                 
          }

        }

        break;
      }
      i++;
      size += mc.code.size();
    }
  }

}

/**
 * @brief prints in help file to see if everything is ok
 * 
 */
void Linker::printHelpFile(){

  ofstream linkerHelper;
  linkerHelper.open("linkerHelper.hex", ios::out|ios::trunc);

  for(Section s: Sections){
    linkerHelper << s.id << "\t" << s.size << "\t" << s.name << "\n";
  }
  linkerHelper << endl << endl;
  for(Symbol s: Symbols){
    linkerHelper << s.id << "\t" << hex << s.offset << dec << "\t" << s.sectionId << "\t" << s.symbolName << "\t" << s.defined << "\t";

    switch(s.bind){
      case GLOBAL:
        linkerHelper << "GLOBAL" << "\t\t";
        break;

      case LOCAL:
        linkerHelper << "LOCAL" << "\t\t";
        break;

      case NOBIND:
      linkerHelper << "NOBIND" << "\t\t";
      break;
    }

    linkerHelper << s.fileName << endl;
  }
  linkerHelper << endl << endl;

  for(Relocations rels: allRelocations){

    linkerHelper << "Relocations from section " << rels.name << "\t FILE NAME: " << rels.fileName << endl;

    for(Relocation rel: rels.relocations){
      linkerHelper << hex << rel.offset << dec << "\t";
      switch(rel.type){
        case R_16:
          linkerHelper << "R_16\t";
          break;

        case R_PC16:
          linkerHelper << "R_PC16\t";
          break;

        case R_WORD16:
          linkerHelper << "R_WORD16\t";
      }
      linkerHelper << rel.symbolId << "\t" << rel.addend << endl;
    }
    linkerHelper << endl;
  }

  for(MachineCode mc:allMachineCode){
    linkerHelper << "Machine code from section " << mc.sectionName << "\t FILE NAME: " << mc.fileName << endl;

    int i = 0;
    for(string s: mc.code){
      if(i % 8 == 0){
        linkerHelper << endl << hex << setfill('0') << setw(4) << i << dec << ": ";
      }
      linkerHelper << s << " ";
      i++;
    }
    linkerHelper << endl;
  }

  linkerHelper << endl << endl << "All machine code linked (GOOD CODE)\n";

  this->outputFile.open(outputFileString, ios::out|ios::trunc);

  int j = 0;
  for(MachineCode mc: goodMachineCode){
    for(string s: mc.code){

      if(j % 8 == 0){
        linkerHelper << endl << hex << setfill('0') << setw(4) << j << dec << ": ";
      }
      linkerHelper << s << " ";
      j++;
    }

  }

  j = 0;
  for(MachineCode mc: goodMachineCode){
    for(string s: mc.code){

      if(j == 0){
        this->outputFile << hex << setfill('0') << setw(4) << j << dec << ": ";
      } else {
        if(j % 8 == 0){
          this->outputFile << endl << hex << setfill('0') << setw(4) << j << dec << ": ";
        } 
      }
      
      this->outputFile<< s << " ";
      j++;
    }

  }

  linkerHelper.close();

}

/**
 * @brief Linker links all input files
 * 
 * @return int 0 - everything is okay, -1 - wrong terminal input, -2 - some input files don't exist
 * 
 */
int Linker::link(){

  Symbol oldSection;
  vector<Symbol> currentSymbols;
  int currentRelocations;
  int currentMC;
  bool turn = true;

  for(string s: this->inputFileStrings){
    inputFile.open(s, ios::in);

    if(!inputFile.is_open()) return -2;

    string line;
    int current = -1;

    while(getline(inputFile, line)){

      if(line == "" || line == "UND") continue;

      if(line == "SECTIONS"){
        current = 0;
        continue;
      }

      if(line == "SYMBOLS"){
        currentSymbols.clear();
        current = 1;
        continue;
      }

      if(line == "RELOCATIONS"){
        current = 2;
        continue;
      }

      if(line == "MACHINE CODE"){
        current = 3;
        continue;
      }

      if(line == "END"){
        break;
      }

      if(current == -1) return -2;
      
      if(current == 0){
        size_t pos = 0;
        string delimiter = "\t";
        
        Section sec;
        
        int i = 0;
        while((pos = line.find(delimiter)) != std::string::npos){
          string token = line.substr(0, pos);
          line.erase(0, pos + delimiter.length());

          switch(i){
            case 0:
              sec.id = stoi(token);
              break;
            case 1:
              sec.size = stoi(token);
              break;
          }
          i++;
        }
        sec.name = line;
        i = searchSection(sec);

        if(i == -1){  // new section, add to sections
          sec.id = Sections.size();
          Sections.push_back(sec);
        } else {      // section already exists, add to size only
          Sections[i].size += sec.size;
        }

      }

      if(current == 1){
        
        size_t pos = 0;
        string delimiter = "\t";
        
        Symbol symb;
        int i = 0;
        while((pos = line.find(delimiter)) != std::string::npos){
          string token = line.substr(0, pos);
          line.erase(0, pos + delimiter.length());

          switch(i){
            case 0:
              symb.id = stoi(token);
              break;

            case 1:
              sscanf(token.c_str(), "%X", &symb.offset);
              break;

            case 2:
              if(token == "SCTN") symb.type = SCTN;
              else if(token == "NOTYP") symb.type = NOTYP;
              break;

            case 3:
              if(token == "NOBIND") symb.bind = NOBIND;
              else if(token == "GLOB") symb.bind = GLOBAL;
              else if(token == "LOC") symb.bind = LOCAL;
              break;

            case 4:
              if(token == "UND") symb.sectionId = -1;
              else symb.sectionId = stoi(token);
              break;
            
            case 5:
              symb.symbolName = token;
              break;
          }

          i++;
        }

        if(line == "UND") symb.defined = false;
        else if(line == "DEF") {
          symb.defined = true;
          symb.fileName = s;
        }
        i = searchSymbol(symb);

        if(i == -1){
          int oldId = symb.id;
          symb.id = Symbols.size();
          if(symb.type == SCTN){ 
            oldSection = symb;
            oldSection.id = symb.id;
            symb.sectionId = symb.id;
          }
          Symbols.push_back(symb);

          if(symb.type == SCTN && symb.symbolName != "UND"){    
            int i = 0;        
            for(Symbol s: Symbols){
              if(s.sectionId == oldId && s.type != SCTN){
                Symbols[i].sectionId= symb.id;
              }
              i++;
            }
          }

        } else {
          if(Symbols[i].defined && symb.defined){
            return -3;
          }
          if(!Symbols[i].defined && symb.defined){
            Symbols[i].defined = true;
            Symbols[i].offset = symb.offset;
            Symbols[i].sectionId = symb.sectionId;
            Symbols[i].fileName = s;
            if(Symbols[i].sectionId != oldSection.id){
              for(Symbol ss: Symbols){
                if(ss.symbolName == oldSection.symbolName){
                  Symbols[i].sectionId = ss.id;
                }
              }
            }
              
          }
        }
        currentSymbols.push_back(symb);

      }

      if(current == 2){

        size_t pos = 0;
        string delimiter = "\t";
        vector<string> params;

        for(int j = 0; j < 4; j++){
          pos = line.find(delimiter);
          string token = line.substr(0, pos);
          line.erase(0, pos + delimiter.length());
          if(token != "") params.push_back(token);
        }

        if(params.size() == 4){
          if(params[0] == params[1]){
            Relocations relos;
            relos.name = params[0];
            relos.fileName = s;
            currentRelocations = allRelocations.size();
            allRelocations.push_back(relos);
          } else {
            Relocation relo;

            sscanf(params[0].c_str(), "%X", &relo.offset);
            if(params[1] == "R_16") relo.type = R_16;
            else if(params[1] == "R_PC16") relo.type = R_PC16;
            else if(params[1] == "R_WORD16") relo.type = R_WORD16;
            relo.symbolId = stoi(params[2]);

            int num;
            sscanf(params[3].substr(0,1).c_str(), "%X", &num);
            if(num >= 8){
              params[3] = "FFFF" + params[3];
            }

            sscanf(params[3].c_str(), "%X", &relo.addend);

            for(Symbol s: Symbols){
              if(s.symbolName == currentSymbols[relo.symbolId].symbolName){
                relo.symbolId = s.id;
                break;
              }
            }

            allRelocations[currentRelocations].relocations.push_back(relo);
          }
        } 

      }

      if(current == 3){
        size_t pos = 0;
        string delimiter = " ";
        vector<string> params;

        if((pos = line.find(delimiter)) == std::string::npos){
          params.push_back(line);
        }

        while((pos = line.find(delimiter)) != std::string::npos){
          string token = line.substr(0, pos);
          line.erase(0, pos + delimiter.length());
          params.push_back(token);
        }

        if(params.size() == 1 && turn){
          MachineCode mc;
          mc.sectionName = params[0];
          mc.fileName = s;
          currentMC = allMachineCode.size();
          allMachineCode.push_back(mc);
          turn = false;
        } else {
          allMachineCode[currentMC].code = params;
          turn = true;
        }

      }

    }

    inputFile.close();
  }

  bool ret = checkForUNDSymbols();
  if(ret) return -4;

  setGoodCode();
  setSymbolOffset();
  doRelocations();

  printHelpFile();

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

    if(ret == -2) throw InputException();
    if(ret == -3) throw MulitpleDefinitionOfSymbolException();
    if(ret == -4) throw UndefinedSymbolException();
  }
  catch(const exception& e){

    cerr << e.what() << '\n';
  }
  
  
  return 0;
}
