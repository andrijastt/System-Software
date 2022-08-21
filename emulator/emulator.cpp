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
 * @brief Returns Operation code for set byte
 * 
 * @param s input byte
 * @return Emulator::OPCode 
 */
Emulator::OPCode Emulator::getOPCode(char s){
  if(s == '0') return HALT;
  else if(s == '1') return INT;
  else if(s == '2') return IRET;
  else if(s == '3') return CALL;
  else if(s == '4') return RET;
  else if(s == '5') return JUMP;
  else if(s == '6') return XCHG;
  else if(s == '7') return ARITHMETIC;
  else if(s == '8') return LOGIC;
  else if(s == '9') return SHIFT;
  else if(s == 'A') return LOAD;
  else if(s == 'B') return STORE;
  else return ERROROP;
}

/**
 * @brief Return what type of Jump it is
 * 
 * @param s input byte
 * @return Emulator::JumpInstr 
 */
Emulator::JumpInstr Emulator::getJumpType(char s){
  if(s == '0') return JMP;
  else if(s == '1') return JEQ;
  else if(s == '2') return JNE;
  else if(s == '3') return JGT;
  else ERRORJMP;
}

/**
 * @brief Return what register it is
 * 
 * @param s input byte
 * @return Emulator::Registers 
 */
Emulator::Registers Emulator::getRegister(char s){
  if(s == '0') return R0;
  else if(s == '1') return R1;
  else if(s == '2') return R2;
  else if(s == '3') return R3;
  else if(s == '4') return R4;
  else if(s == '5') return R5;
  else if(s == '6') return R6;
  else if(s == '7') return R7;
  else if(s == '8') return R8;
  else ERRORREG;
}

/**
 * @brief Return what type of address it is
 * 
 * @param s input byte
 * @return Emulator::AddressType 
 */
Emulator::AddressType Emulator::getAddressType(char s){
  if(s == '0') return IMMED;
  else if(s == '1') return REGDIR;
  else if(s == '2') return REGIND;
  else if(s == '3') return REGINDPOM;
  else if(s == '4') return MEMDIR;
  else if(s == '5') return REGDIRPOM;
  else return ERRORADDR;
}

/**
 * @brief Return what type of address update it is
 * 
 * @param s input byte
 * @return Emulator::AddressUpdate 
 */
Emulator::AddressUpdate Emulator::getAddressUpdate(char s){
  if(s == '0') return NOUPD;
  else if(s == '1') return DECBEFORE;
  else if(s == '2') return INCBEFORE;
  else if(s == '3') return DECAFTER;
  else if(s == '4') return INCAFTER;
  else ERRORUPD;
}

/**
 * @brief Return what type of arithmetic instruction it is
 * 
 * @param s input byte
 * @return Emulator::ArithmeticInstr 
 */
Emulator::ArithmeticInstr Emulator::getArithmeticInstr(char s){
  if(s == '0') return ADD;
  else if(s == '1') return SUB;
  else if(s == '2') return MUL;
  else if(s == '3') return DIV;
  else if(s == '4') return CMP;
  else ERRORARITHMETHIC;
}

/**
 * @brief Return what type of logic instruction it is
 * 
 * @param s input byte
 * @return Emulator::LogicInstr 
 */
Emulator::LogicInstr Emulator::getLogicInstr(char s){
  if(s == '0') return NOT;
  else if(s == '1') return AND;
  else if(s == '2') return OR;
  else if(s == '3') return XOR;
  else if(s == '4') return TEST;
  else ERRORLOGIC;
}


Emulator::ShiftInstr Emulator::getShiftInstr(char s){
  if(s == '0') return SHL;
  else if(s == '1') return SHR;
  else return ERRORSHIFT;
}

/**
 * @brief sets Emulator::instruction to set instruction from reading
 * 
 */
void Emulator::getInstruction(){

  string byte1 = Memory[reg[7]];
  instruction.operation = getOPCode(byte1[0]);
  instruction.size = 1;

  // if here is 1B instruction nothing happens
  if(instruction.operation != HALT && instruction.operation != IRET && instruction.operation != RET){
    string regs = Memory[reg[7] + 1];
    instruction.regD = getRegister(regs[0]);
    instruction.regS = getRegister(regs[1]);
    instruction.size++;
  }

  // here it is 3B or 5B instruction
  if(instruction.operation != INT && instruction.operation != XCHG && instruction.operation != ARITHMETIC && instruction.operation != LOGIC 
  && instruction.operation != SHIFT){
    string addrTyp = Memory[reg[7] + 2];
    instruction.addressType = getAddressType(addrTyp[1]);

    if(instruction.addressType == REGIND || instruction.addressType == REGINDPOM){
      instruction.addressUpdate = getAddressUpdate(addrTyp[0]);
    } else {
      instruction.addressUpdate = NOUPD;
    }

  } else {  // it is 2B instruction

    switch(instruction.operation){

      case INT: // already has all info needed
        instruction.arithemtic = NOTARITHMETIC;
        instruction.logic = NOTLOGIC;
        instruction.load = NOTLOAD;
        instruction.store = NOTSTORE;
        instruction.shift = NOTSHIFT;
        instruction.jump = NOTJMP;
        break;

      case XCHG:  // already has all info needed
        instruction.arithemtic = NOTARITHMETIC;
        instruction.logic = NOTLOGIC;
        instruction.load = NOTLOAD;
        instruction.store = NOTSTORE;
        instruction.shift = NOTSHIFT;
        instruction.jump = NOTJMP;
        break;

      case ARITHMETIC:
        instruction.arithemtic = getArithmeticInstr(byte1[1]);
        instruction.logic = NOTLOGIC;
        instruction.load = NOTLOAD;
        instruction.store = NOTSTORE;
        instruction.shift = NOTSHIFT;
        instruction.jump = NOTJMP;
        break;

      case LOGIC:
        instruction.arithemtic = NOTARITHMETIC;
        instruction.logic = getLogicInstr(byte1[1]);
        instruction.load = NOTLOAD;
        instruction.store = NOTSTORE;
        instruction.shift = NOTSHIFT;
        instruction.jump = NOTJMP;
        break;

      case SHIFT:
        instruction.arithemtic = NOTARITHMETIC;
        instruction.logic = NOTLOGIC;
        instruction.load = NOTLOAD;
        instruction.store = NOTSTORE;
        instruction.shift = getShiftInstr(byte1[1]);
        instruction.jump = NOTJMP;
        break;
    }

  }

}

/**
 * @brief Emulation
 * 
 * @return int 0 - everything is good, -1 input file doesn't exist
 */
int Emulator::emulate(){

  if(!openFile()) return -1;
  loadMemory();

  string startPC = Memory[1] + Memory[0];
  stringstream ss;
  ss << startPC;
  ss >> hex >> reg[7];
  cout << reg[7] << endl;

  while(true){

    break;

  }

  cout << "------------------------------------------------\nEmulated processor executed halt instruction\nEmulated processor state: psw=";

  return 0;
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