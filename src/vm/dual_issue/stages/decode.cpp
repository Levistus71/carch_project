#include "vm/dual_issue/stages/stages.h"
#include "common/instructions.h"

namespace dual_issue
{

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;

void DualIssueStages::Decode(DualIssueCore& vm_core){
	DualIssueInstrContext& instr1 = vm_core.pipeline_reg_instrs_.if_id_1;
	if(!instr1.illegal){
        vm_core.decode_unit_.DecodeInstruction(instr1);

        if (instr1.opcode == get_instr_encoding(Instruction::kecall).opcode && 
            instr1.funct3 == get_instr_encoding(Instruction::kecall).funct3) {
            HandleSyscall(vm_core);
            return;
        }
    }

    DualIssueInstrContext& instr2 = vm_core.pipeline_reg_instrs_.if_id_2;
    if(!instr2.illegal){
        vm_core.decode_unit_.DecodeInstruction(instr2);

        if (instr2.opcode == get_instr_encoding(Instruction::kecall).opcode && 
            instr2.funct3 == get_instr_encoding(Instruction::kecall).funct3) {
            HandleSyscall(vm_core);
            return;
        }
    }
}

void DualIssueStages::HandleSyscall(DualIssueCore& vm_core){
    globals::vm_cout_file << "SYSCALLS ARE CURRENTLY UNDER DEVELOPMENT." << std::endl;
    return;
}

    
} // namespace dual_issue
