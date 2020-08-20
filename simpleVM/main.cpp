#include "VM.h"

int main()
{
	VM* vm = new VM;
	vm->LoadCode("D:\\2048.obj");
	vm->RunCode();
	delete vm;
	return 0;
}