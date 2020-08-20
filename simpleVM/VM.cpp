#include "VM.h"

HANDLE hStdin = INVALID_HANDLE_VALUE;
DWORD fdwMode, fdwOldMode;

uint16_t VM::Swap16(uint16_t num)
{
	return (num >> 8) | (num << 8);
} 
uint16_t VM::SignExtend(uint16_t num, int opNum)
{
	return ((num >> (opNum - 1)) & 1) ?
		((0xFFFF << opNum) | num) : num;
} 
void VM::UpdateFlag(uint16_t num)
{
	if (num & 0x8000)
		registerMemory[R_COND] = F_NEG;
	else if (num == 0)
		registerMemory[R_COND] = F_ZER;
	else
		registerMemory[R_COND] = F_POS;
} 
void VM::MemoryWrite(uint16_t address, uint16_t value)
{
	memory[address] = value;
} 
uint16_t VM::MemoryRead(uint16_t address)
{
	if (address == KBSR)
	{
		if (CheckKey())
		{
			memory[KBSR] = 1 << 15;
			memory[KBDR] = getchar();
		}
		else
			memory[KBSR] = 0;
	}
	return memory[address];
} 

void VM::ADD(uint16_t operation)
{
	uint16_t DR = (operation >> 9) & 0x7;
	uint16_t SR1 = (operation >> 6) & 0x7;
	if ((operation >> 5) & 0x1)
		registerMemory[DR] = registerMemory[SR1] + 
		SignExtend(operation & 0x1F, 5);
	else
		registerMemory[DR] = registerMemory[SR1] + 
		registerMemory[operation & 0x7];
	UpdateFlag(registerMemory[DR]);
}
void VM::AND(uint16_t operation)
{
	uint16_t DR = (operation >> 9) & 0x7;
	uint16_t SR1 = (operation >> 6) & 0x7;
	if ((operation >> 5) & 0x1)
		registerMemory[DR] = registerMemory[SR1] &
		SignExtend(operation & 0x1F, 5);
	else
		registerMemory[DR] = registerMemory[SR1] &
		registerMemory[operation & 0x7];
	UpdateFlag(registerMemory[DR]);
}
void VM::NOT(uint16_t operation)
{
	uint16_t DR = (operation >> 9) & 0x7;
	uint16_t SR1 = (operation >> 6) & 0x7;
	registerMemory[DR] = ~registerMemory[SR1];
	UpdateFlag(registerMemory[DR]);
}
void VM::LD(uint16_t operation)
{
	uint16_t DR = (operation >> 9) & 0x7;
	uint16_t PCOffset = SignExtend(operation & 0x1ff, 9);
	registerMemory[DR] = MemoryRead(registerMemory[R_PC] + PCOffset);
	UpdateFlag(registerMemory[DR]);
}
void VM::ST(uint16_t operation)
{
	uint16_t DR = (operation >> 9) & 0x7;
	uint16_t PCOffset = SignExtend(operation & 0x1ff, 9);
	MemoryWrite(registerMemory[R_PC] + PCOffset, registerMemory[DR]);
}
void VM::LDR(uint16_t operation)
{
	uint16_t DR = (operation >> 9) & 0x7;
	uint16_t SR1 = (operation >> 6) & 0x7;
	uint16_t PCOffset = SignExtend(operation & 0x3f, 6);
	registerMemory[DR] = MemoryRead(registerMemory[SR1] + PCOffset);
	UpdateFlag(registerMemory[DR]);
}
void VM::STR(uint16_t operation)
{
	uint16_t DR = (operation >> 9) & 0x7;
	uint16_t SR1 = (operation >> 6) & 0x7;
	uint16_t PCOffset = SignExtend(operation & 0x3f, 6);
	MemoryWrite(registerMemory[SR1] + PCOffset, registerMemory[DR]);
}
void VM::LDI(uint16_t operation)
{
	uint16_t DR = (operation >> 9) & 0x7;
	uint16_t PCOffset = SignExtend(operation & 0x1ff, 9);
	registerMemory[DR] = MemoryRead(MemoryRead(registerMemory[R_PC] + PCOffset));
	UpdateFlag(registerMemory[DR]);
}
void VM::LEA(uint16_t operation)
{
	uint16_t DR = (operation >> 9) & 0x7;
	uint16_t SR1 = SignExtend(operation & 0x1ff, 9);
	registerMemory[DR] = registerMemory[R_PC] + SR1;
	UpdateFlag(registerMemory[DR]);
}
void VM::RTI(uint16_t operation)
{
	fprintf(stderr, "Error Instruction RTI!");
	exit(-1);
}
void VM::STI(uint16_t operation)
{
	uint16_t DR = (operation >> 9) & 0x7;
	uint16_t PCOffset = SignExtend(operation & 0x1ff, 9);
	MemoryWrite(MemoryRead(registerMemory[R_PC] + PCOffset), registerMemory[DR]);
}
void VM::BR(uint16_t operation)
{
	uint16_t PCOffset = SignExtend(operation & 0x1ff, 9);
	uint16_t conditionFlag = (operation >> 9) & 0x7;
	if (conditionFlag & registerMemory[R_COND])
		registerMemory[R_PC] +=PCOffset;
}
void VM::JMP(uint16_t operation)
{
	uint16_t DR = (operation >> 6) & 0x7;
	registerMemory[R_PC] = registerMemory[DR];
}
void VM::JSR(uint16_t operation)
{
	uint16_t DR = (operation >> 6) & 0x7;
	uint16_t longOffset = SignExtend(operation & 0x7ff, 11);
	uint16_t longFlag = (operation >> 11) & 1;
	registerMemory[R7] = registerMemory[R_PC];
	if (longFlag)
		registerMemory[R_PC] += longOffset;
	else
		registerMemory[R_PC] = registerMemory[DR];
}
void VM::RES(uint16_t operation)
{
	fprintf(stderr, "Error Instruction RES!");
	exit(-1);
}

void VM::TRAP(uint16_t operation)
{
	(this->*TrapFunctionTable[(operation & 0xff) - 0x20])();
}

void VM::GETC() 
{
	registerMemory[R0] = (uint16_t)getchar();
}
void VM::TOUT() 
{
	putchar((char)registerMemory[R0]);
	fflush(stdout);
}
void VM::PUTS() 
{
	uint16_t* ptr = memory + registerMemory[R0];
	while (*ptr)
	{
		putchar((char)(*ptr));
		++ptr;
	}
	fflush(stdout);
}
void VM::TIN()
{
	printf("Enter a character: ");
	char ch = getchar();
	putchar(ch);
	registerMemory[R0] = (uint16_t)ch;
	fflush(stdout);
}
void VM::PUTSP()
{
	uint16_t* ptr = memory + registerMemory[R0];
	while (*ptr)
	{
		char ch = (*ptr) & 0xff;
		putchar(ch);
		ch = (*ptr) >> 8;
		if (ch)
			putchar(ch);
		++ptr;
	}
	fflush(stdout);
}
void VM::HALT()
{
	printf("Halt\n");
	isRunning = 0;
	fflush(stdout);
}

void VM::Operate(uint16_t operation)
{
	(this->*operationTable[operation >> 12])(operation);
} 

VM::VM()
{
	operationTable[0] = &VM::BR;
	operationTable[1] = &VM::ADD;
	operationTable[2] = &VM::LD;
	operationTable[3] = &VM::ST;
	operationTable[4] = &VM::JSR;
	operationTable[5] = &VM::AND;
	operationTable[6] = &VM::LDR;
	operationTable[7] = &VM::STR;
	operationTable[8] = &VM::RTI;
	operationTable[9] = &VM::NOT;
	operationTable[10] = &VM::LDI;
	operationTable[11] = &VM::STI;
	operationTable[12] = &VM::JMP;
	operationTable[13] = &VM::RES;
	operationTable[14] = &VM::LEA;
	operationTable[15] = &VM::TRAP;

	TrapFunctionTable[0] = &VM::GETC;
	TrapFunctionTable[1] = &VM::TOUT;
	TrapFunctionTable[2] = &VM::PUTS;
	TrapFunctionTable[3] = &VM::TIN;
	TrapFunctionTable[4] = &VM::PUTSP;
	TrapFunctionTable[5] = &VM::HALT;
	memset(memory, 0, sizeof(memory));
	memset(registerMemory, 0, sizeof(registerMemory));
} 
VM::~VM(){} 

bool VM::LoadCode(const char* filePath)
{
	LoadCodeStatus = false;
	FILE* filePtr = fopen(filePath, "rb");
	if (!filePtr)
		return printf("Can not open the file\n"), false;
	uint16_t origin, * originPtr;
	fread(&origin, sizeof(uint16_t), 1, filePtr);
	origin = Swap16(origin);
	originPtr = memory + origin;
	size_t maxReadByte = fread(originPtr, sizeof(uint16_t), 
		UINT16_MAX - origin, filePtr);
	while (maxReadByte--)
	{
		*originPtr = Swap16(*originPtr);
		++originPtr;
	}
	fclose(filePtr);
	printf("Load Code OK!\n");
	LoadCodeStatus = true;
	return true;
}
bool VM::isLoadedCode()
{
	return LoadCodeStatus;
}
bool VM::RunCode()
{
	if (!LoadCodeStatus)
		return printf("Not Code to Run\n"), false;
	signal(SIGINT, HandleInterrupt);
	DisableInputBuffering();
	registerMemory[R_PC] = 0x3000;
	isRunning = true;
	while (isRunning)
		Operate(MemoryRead(registerMemory[R_PC]++));
	RestoreInputBuffering();
	isRunning = false;
	return printf("Finished\n"), true;
}

void DisableInputBuffering()
{
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(hStdin, &fdwOldMode); 
	fdwMode = fdwOldMode
		^ ENABLE_ECHO_INPUT  
		^ ENABLE_LINE_INPUT; 
	SetConsoleMode(hStdin, fdwMode); 
	FlushConsoleInputBuffer(hStdin); 
}
void RestoreInputBuffering()
{
	SetConsoleMode(hStdin, fdwOldMode);
}
void HandleInterrupt(int signal)
{
	RestoreInputBuffering();
	printf("\n");
	exit(-2);
}

uint16_t VM::CheckKey()
{
	return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}
