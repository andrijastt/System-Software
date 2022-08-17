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
 * @brief Linker links all input files
 * 
 * @return int 0 - everything is okay, -1 - wrong terminal input, -2 - some input files don't exist
 * 
 */
int Linker::link(){

  ofstream linkerHelper;
  string helper = "linkerHelper.hex";
  linkerHelper.open(helper, ios::out|ios::trunc);

  Symbol oldSection;
  vector<Symbol> currentSymbols;
  int currentRelocations;

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
        else if(line == "DEF") symb.defined = true;
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
            relo.symbolId = stoi(params[2]);
            sscanf(params[3].c_str(), "%X", &relo.offset);

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

    }

    inputFile.close();
  }

  for(Section s: Sections){
    linkerHelper << s.id << "\t" << s.size << "\t" << s.name << "\n";
  }
  linkerHelper << endl << endl;
  for(Symbol s: Symbols){
    linkerHelper << s.id << "\t" << s.offset << "\t" << s.sectionId << "\t" << s.symbolName << "\t";

    switch(s.bind){
      case GLOBAL:
        linkerHelper << "GLOBAL" << endl;
        break;

      case LOCAL:
        linkerHelper << "LOCAL" << endl;
        break;

      case NOBIND:
      linkerHelper << "NOBIND" << endl;
      break;
    }
  }
  linkerHelper << endl << endl;

  for(Relocations rels: allRelocations){

    linkerHelper << "Relocations from section " << rels.name << "\t FILE NAME: " << rels.fileName << endl;

    for(Relocation rel: rels.relocations){
      linkerHelper << rel.offset << "\t";
      switch(rel.type){
        case R_16:
          linkerHelper << "R_16\t";
          break;

        case R_PC16:
          linkerHelper << "R_PC16\t";
      }
      linkerHelper << rel.symbolId << "\t" << rel.addend << endl;
    }
    linkerHelper << endl;
  }

  linkerHelper.close();

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

  }
  catch(const exception& e){

    cerr << e.what() << '\n';
  }
  
  
  return 0;
}
