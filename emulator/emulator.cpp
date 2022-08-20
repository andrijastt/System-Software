#include "emulator.hpp"
#include "exceptions.hpp"

/**
 * @brief Checks input data
 * 
 * @param inputFile input files has to be .hex
 * @return true everything is good
 * @return false something is bad
 */
bool checkInputData(string inputFile){

  if(inputFile.substr(inputFile.find_last_of(".")+1) == "hex") return true;
  else return false;
}

/**
 * @brief Construct a new Emulator:: Emulator object
 * 
 * @param inputFile input file name
 */
Emulator::Emulator(string inputFile){
  inputFileName = inputFile;
  Memory.clear();
}

/**
 * @brief Opens input file
 * 
 * @return true input file exists
 * @return false input file does not exist
 */
bool Emulator::openFile(){
  inputFile.open(inputFileName, ios::in);

  if(inputFile.is_open()) return true;
  else return false;
}

/**
 * @brief Loads Memory before emulation
 * 
 */
void Emulator::loadMemory(){

  string line;
  while(getline(inputFile, line)){

    line.erase(0, 6); // remove "XXXX: "

    size_t pos = 0;
    string delimiter = " ";
    while((pos = line.find(delimiter)) != std::string::npos){
      string token = line.substr(0, pos);
      line.erase(0, pos + delimiter.length());
      Memory.push_back(token);
    }
    Memory.push_back(line);     // proveri mozda ne treba to
  }

  for(int i = Memory.size(); i < 65536; i++) Memory.push_back("00");

}

/**
 * @brief Emulation
 * 
 * @return int 0 - everything is good, -1 input file doesn't exist
 */
int Emulator::emulate(){

  if(!openFile()) return -1;
  loadMemory();

}

int main(int argc, char const *argv[]){
  
  try{
    if(!argv[1]) throw InputException();
    if(!checkInputData(argv[1])) throw InputException();

    Emulator emulator(argv[1]);
    int ret = emulator.emulate();

    if(ret == -1) throw NonexistantInputFileException();

    return 0;
  }
  catch(const std::exception& e){
    std::cerr << e.what() << '\n';
  }
  
  
}