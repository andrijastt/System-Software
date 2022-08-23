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
  for(int i = 0; i < 9; i++){
    reg[i] = 0;
  }
  Interrupts.clear();
}

/**
 * @brief turns dec number to machine code
 * 
 * @param num the number that need to be changed
 * @return vector<string> helper machine code
 */
vector<string> Emulator::decToCode(string num){
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
 * @brief Returns what instruction is currently
 * 
 * @return string 
 */
string Emulator::toStringOPCode(){
  if(instruction.operation == HALT) return "HALT";
  else if(instruction.operation == INT) return "INT";
  else if(instruction.operation == IRET) return "IRET";
  else if(instruction.operation == CALL) return "CALL";
  else if(instruction.operation == RET) return "RET";
  else if(instruction.operation == JUMP) return "JUMP";
  else if(instruction.operation == XCHG) return "XCHG";
  else if(instruction.operation == ARITHMETIC) return "ARITHMETIC";
  else if(instruction.operation == LOGIC) return "LOGIC";
  else if(instruction.operation == SHIFT) return "SHIFT";
  else if(instruction.operation == LOAD) return "LOAD";
  else if(instruction.operation == STORE) return "STORE";
  else return "ERROROP";
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
 * @brief Returns what type of jump it is
 * 
 * @return string 
 */
string Emulator::toStringJumpType(){
  if(instruction.jump == JMP) return "JMP";
  else if(instruction.jump == JEQ) return "JEQ";
  else if(instruction.jump == JNE) return "JNE";
  else if(instruction.jump == JGT) return "JGT";
  else if(instruction.jump == NOTJMP) return "NOTJMP";
  else return "ERRORJMP";
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
 * @brief Return what register it is in string
 * 
 * @param reg Registwer we want to convert to string
 * @return string 
 */
string Emulator::toStringRegister(Emulator::Registers reg){
  if(reg == R0) return "R0";
  else if(reg == R1) return "R1";
  else if(reg == R2) return "R2";
  else if(reg == R3) return "R3";
  else if(reg == R4) return "R4";
  else if(reg == R5) return "R5";
  else if(reg == R6) return "SP";
  else if(reg == R7) return "PC";
  else if(reg == R8) return "PSW";
  else return "ERRORREG";
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
 * @brief Return string value of address type
 * 
 * @return string 
 */
string Emulator::toStringAddressType(){
  if(instruction.addressType == IMMED) return "IMMED";
  else if(instruction.addressType == REGDIR) return "REGDIR";
  else if(instruction.addressType == REGIND) return "REGIND";
  else if(instruction.addressType == REGINDPOM) return "REGINDPOM";
  else if(instruction.addressType == MEMDIR) return "MEMDIR";
  else if(instruction.addressType == REGDIRPOM) return "REGDIRPOM";
  else return "ERRORADDR";
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

string Emulator::toStringAddressUpdate(){
  if(instruction.addressUpdate == NOUPD) return "NOUPD";
  else if(instruction.addressUpdate == DECBEFORE) return "DECBEFORE";
  else if(instruction.addressUpdate == INCBEFORE) return "INCBEFORE";
  else if(instruction.addressUpdate == DECAFTER) return "DECAFTER";
  else if(instruction.addressUpdate == INCAFTER) return "INCAFTER";
  else return "ERRORUPD";
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
    default:
      return -1;
  }

}

/**
 * @brief Adds interrupt to Interrupts
 * 
 */
void Emulator::addInterrupt(){
  Interrupt interrupt;
  interrupt.entry = 1;
  interrupt.interruptType = NOTMASKED;
  Interrupts.push_back(interrupt);
}

/**
 * @brief Checks if there are unmasked interrupts
 * 
 * @return int -1 no unmasked interrupts
 */
int Emulator::checkUnmaskedInterrupts(){
  if(Interrupts.size() == 0) return -1;

  int ret = 0;
  for(Interrupt intt: Interrupts){
    if(intt.interruptType == NOTMASKED) return ret;
    ret++;
  }
  return -1;
}

/**
 * @brief PC <= operand, has same code for jump operations so to shorten code
 * 
 */
void Emulator::PCJumpChange(){

  stringstream ss;
  string data, dataLow, dataHigh;
  unsigned int indexS, dataInt;
  switch(instruction.addressType){
    case IMMED:
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> reg[7];
      break;

    case REGDIR:
      indexS = getRegIndex(instruction.regS);
      if(indexS == -1){
        addInterrupt();
      } else {
        reg[7] = reg[indexS];
      }
      break;

    case REGINDPOM:
      indexS = getRegIndex(instruction.regS);
      if(indexS == -1){
        addInterrupt();
        break;
      }
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> dataInt;

      switch(instruction.addressUpdate){
        case NOUPD:
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
          data = Memory[reg[indexS] + dataInt + 1] + Memory[reg[indexS] + dataInt];
          reg[indexS] -= 2;     
          break;

        case INCAFTER:
          data = Memory[reg[indexS] + dataInt + 1] + Memory[reg[indexS] + dataInt];
          reg[indexS] += 2;         
          break; 
      }
      reg[7]= stoul(data, nullptr, 16); 
      break;

    case MEMDIR:
      data = Memory[dataInt + 1] + Memory[dataInt];
      ss << data;
      ss >> hex >> reg[7];
      break;

    case REGDIRPOM:
      indexS = getRegIndex(instruction.regS);
      if(indexS == -1){
        addInterrupt();
        break;
      }
      data = instruction.dataHigh + instruction.dataLow;

      if(data[0] == '8' || data[0] == '9' || data[0] == 'A' || data[0] == 'B' || data[0] == 'C' || data[0] == 'D' 
      || data[0] == 'E' || data[0] == 'F') 
        data  = "FFFF" + data;

      ss << data;
      ss >> hex >> dataInt;
      reg[7] = dataInt + reg[indexS];
      break;

    case REGIND:
      indexS = getRegIndex(instruction.regS);
      if(indexS == -1){
        addInterrupt();
        break;
      }
      switch(instruction.addressUpdate){
        case NOUPD:
          data = Memory[reg[indexS] + 1] + Memory[reg[indexS]];
          break;

        case DECBEFORE:
          reg[indexS] -= 2;
          data = Memory[reg[indexS] + 1] + Memory[reg[indexS]];
          break;

        case INCBEFORE:
          reg[indexS] += 2;
          data = Memory[reg[indexS] + 1] + Memory[reg[indexS]];

        case DECAFTER:
          data = Memory[reg[indexS] + 1] + Memory[reg[indexS]];
          reg[indexS] -= 2;     
          break;

        case INCAFTER:
          data = Memory[reg[indexS] + 1] + Memory[reg[indexS]];
          reg[indexS] += 2;         
          break; 
      }
      ss << data;
      ss >> hex >> reg[7];

      break;

    default: /// TODO add Interrupt for ERRORTYPE
      addInterrupt();
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
  
  int indexD = getRegIndex(instruction.regD);
  if(indexD == -1){   // if it didnt't read good register
    addInterrupt();
  } else {
    // stack part
    reg[6] -= 2;
    // Memory[reg[6]] = psw; // TODO
    vector<string> helper = decToCode(to_string(reg[7]));
    Memory[reg[6] + 1] = helper[0];
    Memory[reg[6]] = helper[1];

    reg[6] -= 2;
    helper = decToCode(to_string(reg[8]));
    Memory[reg[6] + 1] = helper[0];
    Memory[reg[6]] = helper[1];
    
    string help = Memory[(reg[indexD] % 8) * 2 + 1] + Memory[(reg[indexD] % 8) * 2];
    stringstream ss;
    ss << help;
    ss >> hex >> reg[7];
  }
  
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
  ss >> hex >> reg[8];  // TODO check psw

  string help2 = Memory[reg[6] + 1] + Memory[reg[6]];
  reg[7] = stoul(help2, nullptr, 16);  
  reg[6] += 2;

}

/**
 * @brief Emulate call instruction
 * 
 */
void Emulator::_call(){

  reg[6]-=2;
  string pc = to_string(reg[7]);
  vector<string> helper = decToCode(pc);

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
  if(reg[8] & 1){
    PCJumpChange();
  }
}

/**
 * @brief Emulate jne instruction
 * 
 */
void Emulator::_jne(){
  if(!(reg[8] & 1)){
    PCJumpChange();
  }
}

/**
 * @brief Emulate jgt instruction
 * 
 */
void Emulator::_jgt(){
  // !(get_flag(N) ^ get_flag(O)) & !get_flag(Z)
  if(!(reg[8] | 1) & !(reg[8] | 8 ^ reg[8] | 2)){
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

  if(regDIndex == -1 || regSIndex == -1){
    addInterrupt();
  } else {
    int temp = reg[regDIndex];
    reg[regDIndex] = reg[regSIndex];
    reg[regSIndex] = temp;
  }
}

/**
 * @brief Emulate add instruction
 * 
 */
void Emulator::_add(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  if(regDIndex == -1 || regSIndex == -1){
    addInterrupt();
  } else{
    reg[regDIndex] += reg[regSIndex];
  }
  
}

/**
 * @brief Emulate sub instruction
 * 
 */
void Emulator::_sub(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  if(regDIndex == -1 || regSIndex == -1){
    addInterrupt();
  } else{
    reg[regDIndex] -= reg[regSIndex];
  }
}

/**
 * @brief Emulate mul instruction
 * 
 */
void Emulator::_mul(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  if(regDIndex == -1 || regSIndex == -1){
    addInterrupt();
  } else{
    reg[regDIndex] *= reg[regSIndex];
  }
}

/**
 * @brief Emulate div instruction
 * 
 */
void Emulator::_div(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  if(regDIndex == -1 || regSIndex == -1){
    addInterrupt();
  } else{
    reg[regDIndex] /= reg[regSIndex];
  }
}

/**
 * @brief Emulate cmp instruction
 * 
 */
void Emulator::_cmp(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  if(regDIndex == -1 || regSIndex == -1){
    addInterrupt();
  } else {
    int newPSW = 0, temp = reg[regDIndex] - reg[regSIndex];
    if( (reg[regDIndex] < 0 && reg[regSIndex] > 0 && temp > 0) || (reg[regDIndex] > 0 && reg[regSIndex] < 0 && temp < 0) ){
      newPSW += 2;
    }
    if(reg[regDIndex] < reg[regSIndex]){
      newPSW += 4;
    }
    if(temp == 0){
      newPSW += 1;
    }
    if(temp < 0){
      newPSW += 8;
    }
    reg[8] = newPSW;
  }
}

/**
 * @brief Emulate not instruction
 * 
 */
void Emulator::_not(){
  int regDIndex = getRegIndex(instruction.regD);
  if(regDIndex == -1) addInterrupt();
  else reg[regDIndex] = ~reg[regDIndex];               // TODO
}

/**
 * @brief Emulates and instruction
 * 
 */
void Emulator::_and(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  if(regDIndex == -1 || regSIndex == -1){
    addInterrupt();
  } else {
    reg[regDIndex] &= reg[regSIndex];
  } 
}

/**
 * @brief Emulates or instruction
 * 
 */
void Emulator::_or(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  if(regDIndex == -1 || regSIndex == -1){
    addInterrupt();
  } else {
    reg[regDIndex] |= reg[regSIndex];
  }
}

/**
 * @brief Emulates xor instruction
 * 
 */
void Emulator::_xor(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  if(regDIndex == -1 || regSIndex == -1){
    addInterrupt();
  } else {
    reg[regDIndex] ^= reg[regSIndex];
  }
}

/**
 * @brief Emulates test instruction
 * 
 */
void Emulator::_test(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  if(regDIndex == -1 || regSIndex == -1){
    addInterrupt();
  } else {
    int newPSW = 0, temp = reg[regDIndex] - reg[regSIndex];
    if(temp == 0){
      newPSW += 1;
    }
    if(temp < 0){
      newPSW += 8;
    }
    reg[8] = newPSW;
  }
}

/**
 * @brief Emulate shl instruction
 * 
 */
void Emulator::_shl(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  if(regDIndex == -1 || regSIndex == -1){
    addInterrupt();
  } else {
    int temp = reg[regDIndex];
    reg[regDIndex] << reg[regSIndex];

    // TODO UPDATE PSW
    int newPSW = 0;
    if(reg[regDIndex] < temp){
      newPSW += 4;
    }
    if(reg[regDIndex] == 0){
      newPSW += 1;
    }
    if(reg[regDIndex] < 0){
      newPSW += 8;
    }
    reg[8] = newPSW;
  }

}

/**
 * @brief Emulate shr instruction
 * 
 */
void Emulator::_shr(){
  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);
  if(regDIndex == -1 || regSIndex == -1){
    addInterrupt();
  } else {
    int temp = reg[regDIndex];
    reg[regDIndex] >> reg[regSIndex];

    // TODO UPDATE PSW
    int newPSW = 0;
    if(reg[regDIndex] < temp){
      newPSW += 4;
    }
    if(reg[regDIndex] == 0){
      newPSW += 1;
    }
    if(reg[regDIndex] < 0){
      newPSW += 8;
    }
    reg[8] = newPSW;
  }
}

/**
 * @brief Emulate ldr and pop instruction
 * 
 */
void Emulator::_load(){ //

  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);

  if(regDIndex == -1){
    addInterrupt();
    return;
  }

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
      if(regSIndex == -1){
        addInterrupt();
      } else {
        reg[regDIndex] = reg[regSIndex];
      }
      break;

    case REGINDPOM:
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> dataInt;

      if(regSIndex == -1){
        addInterrupt();
        break;
      }

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
          reg[regSIndex] += 2;         
          break; 
      }
      reg[regDIndex]= stoul(data, nullptr, 16); 
      break;

    case MEMDIR:
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> dataInt;
      data = Memory[dataInt + 1] + Memory[dataInt];
      reg[regDIndex] = stoul(data, nullptr, 16);
      break;

    case REGDIRPOM:
      if(regSIndex == -1){
        addInterrupt();
        break;
      }
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> dataInt;
      reg[regDIndex] = dataInt + reg[regSIndex];
      break;

    case REGIND:
      if(regSIndex == -1){
        addInterrupt();
        break;
      }
      switch(instruction.addressUpdate){
        case NOUPD:
          data = Memory[reg[regSIndex] + 1] + Memory[reg[regSIndex]];
          break;

        case DECBEFORE:
          reg[regSIndex] -= 2;
          data = Memory[reg[regSIndex] + 1] + Memory[reg[regSIndex]];
          break;

        case INCBEFORE:
          reg[regSIndex] += 2;
          data = Memory[reg[regSIndex]+ 1] + Memory[reg[regSIndex]];
          break;

        case DECAFTER:
          // reg[7] = Memory[reg[indexS] + dataInt];
          data = Memory[reg[regSIndex] +1] + Memory[reg[regSIndex]];
          reg[regSIndex] -= 2;     
          break;

        case INCAFTER:
          // reg[7] = Memory[reg[indexS] + dataInt];
          data = Memory[reg[regSIndex] + 1] + Memory[reg[regSIndex]];
          reg[regSIndex] += 2;         
          break; 
      }
      ss << data;
      ss >> hex >> reg[regDIndex];
      break;

    default: /// TODO add Interrupt for ERRORTYPE
      addInterrupt();
      break;

  }

}

/**
 * @brief Emulate push and str instructions
 * 
 */
void Emulator::_store(){  // TODO

  int regDIndex = getRegIndex(instruction.regD);
  int regSIndex = getRegIndex(instruction.regS);

  if(regDIndex == -1){
    addInterrupt();
    return;
  }

  stringstream ss;
  string data, dataLow, dataHigh;
  int dataInt;
  vector<string> helper;

  switch(instruction.addressType){
    case REGDIR:
      if(regSIndex == -1){
        addInterrupt();
      } else {
        reg[regSIndex] = reg[regDIndex];
      }
      break;

    case REGINDPOM:
      if(regSIndex == -1){
        addInterrupt();
        break;
      }
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
          reg[regSIndex] += 2;         
          break; 
      }
      ss << data;
      ss >> hex >> dataInt;

      helper = decToCode(to_string(reg[regDIndex]));
      Memory[dataInt + 1] = helper[0];
      Memory[dataInt] = helper[1];

      break;

    case MEMDIR:
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> dataInt;
      helper = decToCode(to_string(reg[regDIndex]));
      Memory[dataInt + 1] = helper[0];
      Memory[dataInt] = helper[1];
      break;

    case REGDIRPOM:
      if(regSIndex == -1){
        addInterrupt();
        break;
      }
      data = instruction.dataHigh + instruction.dataLow;
      ss << data;
      ss >> hex >> dataInt;
      reg[regSIndex] = dataInt + reg[regDIndex];
      break;

    case REGIND:
      if(regSIndex == -1){
        addInterrupt();
        break;
      }

      switch(instruction.addressUpdate){
        case NOUPD:
          helper = decToCode(to_string(reg[regDIndex]));
          Memory[reg[regSIndex] + 1] = helper[0];
          Memory[reg[regSIndex]] = helper[1];
          break;

        case DECBEFORE:
          reg[regSIndex] -= 2;
          helper = decToCode(to_string(reg[regDIndex]));
          Memory[reg[regSIndex]+ 1] = helper[0];
          Memory[reg[regSIndex]] = helper[1];
          break;

        case INCBEFORE:
          reg[regSIndex] += 2;
          helper = decToCode(to_string(reg[regDIndex]));
          Memory[reg[regSIndex]+ 1] = helper[0];
          Memory[reg[regSIndex]] = helper[1];
          break;

        case DECAFTER:
          // reg[7] = Memory[reg[indexS] + dataInt];
          helper = decToCode(to_string(reg[regDIndex]));
          Memory[reg[regSIndex]+ 1] = helper[0];
          Memory[reg[regSIndex]] = helper[1];
          reg[regSIndex] -= 2;     
          break;

        case INCAFTER:
          // reg[7] = Memory[reg[indexS] + dataInt];
          helper = decToCode(to_string(reg[regDIndex]));
          Memory[reg[regSIndex]+ 1] = helper[0];
          Memory[reg[regSIndex]] = helper[1];
          reg[regSIndex] += 2;         
          break; 
      }
      break;

    default: /// TODO add Interrupt for IMMED and ERRORTYPE
      addInterrupt();
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

}

/**
 * @brief Executes set instruction
 * 
 */
void Emulator::execute(){

  switch (instruction.operation){
    case HALT:
      _halt();
      break;

    case INT:
      _int();
      break;
    
    case IRET:
      _iret();
      break;

    case CALL:
      _call();
      break;

    case RET:
      _ret();
      break;

    case JUMP:
      switch(instruction.jump){
        case JMP:
          _jmp();
          break;

        case JEQ:
          _jeq();
          break;

        case JNE:
          _jne();
          break;

        case JGT:
          _jgt();
          break;
        
        case ERRORJMP:
          addInterrupt();
          break;

        default:  // TODO
          break;
      }
      break;

    case XCHG:
      _xchg();
      break;

    case ARITHMETIC:
      switch(instruction.arithemtic){
        case ADD:
          _add();
          break;

        case SUB:
          _sub();
          break;

        case MUL:
          _mul();
          break;

        case DIV:
          _div();
          break;

        case CMP:
          _cmp();
          break;
        
        case ERRORARITHMETHIC:
          addInterrupt();
          break;

        default:  // TODO
          break;
      }
      break;

    case LOGIC:
      switch(instruction.logic){
        case NOT:
          _not();
          break;

        case AND:
          _and();
          break;

        case OR:
          _or();
          break;

        case XOR:
          _xor();
          break;

        case TEST:
          _test();
          break;

        case ERRORLOGIC:
          addInterrupt();
          break;

        default:  // TODO
          break;
      }
      break;

    case SHIFT:
      switch(instruction.shift){
        case SHL:
          _shl();
          break;

        case SHR:
          _shr();
          break;

        case ERRORSHIFT:
          addInterrupt();
          break;

        default:  // TODO
          break;
      }
      break;

    case LOAD:
      _load();
      break;

    case STORE:
      _store();
      break;
  
    default:
      break;
  }

}

/**
 * @brief Executes interrupt
 * 
 */
void Emulator::executeInterrupt(Interrupt interrupt){

  string data = Memory[interrupt.entry * 2 + 1] + Memory[interrupt.entry * 2];
  stringstream ss;

  instruction.operation = STORE;
  instruction.regD = PC;
  instruction.regS = SP;
  instruction.addressType = REGIND;
  instruction.addressUpdate = DECBEFORE;

  execute();
  instruction.regD = PSW;
  execute();

  ss << data;
  ss >> dec >> reg[7];

}

/**
 * @brief emulates interrupt
 * 
 */
void Emulator::interrupt(){

  if(Interrupts.size() > 0){

    if(reg[8] | 32768){     // reg I == 1
      executeInterrupt(Interrupts[0]);
      Interrupts.erase(Interrupts.begin());
    } else{
      int ret = checkUnmaskedInterrupts();
      if(ret != -1){          // there are unmasked interupts
        executeInterrupt(Interrupts[ret]);
        Interrupts.erase(Interrupts.begin() + ret);
      }
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

  ofstream helperStream;
  helperStream.open("helper.emulator.hex", ios::out|ios::trunc);

  while(true){

    getInstruction();
    helperStream << toStringOPCode() << "\t" << toStringAddressType() << "\t" << toStringAddressUpdate() << " RegD " << toStringRegister(instruction.regD) 
    << "\tRegS " << toStringRegister(instruction.regS)  << "\t" << instruction.dataHigh << instruction.dataLow << endl;

    for(int i = 0; i < 9; i++){
      helperStream << "R[" << i << "]:\t" << reg[i] << "\t\t";
      if(i % 3 == 2) helperStream << endl;
    }
    helperStream << endl << endl;
    reg[7] += instruction.size;

    if(instruction.operation == ERROROP){
      addInterrupt();
    } else {
      execute();
    }

    if(stop){
      break;
    }
    interrupt();
  }

  cout << "------------------------------------------------\n"
  << "Emulated processor executed halt instruction\n"
  << "Emulated processor state: psw=0b";

  int help = 32768;
  while(help > 0){
    if(reg[8] & help) cout << "1";
    else cout << "0";
    help /= 2;
  }
  cout << endl;
  for(int i = 0; i < 8; i++){
    cout << "r" << i << "=0x" << hex << setfill('0') << setw(4) << reg[i];

    if(i == 3){
      cout << endl;
    } else {
      cout << "\t";
    }
  }
  cout << endl;

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