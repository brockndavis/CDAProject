// Group #27
// Brock Davis
// Sebastian Hernandez
// Dylan Adams

#include "spimcore.h"

/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
  // decode ALUControl signal
  switch(ALUControl)
  {
    // add operands
    case 0:
      *ALUresult = A + B;
      break;
    // subtract operands
    case 1:
      *ALUresult = A - B;
      break;
    // signed operand comparison
    case 2:
      *ALUresult = ((int)A  < (int)B) ? 1 : 0;
      break;
    // unsigned operand comparison
    case 3:
      *ALUresult = (A < B) ? 1 : 0;
      break;
    // AND operands
    case 4:
      *ALUresult = A & B;
      break;
    // OR operands
    case 5:
      *ALUresult = A | B;
      break;
    // shift second operand 16 bits to the left
    case 6:
      *ALUresult = B << 16;
      break;
    // NOT the first operand
    case 7:
      *ALUresult = ~A;
  }
  // set the Zero flag
  *Zero = (*ALUresult == 0) ? 1 : 0;
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
  // check for valid address
   if (PC % 4 == 0)
   {
     // read instruction from instruction memory
     *instruction = Mem[PC >> 2];
     // success!!
     return 0;
   }
   // invalid PC :(
   return 1;
}

/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
  // opCode is specified in bits 31-26
  *op = (instruction >> 26) & 63;
  // register1 is specified in bits 25-21
  *r1 = (instruction >> 21) & 31;
  // register2 is specified in bits 20-16
  *r2 = (instruction >> 16) & 31;
  // register3 is specified in bits 15-11
  *r3 = (instruction >> 11) & 31;
  // the funct code is in bits 5-0
  *funct = (instruction & 63);
  // the immediate/constant value is in bits 0-15
  *offset = (instruction & 65535);
  // the jump address is in bits 25-0
  *jsec = (instruction & 67108863);

}

// intializes all struct members to the off position
void initializeOpCodes(struct_controls *controls)
{
  controls->RegDst = 0;
  controls->Jump = 0;
  controls->Branch = 0;
  controls->MemRead = 0;
  controls->MemtoReg = 0;
  controls->ALUOp = 0;
  controls->MemWrite = 0;
  controls->ALUSrc = 0;
  controls->RegWrite = 0;
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
  // intialize all signals to 0
  initializeOpCodes(controls);

  // decode operation type
  switch (op)
  {
    // r-type
    case 0:
      // destination register specified in bits 15-11 for r-types
      controls->RegDst = 1;
      // r-type instructions write to register file
      controls->RegWrite = 1;
      // ALU Control must decode using func
      controls->ALUOp = 7;
      break;
    // add immediate
    case 8:
      // instruction writes to register file
      controls->RegWrite = 1;
      // instruction operates on sign extended value
      controls->ALUSrc = 1;
      break;
    // load word
    case 35:
      // reads from memory
      controls->MemRead = 1;
      // selects memory output instead of ALU output to send to register file
      controls->MemtoReg = 1;
      // uses sign extended offset constant
      controls->ALUSrc = 1;
      // instructions writes to register file
      controls->RegWrite = 1;
      break;
    // store word
    case 43:
      // instruction writes to memory
      controls->MemWrite = 1;
      // instructions uses sign extended offset constant to calculate memory address
      controls->ALUSrc = 1;
      break;
    // load upper immediate
    case 15:
      // instruction writes to register file
      controls->RegWrite = 1;
      // ALU opcode to shift the extended_value left by 16
      controls->ALUOp = 6;
      // routes sign extended constant to the ALU
      controls->ALUSrc = 1;
      break;
    // branch if equal
    case 4:
      // instruction causes a branch if zero signal is asserted
      controls->Branch = 1;
      // ALU operands should be subtracted to test for equality with zero signal
      controls->ALUOp = 1;
      break;
    // set on less than immediate
    case 10:
      // routes the sign extended constant to the ALU
      controls->ALUSrc = 1;
      // instructs ALU to set output to 1 if operand A < operand B
      controls->ALUOp = 2;
      // instruction writes result of comparison to register file
      controls->RegWrite = 1;
      break;
    // set less than immediate unsigned
    case 11:
      // routes the sign extended constant to the ALU
      controls->ALUSrc = 1;
      // instructs ALU to do an unsigned comparison of its operands
      controls->ALUOp = 3;
      // instruction writes result of comparison to register file
      controls->RegWrite = 1;
      break;
    // jump
    case 2:
      // choose jump address for PC result
      controls->Jump = 1;
      break;
    // invalid op code / not supported
    default:
      return 1;
  }
  // valid op code
  return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
  // data1 is specified to be the value stored in register r1
  *data1 = Reg[r1];
  // data2 is specified to be the value stored in register r2
  *data2 = Reg[r2];
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{

  // sign bit is the 16th bit, so shifting 15 to the right reveals the sign
  if (offset >> 15 == 0)
    // AND with constant that has bits 0-15 set
    *extended_value = offset & 65535;
  else
    // OR with constant that has bits 31-16 set
    *extended_value = offset | 4294901760;
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
  // r-type instruction must be decoded using funct code
  if (ALUOp == 7)
  {
    switch (funct)
    {
      // add
      case 32:
        // instructs ALU to add operands
        ALUOp = 0;
        break;
      // sub
      case 34:
        // instructs ALU to subtract operands
        ALUOp = 1;
        break;
      // and
      case 36:
        // instructs ALU to AND operands
        ALUOp = 4;
        break;
      // or
      case 37:
        // instructs ALU to OR operands
        ALUOp = 5;
        break;
      // slt
      case 42:
        // instructs ALU to compare its operands
        ALUOp = 2;
        break;
      // sltu
      case 43:
        // instructs ALU to conduct an unsigned comparison of its operands
        ALUOp = 3;
        break;
      // invalid / non-supported funct code
      default:
        return 1;
    }
  }

  // set the second ALU source operand using signal
  data2 = (ALUSrc == 1)? extended_value : data2;

  // sends formatted data and ALU control signal to the ALU
  ALU(data1, data2, ALUOp, ALUresult, Zero);
  // success!
  return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
  // NoOp
  if (MemWrite == 0 && MemRead == 0)
    return 0;

  // load word instruction
  if (MemRead == 1)
  {
    // Illegal instruction
    if (ALUresult > 65536 || ALUresult % 4 != 0)
      return 1;

    // read from data memory using left shift to adjust for word spacing
    *memdata = Mem[ALUresult >> 2];
  }

  // store word instruction
  else if (MemWrite == 1)
  {
    // Illegal instruction
    if (ALUresult > 65536 || ALUresult % 4 != 0)
      return 1;

    // write data2 value to data memory
    Mem[ALUresult >> 2] = data2;
  }

  // success!!
  return 0;
}

/* Write Register */
/* 10 Points */
// r3 is an rtype
// r2 is an itype
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
  // Data is coming from memory (memdata)
  if (RegWrite == 1 && MemtoReg == 1)
  {
    // Instruction is an itype
    if (RegDst == 0)
      Reg[r2] = memdata;

    // Instruction is an rtype
    else if (RegDst == 1)
      Reg[r3] = memdata;
  }

  // Data is coming from ALUresult
  else if (RegWrite == 1 && MemtoReg == 0)
  {
    // Instruction is an itype
    if (RegDst == 0)
      Reg[r2] = ALUresult;

    // Instruction is an rtype
    else if (RegDst == 1)
      Reg[r3] = ALUresult;
  }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
  // always add 4 to PC
  *PC += 4;

  // check for branch conditions
  if (Branch == 1 && Zero == 1)
  {
    // branch address is derived from the sign extended value
    *PC += (extended_value << 2);
  }
  // check for jump
  else if (Jump == 1)
  {
    // jump address is the upper 4 bits of the PC+4 concatenated with 28 bit jump address
    *PC = (jsec << 2) | (*PC & 0xF0000000);
  }
}
