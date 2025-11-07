#include "vm/dual_issue/hardware/commit_buffer.h"
#include "vm/dual_issue/core/core.h"

namespace dual_issue
{
    
bool CommitBuffer::IsEmpty(){
    return buffer.empty();
}

bool CommitBuffer::IsFull(){
    return buffer.size()>=max_buffer_size;
}


void CommitBuffer::Push(DualIssueInstrContext& instr, DualIssueCore& vm_core){
    if(IsFull())
        throw std::logic_error("Commit Buffer is full.");

    buffer.insert(instr);

    // updating the reg_status_file_
    if(instr.reg_write){
        uint64_t wb_val = instr.mem_to_reg ? instr.mem_out : instr.alu_out;

        if(instr.reg_write_to_fpr){
            auto [valid, tag] = vm_core.reg_status_file_.QueryTableTag(instr.rd, false);
            
            // ts should be valid, idk why we checking. put an assert instead
            if(valid && tag==instr.tag){
                vm_core.reg_status_file_.UpdateTableValue(instr.rd, false, wb_val);
            }
        }
        else{
            auto [valid, tag] = vm_core.reg_status_file_.QueryTableTag(instr.rd, true);

            if(valid && tag==instr.tag){
                vm_core.reg_status_file_.UpdateTableValue(instr.rd, true, wb_val);
            }
        }
    }

    BroadCastMsgs(instr, vm_core.broadcast_bus_);

    Commit(vm_core);
}


std::pair<bool, uint64_t> CommitBuffer::Query(uint64_t tag){
    if(buffer.find(tag) == buffer.end()){
        return {false, 0};
    }
    else{
        DualIssueInstrContext instr = *buffer.find(tag);
        return {true, instr.rd};
    }
}


void CommitBuffer::BroadCastMsgs(DualIssueInstrContext& instr, CommonDataBus& data_bus){
    if(instr.reg_write){
        uint64_t broadcast_val = instr.mem_to_reg ? instr.mem_out : instr.alu_out;
        data_bus.BroadCast(instr.tag, broadcast_val);
    }
}


void CommitBuffer::Commit(DualIssueCore& vm_core){
    if(buffer.empty())
        return;
    
    DualIssueInstrContext top_instr = *buffer.begin();
    
    if(top_instr.tag == vm_core.to_commit_tag_){
        buffer.erase(top_instr);
        // TODO: WriteBack
        vm_core.to_commit_tag_++;
    }
    // to_commit_tag_ is updated if the branch was predicted incorrectly. we nuke it here
    else if(top_instr.tag < vm_core.to_commit_tag_){
        buffer.erase(top_instr);
    }
    // else wait for previous instructions to complete
}

} // namespace dual_issue
