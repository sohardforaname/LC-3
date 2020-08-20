#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <csignal>
#include <Windows.h>
#include <conio.h>

void DisableInputBuffering();
void RestoreInputBuffering();
void HandleInterrupt(int signal);

enum
{
	R0,
	R1,
	R2,
	R3,
	R4,
	R5,
	R6,
	R7,
	R_COND, 
	R_PC
};

enum
{
    F_POS = 1 << 0,
    F_ZER = 1 << 1,
    F_NEG = 1 << 2
};

enum
{
    OP_BR,
    OP_ADD,  
    OP_LD,    
    OP_ST,    
    OP_JSR,    
    OP_AND,   
    OP_LDR,   
    OP_STR,  
    OP_RTI,  
    OP_NOT,  
    OP_LDI,   
    OP_STI,   
    OP_JMP,    
    OP_RES,    
    OP_LEA,   
    OP_TRAP    
};

enum
{
    KBSR = 0xFE00,
    KBDR = 0xFE02 
};

enum
{
    TRAP_GETC = 0x20, 
    TRAP_OUT = 0x21,
    TRAP_PUTS = 0x22,
    TRAP_IN = 0x23,
    TRAP_PUTSP = 0x24,
    TRAP_HALT = 0x25
};

class VM
{
private:
    uint16_t memory[UINT16_MAX];
    uint16_t registerMemory[R_PC + 1];
    void (VM::* operationTable[16])(uint16_t operation);
    void (VM::* TrapFunctionTable[16])();
    bool LoadCodeStatus;

private:
    uint16_t Swap16(uint16_t num);
    uint16_t SignExtend(uint16_t num, int opNum);
    void UpdateFlag(uint16_t num);
    void MemoryWrite(uint16_t address, uint16_t value);
    uint16_t MemoryRead(uint16_t address);
    bool isRunning;

private:
    void ADD(uint16_t operation);
    void AND(uint16_t operation);
    void NOT(uint16_t operation);
    void LD(uint16_t operation);
    void ST(uint16_t operation);
    void LDR(uint16_t operation);
    void LDI(uint16_t operation);
    void LEA(uint16_t operation);
    void STR(uint16_t operation);
    void RTI(uint16_t operation);
    void STI(uint16_t operation);
    void BR(uint16_t operation);
    void JMP(uint16_t operation);
    void JSR(uint16_t operation);
    void RES(uint16_t operation);
    void TRAP(uint16_t operation);

    void GETC();
    void TOUT();
    void PUTS();
    void TIN();
    void PUTSP();
    void HALT();

private:
    void Operate(uint16_t operation);

    uint16_t CheckKey();

public:
    VM();
    ~VM();
    bool LoadCode(const char* filePath);
    bool isLoadedCode();
    bool RunCode();
};
