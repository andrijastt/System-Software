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

  for(string s: this->inputFileStrings){
    inputFile.open(s, ios::in);

    if(!inputFile.is_open()) return -2;

    string line;
    int current = -1;

    while(getline(inputFile, line)){

      if(line == "") continue;

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
              symb.offset = stoi(token);
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
          Symbols.push_back(symb);

          if(symb.type == SCTN && symb.symbolName != "UND"){
            for(Symbol s: Symbols){
              if(s.sectionId == oldId){
                s.sectionId = symb.id;
              }
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
          }
        }

      }

    }

    inputFile.close();
  }

  for(Section s: Sections){
    cout << s.id << "\t" << s.size << "\t" << s.name << "\n";
  }
  cout << endl << endl;
  for(Symbol s: Symbols){
    cout << s.id << "\t" << s.offset << "\t" << s.sectionId << "\t" << s.symbolName << endl;
  }

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
