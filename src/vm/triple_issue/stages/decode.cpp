#include "vm/triple_issue/stages/stages.h"
#include "common/instructions.h"

namespace triple_issue
{

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;

void TripleIssueStages::Decode(TripleIssueCore& vm_core){
	TripleIssueInstrContext& instr1 = vm_core.pipeline_reg_instrs_.if_id_1;
	if(!instr1.illegal){
        vm_core.decode_unit_.DecodeInstruction(instr1);

        if (instr1.opcode == get_instr_encoding(Instruction::kecall).opcode && 
            instr1.funct3 == get_instr_encoding(Instruction::kecall).funct3) {
            HandleSyscall(vm_core);
            return;
        }
    }

    TripleIssueInstrContext& instr2 = vm_core.pipeline_reg_instrs_.if_id_2;
    if(!instr2.illegal){
        vm_core.decode_unit_.DecodeInstruction(instr2);

        if (instr2.opcode == get_instr_encoding(Instruction::kecall).opcode && 
            instr2.funct3 == get_instr_encoding(Instruction::kecall).funct3) {
            HandleSyscall(vm_core);
            return;
        }
    }

    TripleIssueInstrContext& instr3 = vm_core.pipeline_reg_instrs_.if_id_3;
    if(!instr3.illegal){
        vm_core.decode_unit_.DecodeInstruction(instr3);

        if (instr3.opcode == get_instr_encoding(Instruction::kecall).opcode && 
            instr3.funct3 == get_instr_encoding(Instruction::kecall).funct3) {
            HandleSyscall(vm_core);
            return;
        }
    }
}

void TripleIssueStages::HandleSyscall(TripleIssueCore& vm_core){
    globals::vm_cout_file << "SYSCALLS ARE CURRENTLY UNDER DEVELOPMENT." << std::endl;
    return;
}

} // namespace triple_issue
