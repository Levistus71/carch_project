#include "vm/triple_issue/hardware/decode_unit.h"
#include "vm/dual_issue/core/instruction_context/instruction_context.h"
#include "vm/triple_issue/core/instruction_context/instruction_context.h"
#include "common/instructions.h"

namespace triple_issue
{
    
void TripleIssueDecodeUnit::DecodeInstruction(TripleIssueInstrContext& instr_context, register_file::RegisterFile& rf){
    dual_issue::DualIssueInstrContext& base_part = static_cast<dual_issue::DualIssueInstrContext&>(instr_context);
    dual_issue::DualIssueDecodeUnit::DecodeInstruction(base_part, rf);

    SetAluQue(instr_context);
}


void TripleIssueDecodeUnit::SetAluQue(TripleIssueInstrContext& instr_context){
    if(instr_context.mem_read || instr_context.mem_write){
        return;
    }

    if (instruction_set::uses_falu(instr_context.instruction)) {
        instr_context.into_falu = true;
    }
    else{
        instr_context.into_falu = false;
    }
}


} // namespace triple_issue
