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
 * @brief Turns hex number to machine code
 * 
 * @param num the number that need to be changed
 * @return vector<string> helper machine code
 */
vector<string> Emulator::hexToCode(string num){

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
  else return ERRORJMP;
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
  else return ERRORREG;
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
  else return ERRORUPD;
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
  else return ERRORARITHMETHIC;
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
  else return ERRORLOGIC;
}

/**
 * @brief Return what type of shift instruction it is
 * 
 * @param s input byte
 * @return Emulator::ShiftInstr 
 */
Emulator::ShiftInstr Emulator::getShiftInstr(char s){
  if(s == '0') return SHL;
  else if(s == '1') return SHR;
  else return ERRORSHIFT;
}

/**
 * @brief Return what type of jump instruction it is
 * 
 * @param s input byte
 * @return Emulator::JumpInstr 
 */
Emulator::JumpInstr Emulator::getJumpInstr(char s){
  if(s == '0') return JMP;
  else if(s == '1') return JEQ;
  else if(s == '2') return JNE;
  else if(s == '3') return JGT;
  else return ERRORJMP;
}

/**
 * @brief Returns the index of register
 * 
 * @param reg The index of register we want
 * @return int What index is
 */
int Emulator::getRegIndex(Registers reg){

  switch(reg){
    case R0:
      return 0;
    case R1:
      return 1;
    case R2:
      return 2;
    case R3:
      return 3;
    case R4:
      return 4;
    case R5:
      return 5;
    case R6:
      return 6;
    case R7:
      return 7;
    case R8:
      return 8;
    case ERRORREG:
      return -1;
    default:
      return -1;
  }

}

/**
 * @brief PC <= operand, has same code for jump operations so to shorten codde
 * 
 */
void Emulator::PCJumpChange(){

  stringstream ss;
  string data, dataLow, dataHigh;
  int indexS, dataInt;
  switch(instruction.addressType){
    case IMMED:
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> reg[7];
      break;

    case REGDIR:
      indexS = getRegIndex(instruction.regS);
      reg[7] = reg[indexS];
      break;

    case REGINDPOM:
      indexS = getRegIndex(instruction.regS);
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> dataInt;

      switch(instruction.addressUpdate){
        case NOUPD:
          // reg[7] = Memory[reg[indexS] + dataInt];
          data = Memory[reg[indexS] + dataInt + 1] + Memory[reg[indexS] + dataInt];
          break;

        case DECBEFORE:
          reg[indexS] -= 2;
          data = Memory[reg[indexS] + dataInt + 1] + Memory[reg[indexS] + dataInt];
          break;

        case INCBEFORE:
          reg[indexS] += 2;
          data = Memory[reg[indexS] + dataInt + 1] + Memory[reg[indexS] + dataInt];
          break;

        case DECAFTER:
          // reg[7] = Memory[reg[indexS] + dataInt];
          data = Memory[reg[indexS] + dataInt + 1] + Memory[reg[indexS] + dataInt];
          reg[indexS] -= 2;     
          break;

        case INCAFTER:
          // reg[7] = Memory[reg[indexS] + dataInt];
          data = Memory[reg[indexS] + dataInt + 1] + Memory[reg[indexS] + dataInt];
          reg[indexS] -= 2;         
          break; 
      }
      ss << data;
      ss >> hex >> reg[7];

      break;

    case MEMDIR:
      data = Memory[dataInt + 1] + Memory[dataInt];
      ss << data;
      ss >> hex >> reg[7];
      break;

    case REGDIRPOM:
      indexS = getRegIndex(instruction.regS);
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> dataInt;
      reg[7] = dataInt + reg[indexS];
      break;
  }
}

/**
 * @brief Emulates halt instruction
 * 
 */
void Emulator::_halt(){
  stop = true;
}

/**
 * @brief Emulates int instruction
 * 
 */
void Emulator::_int(){
  // stack part
  reg[6] -= 2;
  Memory[reg[6]] = psw; // TODO

  // new pc
  int indexD = getRegIndex(instruction.regD);
  string help = Memory[(reg[indexD] % 8) + 1] + Memory[(reg[indexD] % 8)];
  stringstream ss;
  ss << help;
  ss >> hex >> reg[7];
}

/**
 * @brief Emulate iret instruction
 * 
 */
void Emulator::_iret(){ // TODO check

  string help1 = Memory[reg[6] + 1] + Memory[reg[6]];
  reg[6] += 2;

  stringstream ss;
  ss << help1;
  ss >> hex >> reg[8];

  help1 = Memory[reg[6] + 1] + Memory[reg[6]];
  reg[6] += 2;

  ss << help1;
  ss >> hex >> reg[7];

}

/**
 * @brief Emulate call instruction
 * 
 */
void Emulator::_call(){

  reg[6]-=2;
  string pc = to_string(reg[7]);
  vector<string> helper = hexToCode(pc);

  Memory[reg[6]] = helper[1];               // TODO check
  Memory[reg[6] + 1] = helper[0];

  PCJumpChange();
}

/**
 * @brief Emulate ret instruction
 * 
 */
void Emulator::_ret(){
  string help1 = Memory[reg[6] + 1] + Memory[reg[6]];
  reg[6] += 2;

  stringstream ss;
  ss << help1;
  ss >> hex >> reg[7];
}

/**
 * @brief Emulate jmp instruction
 * 
 */
void Emulator::_jmp(){

  PCJumpChange();
}

/**
 * @brief Emulate jeq instruction
 * 
 */
void Emulator::_jeq(){
  if(reg[8] | 1){
    PCJumpChange();
  }
}

/**
 * @brief Emulate jne instruction
 * 
 */
void Emulator::_jne(){
  if(!(reg[8] | 1)){
    PCJumpChange();
  }
}

/**
 * @brief Emulate jgt instruction
 * 
 */
void Emulator::_jgt(){
  if(!(reg[8] | 1) && !(reg[8] | 8 && reg[8] | 2)){
    PCJumpChange();
  }
}

/**
 * @brief Emulate xchg instruction
 * 
 */
void Emulator::_xchg(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);

  int temp = reg[regDIndex];
  reg[regDIndex] = reg[regSIndex];
  reg[regSIndex] = temp;
}

/**
 * @brief Emulate add instruction
 * 
 */
void Emulator::_add(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  reg[regDIndex] += reg[regSIndex];
}

/**
 * @brief Emulate sub instruction
 * 
 */
void Emulator::_sub(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  reg[regDIndex] -= reg[regSIndex];
}

/**
 * @brief Emulate mul instruction
 * 
 */
void Emulator::_mul(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  reg[regDIndex] *= reg[regSIndex];
}

/**
 * @brief Emulate div instruction
 * 
 */
void Emulator::_div(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  reg[regDIndex] /= reg[regSIndex];
}

/**
 * @brief Emulate sub instruction
 * 
 */
void Emulator::_cmp(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);

  // TODO PSW
}

/**
 * @brief Emulate not instruction
 * 
 */
void Emulator::_not(){
  int regDIndex = getRegIndex(instruction.regD);
  reg[regDIndex] = ~reg[regDIndex];               // TODO
}

/**
 * @brief Emulates and instruction
 * 
 */
void Emulator::_and(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  reg[regDIndex] &= reg[regSIndex];
}

/**
 * @brief Emulates or instruction
 * 
 */
void Emulator::_or(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  reg[regDIndex] |= reg[regSIndex];
}

/**
 * @brief Emulates xor instruction
 * 
 */
void Emulator::_xor(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  reg[regDIndex] ^= reg[regSIndex];
}

/**
 * @brief Emulates test instruction
 * 
 */
void Emulator::_test(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  int temp = reg[regDIndex] - reg[regSIndex];

  // TODO UPDATE PSW
}

/**
 * @brief Emulate shl instruction
 * 
 */
void Emulator::_shl(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  reg[regDIndex] << reg[regSIndex];

  // TODO UPDATE PSW

}

/**
 * @brief Emulate shr instruction
 * 
 */
void Emulator::_shr(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  reg[regDIndex] >> reg[regSIndex];

  // TODO UPDATE PSW

}

/**
 * @brief Emulate ldr and pop instruction
 * 
 */
void Emulator::_load(){ //

  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);

  stringstream ss;
  string data, dataLow, dataHigh;
  int dataInt;
  switch(instruction.addressType){

    case IMMED:
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> reg[regDIndex];
      break;

    case REGDIR:
      reg[regDIndex] = reg[regSIndex];
      break;

    case REGINDPOM:
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> dataInt;

      switch(instruction.addressUpdate){
        case NOUPD:
          data = Memory[reg[regSIndex] + dataInt + 1] + Memory[reg[regSIndex] + dataInt];
          break;

        case DECBEFORE:
          reg[regSIndex] -= 2;
          data = Memory[reg[regSIndex] + dataInt + 1] + Memory[reg[regSIndex] + dataInt];
          break;

        case INCBEFORE:
          reg[regSIndex] += 2;
          data = Memory[reg[regSIndex] + dataInt + 1] + Memory[reg[regSIndex] + dataInt];
          break;

        case DECAFTER:
          // reg[7] = Memory[reg[indexS] + dataInt];
          data = Memory[reg[regSIndex] + dataInt + 1] + Memory[reg[regSIndex] + dataInt];
          reg[regSIndex] -= 2;     
          break;

        case INCAFTER:
          // reg[7] = Memory[reg[indexS] + dataInt];
          data = Memory[reg[regSIndex] + dataInt + 1] + Memory[reg[regSIndex] + dataInt];
          reg[regSIndex] -= 2;         
          break; 
      }
      ss << data;
      ss >> hex >> reg[regDIndex];

      break;

    case MEMDIR:
      data = Memory[dataInt + 1] + Memory[dataInt];
      ss << data;
      ss >> hex >> reg[regDIndex];
      break;

    case REGDIRPOM:
      regSIndex = getRegIndex(instruction.regS);
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> dataInt;
      reg[regDIndex] = dataInt + reg[regSIndex];
      break;

  }

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
  && instruction.operation != SHIFT && instruction.operation != ERROROP){
    string addrTyp = Memory[reg[7] + 2];
    instruction.addressType = getAddressType(addrTyp[1]);

    if(instruction.addressType == REGIND || instruction.addressType == REGINDPOM){
      instruction.addressUpdate = getAddressUpdate(addrTyp[0]);
    } else {
      instruction.addressUpdate = NOUPD;
    }

    instruction.arithemtic = NOTARITHMETIC;
    instruction.logic = NOTLOGIC;
    instruction.shift = NOTSHIFT;

    if(instruction.operation == JUMP){
      instruction.jump = getJumpInstr(byte1[1]);
    } else {
      if(instruction.operation == LOAD || instruction.operation == STORE || instruction.operation == CALL){
        instruction.jump = NOTJMP;
      } 
    }

    if(instruction.addressType == MEMDIR || instruction.addressType == REGINDPOM || instruction.addressType == IMMED 
    || instruction.addressType == REGDIRPOM){
      instruction.size = 5;
      instruction.dataHigh = Memory[reg[7] + 3];
      instruction.dataLow = Memory[reg[7] + 4];
    } else {
      instruction.size = 3;
    }


  } else {  // it is 2B instruction, or ERROROP

    switch(instruction.operation){

      case INT: // already has all info needed
        instruction.arithemtic = NOTARITHMETIC;
        instruction.logic = NOTLOGIC;
        instruction.shift = NOTSHIFT;
        instruction.jump = NOTJMP;
        break;

      case XCHG:  // already has all info needed
        instruction.arithemtic = NOTARITHMETIC;
        instruction.logic = NOTLOGIC;
        instruction.shift = NOTSHIFT;
        instruction.jump = NOTJMP;
        break;

      case ARITHMETIC:
        instruction.arithemtic = getArithmeticInstr(byte1[1]);
        instruction.logic = NOTLOGIC;
        instruction.shift = NOTSHIFT;
        instruction.jump = NOTJMP;
        break;

      case LOGIC:
        instruction.arithemtic = NOTARITHMETIC;
        instruction.logic = getLogicInstr(byte1[1]);
        instruction.shift = NOTSHIFT;
        instruction.jump = NOTJMP;
        break;

      case SHIFT:
        instruction.arithemtic = NOTARITHMETIC;
        instruction.logic = NOTLOGIC;
        instruction.shift = getShiftInstr(byte1[1]);
        instruction.jump = NOTJMP;
        break;

      case ERROROP:
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

    getInstruction();

    if(instruction.operation != ERROROP){

    } else {
      // execute();
    }

    if(stop){
      break;
    }

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