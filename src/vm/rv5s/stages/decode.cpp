#include "vm/rv5s/rv5s_vm.h"

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;

void RV5SVM::Decode() {
	decode_unit.DecodeInstruction(*(this->id_instruction), registers_);

	if (this->id_instruction->opcode == get_instr_encoding(Instruction::kecall).opcode && 
		this->id_instruction->funct3 == get_instr_encoding(Instruction::kecall).funct3) {
		// TODO: flush all instructions, start fresh at pc = pc+4
		HandleSyscall();
		return;
	}
}


void RV5SVM::DebugDecode(){
	Decode();
}