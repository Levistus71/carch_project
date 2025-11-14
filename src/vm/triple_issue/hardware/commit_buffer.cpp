#include "vm/triple_issue/hardware/commit_buffer.h"
#include "vm/triple_issue/core/core.h"
#include "vm/triple_issue/stages/stages.h"


namespace triple_issue
{

size_t ReorderBuffer::EmptySlots(){
    return buffer.EmptySlots();
}

void ReorderBuffer::Pull(TripleIssueCore& vm_core){
    dual_issue::DualIssueInstrContext instr_alu_out = vm_core.pipeline_reg_instrs_.alu_commit;
    dual_issue::DualIssueInstrContext instr_falu_out = vm_core.pipeline_reg_instrs_.falu_commit;
    dual_issue::DualIssueInstrContext instr_mem_out = vm_core.pipeline_reg_instrs_.lsu_commit;

    if(!instr_alu_out.illegal){
        Push(instr_alu_out, vm_core);
    }
    if(!instr_falu_out.illegal){
        Push(instr_falu_out, vm_core);
    }
    if(!instr_mem_out.illegal){
        Push(instr_mem_out, vm_core);
    }
}

void ReorderBuffer::Push(dual_issue::DualIssueInstrContext& instr, TripleIssueCore& vm_core){
    if(buffer.Push(instr)){
        BroadCastMsgs(instr, vm_core.broadcast_bus_, false);
    }
    else{
        BroadCastMsgs(instr, vm_core.broadcast_bus_, true);
        vm_core.reg_status_file_.EndDependency(instr.rd, instr.rob_idx, !instr.reg_write_to_fpr);
    }
}


void ReorderBuffer::BroadCastMsgs(dual_issue::DualIssueInstrContext& instr, dual_issue::CommonDataBus& data_bus, bool clear_dependency){
    if(instr.reg_write){
        uint64_t broadcast_val = instr.mem_to_reg ? instr.mem_out : instr.alu_out;
        data_bus.BroadCast(instr.rob_idx, broadcast_val, clear_dependency, instr.epoch);
    }
}


std::pair<size_t, size_t> ReorderBuffer::Reserve(){
    return buffer.Reserve();
}

std::tuple<bool, uint64_t, uint64_t> ReorderBuffer::QueryVal(uint64_t idx){
    return buffer.QueryVal(idx);
}


void ReorderBuffer::Commit(TripleIssueCore& vm_core){
    // The ROB can commit 3 instructions in 1 cycle. doing it in a for loop instead of making 3 write ports
    for(int i=0;i<3;i++){

        if(!buffer.HeadReady())
            return;
        
        dual_issue::DualIssueInstrContext top_instr = buffer.Top();
        
        buffer.Pop();
        TripleIssueStages::WriteBack(top_instr, vm_core);
        vm_core.reg_status_file_.EndDependency(top_instr.rd, top_instr.rob_idx, !top_instr.reg_write_to_fpr);
    }
}


void ReorderBuffer::Reset(){
    buffer.Reset();
}

std::vector<std::unique_ptr<const InstrContext>> ReorderBuffer::GetInstrs(){
    return buffer.GetInstrs();    
}
std::vector<bool> ReorderBuffer::GetStatus(){
    return buffer.GetStatus();
}

std::pair<size_t, size_t> ReorderBuffer::GetHeadTail(){
    return buffer.GetHeadTail();
}


void ReorderBuffer::ResetTailTillIdx(size_t till_head, TripleIssueCore& vm_core){
    buffer.ResetTailTillIdx(till_head, vm_core);
}

} // namespace dual_issue
