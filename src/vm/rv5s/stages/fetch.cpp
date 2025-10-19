#include "vm/rv5s/rv5s_vm.h"

void RV5SVM::Fetch() {
	InstrContext& if_instruction = GetIfInstruction();
  	if_instruction.instruction = this->memory_controller_.ReadWord(program_counter_);
  	UpdateProgramCounter(4);
}

void RV5SVM::DebugFetch() {
	Fetch();
}