#include "vm/rv5s/single_cycle/stages/stages.h"
#include "common/instructions.h"

namespace rv5s{

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;

void SingleCycleStages::Decode(SingleCycleCore& vm_core){
	vm_core.decode_unit_.DecodeInstruction(vm_core.instr, vm_core.register_file_);

	if (vm_core.instr.opcode == get_instr_encoding(Instruction::kecall).opcode && 
		vm_core.instr.funct3 == get_instr_encoding(Instruction::kecall).funct3) {
		HandleSyscall(vm_core);
		return;
	}
}

void SingleCycleStages::HandleSyscall(SingleCycleCore& vm_core){
    globals::vm_cout_file << "SYSCALLS ARE CURRENTLY UNDER DEVELOPMENT." << std::endl;
    return;
}

} // namespace rv5s