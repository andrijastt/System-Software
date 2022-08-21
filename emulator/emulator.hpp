#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iomanip>

using namespace std;

class Emulator{

public:

  Emulator(string inputFileString);
  int emulate();
  
private:

  bool openFile();
  void loadMemory();
  vector<string> hexToCode(string num);

  ifstream inputFile;
  string inputFileName;
  bool stop = false;
  vector<string> Memory;
  string psw = "0000000000000000";
  int reg[9];   // r[0-7] + psw

  enum Registers{ R0, R1, R2, R3, R4, R5, R6, SP = R6, R7, PC = R7, R8, PSW = R8, ERRORREG};
  enum AddressType{ IMMED, REGDIR, REGIND, REGINDPOM, MEMDIR, REGDIRPOM, ERRORADDR};
  enum AddressUpdate{ NOUPD, DECBEFORE, INCBEFORE, DECAFTER, INCAFTER, ERRORUPD};
  enum OPCode{ HALT, INT, IRET, CALL, RET, JUMP, XCHG, ARITHMETIC, LOGIC, SHIFT, LOAD, STORE, ERROROP};
  enum JumpInstr{ NOTJMP, JMP, JEQ, JNE, JGT, ERRORJMP};
  enum ArithmeticInstr{ NOTARITHMETIC, ADD, SUB, MUL, DIV, CMP, ERRORARITHMETHIC};
  enum LogicInstr{ NOTLOGIC, NOT, AND, OR, XOR, TEST, ERRORLOGIC};
  enum ShiftInstr{ NOTSHIFT, SHL, SHR, ERRORSHIFT};
  enum InterruptType{ NOTMASKED, MASKED};

  struct Instruction{
    AddressType addressType;
    AddressUpdate addressUpdate;
    OPCode operation;
    JumpInstr jump = NOTJMP;
    ArithmeticInstr arithemtic = NOTARITHMETIC;
    LogicInstr logic = NOTLOGIC;
    ShiftInstr shift = NOTSHIFT;
    int size = 0;
    Registers regD;
    Registers regS;
    string dataLow, dataHigh;
  };

  struct Interrupt{
    InterruptType interruptType;
    int entry;
  };

  Instruction instruction;
  Interrupt interrupt;

  OPCode getOPCode(char s);
  JumpInstr getJumpType(char s);
  Registers getRegister(char s);
  AddressType getAddressType(char s);
  AddressUpdate getAddressUpdate(char s);
  ArithmeticInstr getArithmeticInstr(char s);
  LogicInstr getLogicInstr(char s);
  ShiftInstr getShiftInstr(char s);
  JumpInstr getJumpInstr(char s);
  int getRegIndex(Registers reg);  
  void PCJumpChange();

  void getInstruction();
  void execute();

  void _halt();
  void _int();
  void _iret();
  void _call();
  void _ret();
  void _jmp();
  void _jeq();
  void _jne();
  void _jgt();
  void _xchg();
  void _add();
  void _sub();
  void _mul();
  void _div();
  void _cmp();
  void _not();
  void _and();
  void _or();
  void _xor();
  void _test();
  void _shl();
  void _shr();
  void _load();
  void _store();

};
