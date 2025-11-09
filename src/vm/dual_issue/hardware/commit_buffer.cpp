#include "vm/dual_issue/hardware/commit_buffer.h"
#include "vm/dual_issue/core/core.h"
#include "vm/dual_issue/stages/stages.h"


namespace dual_issue{

ROBBuffer::ROBBuffer() : buffer(max_size){
    Reset();
}

size_t ROBBuffer::EmptySlots(){
    if(tail > head){
        return max_size - (tail-head);
    }
    else{
        return max_size - head + tail;
    }
}

bool ROBBuffer::HeadReady(){
    return buffer[head].ready_to_commit;
}

void ROBBuffer::Push(DualIssueInstrContext instr){
    buffer[instr.rob_idx].ready_to_commit = true;
    buffer[instr.rob_idx].instr = instr;
}

DualIssueInstrContext ROBBuffer::Top(){
    return buffer[head].instr;
}

void ROBBuffer::Pop(){
    buffer[head].ready_to_commit = false;
    buffer[head].instr.illegal = true;
    head = (head+1) % max_size;
}

size_t ROBBuffer::Reserve(){
    size_t ret = tail;
    tail = (tail+1) % max_size;
    return ret;
}

std::pair<bool, uint64_t> ROBBuffer::QueryVal(uint64_t idx){
    uint64_t write_val = buffer[idx].instr.mem_to_reg 
        ? buffer[idx].instr.mem_out 
        : buffer[idx].instr.alu_out;

    return {buffer[idx].ready_to_commit, write_val};
}

void ROBBuffer::Reset(){
    tail = 0;
    head = 0;

    for(size_t i=0;i<max_size;i++){
        buffer[i].ready_to_commit = false;
        buffer[i].instr.illegal = true;
    }
}

std::vector<std::unique_ptr<const InstrContext>> ROBBuffer::GetInstrs(){
    std::vector<std::unique_ptr<const InstrContext>> ret;
    for(auto& p : buffer){
        ret.push_back(std::make_unique<const DualIssueInstrContext>(p.instr));
    }

    return ret;
}
std::vector<bool> ROBBuffer::GetStatus(){
    std::vector<bool> ret;
    for(auto& p : buffer){
        ret.push_back(p.ready_to_commit);
    }

    return ret;
}
std::pair<size_t, size_t> ROBBuffer::GetHeadTail(){
    return {head, tail};
}

} // namespace dual_issue


namespace dual_issue
{

size_t ReorderBuffer::EmptySlots(){
    return buffer.EmptySlots();
}


void ReorderBuffer::Pull(DualIssueCore& vm_core){
    DualIssueInstrContext instr_alu_out = vm_core.pipeline_reg_instrs_.alu_commit;
    DualIssueInstrContext instr_mem_out = vm_core.pipeline_reg_instrs_.lsu_commit;

    if(!instr_alu_out.illegal){
        Push(instr_alu_out, vm_core);
    }
    if(!instr_mem_out.illegal){
        Push(instr_mem_out, vm_core);
    }
}

void ReorderBuffer::Push(DualIssueInstrContext& instr, DualIssueCore& vm_core){
    buffer.Push(instr);

    BroadCastMsgs(instr, vm_core.broadcast_bus_);
}


void ReorderBuffer::BroadCastMsgs(DualIssueInstrContext& instr, CommonDataBus& data_bus){
    if(instr.reg_write){
        uint64_t broadcast_val = instr.mem_to_reg ? instr.mem_out : instr.alu_out;
        data_bus.BroadCast(instr.rob_idx, broadcast_val);
    }
}


size_t ReorderBuffer::Reserve(){
    return buffer.Reserve();
}

std::pair<bool, uint64_t> ReorderBuffer::QueryVal(uint64_t idx){
    return buffer.QueryVal(idx);
}


void ReorderBuffer::Commit(DualIssueCore& vm_core){
    // The ROB can commit 2 instructions in 1 cycle. doing it in a for loop instead of making 2 write ports
    for(int i=0;i<2;i++){

        if(!buffer.HeadReady())
            return;
        
        DualIssueInstrContext top_instr = buffer.Top();
        
        if(top_instr.tag == vm_core.to_commit_tag_){
            buffer.Pop();
            DualIssueStages::WriteBack(top_instr, vm_core);
            vm_core.reg_status_file_.EndDependency(top_instr.rd, top_instr.rob_idx, !top_instr.reg_write_to_fpr);
            vm_core.to_commit_tag_++;
        }
        // to_commit_tag_ is updated if the branch was predicted incorrectly. we nuke it here
        else if(top_instr.tag < vm_core.to_commit_tag_){
            buffer.Pop();
        }
        // else wait for previous instructions to complete
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

} // namespace dual_issue
