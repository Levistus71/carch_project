#include "vm/rv5s/stages/stages.h"
#include "common/instructions.h"

namespace rv5s{

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;

void Stages::Decode(Core& vm_core){
	InstrContext& id_instruction = vm_core.GetIdInstruction();
	if(id_instruction.nopped)
		return;
	
	vm_core.decode_unit_.DecodeInstruction(id_instruction, vm_core.register_file_);

	if (id_instruction.opcode == get_instr_encoding(Instruction::kecall).opcode && 
		id_instruction.funct3 == get_instr_encoding(Instruction::kecall).funct3) {
		HandleSyscall(vm_core);
		return;
	}
}

void Stages::HandleSyscall(Core& vm_core){
    globals::vm_cout_file << "SYSCALLS ARE CURRENTLY UNDER DEVELOPMENT." << std::endl;
    return;
}

} // namespace rv5s