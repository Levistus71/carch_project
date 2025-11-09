#include "vm/dual_issue/hardware/reservation_station.h"
#include "vm/dual_issue/core/core.h"

namespace dual_issue
{

ReservationStation::ReservationStation() : que_(max_size_){
    Reset();
}

size_t ReservationStation::EmptySlots(){
    return max_size_-que_.size();
}

void ReservationStation::ListenToBroadCast(CommonDataBus& data_bus){

    for(auto& broadcast_msg : data_bus.broadcast_msgs){

        for(auto& instr : que_){
            if(instr.ready_to_exec)
                continue;

            if(instr.wait_for_rs1 && instr.rs1_tag==broadcast_msg.rob_idx){
                instr.wait_for_rs1 = false;

                if(instr.rs1_from_fprf)
                    instr.frs1_value = broadcast_msg.value;
                else
                    instr.rs1_value = broadcast_msg.value;
            }
            if(instr.wait_for_rs2 && instr.rs2_tag==broadcast_msg.rob_idx){
                instr.wait_for_rs2 = false;

                if(instr.rs2_from_fprf)
                    instr.frs2_value = broadcast_msg.value;
                else
                    instr.rs2_value = broadcast_msg.value;
            }
            if(instr.wait_for_rs3 && instr.rs3_tag==broadcast_msg.rob_idx){
                instr.wait_for_rs3 = false;
                instr.frs3_value = broadcast_msg.value;
            }

            instr.ready_to_exec = !instr.wait_for_rs1 && !instr.wait_for_rs2 && !instr.wait_for_rs3;
        }
    }
}


void ReservationStation::Push(DualIssueInstrContext instr, DualIssueCore& vm_core){
    instr.wait_for_rs1 = true;
    instr.wait_for_rs2 = true;
    instr.wait_for_rs3 = true;

    if(instr.uses_rs1){
        auto [dependency, dependent_idx] = vm_core.reg_status_file_.QueryTableRobIdx(instr.rs1, !instr.rs1_from_fprf);

        if(!dependency){
            instr.wait_for_rs1 = false;
        }
        else{
            auto [value_ready, value] = vm_core.commit_buffer_.QueryVal(dependent_idx);

            if(value_ready){
                instr.wait_for_rs1 = false;
                
                if(instr.rs1_from_fprf)
                    instr.frs1_value = value;
                else
                    instr.rs1_value = value;
            }
        }
    }
    else{
        instr.wait_for_rs1 = false;
    }

    if(instr.uses_rs2){
        auto [dependency, dependent_idx] = vm_core.reg_status_file_.QueryTableRobIdx(instr.rs2, !instr.rs2_from_fprf);

        if(!dependency){
            instr.wait_for_rs2 = false;
        }
        else{
            auto [value_ready, value] = vm_core.commit_buffer_.QueryVal(dependent_idx);

            if(value_ready){
                instr.wait_for_rs2 = false;

                if(instr.rs2_from_fprf)
                    instr.frs2_value = value;
                else
                    instr.rs2_value = value;
            }
        }
    }
    else{
        instr.wait_for_rs2 = false;
    }

    if(instr.uses_rs3){
        auto [dependency, dependent_idx] = vm_core.reg_status_file_.QueryTableRobIdx(instr.frs3, false);

        if(!dependency){
            instr.wait_for_rs3 = false;
        }
        else{
            auto [value_ready, val] = vm_core.commit_buffer_.QueryVal(dependent_idx);

            if(value_ready){
                instr.wait_for_rs3 = false;
                instr.frs3_value = val;
            }
        }
    }
    else{
        instr.wait_for_rs3 = false;
    }

    instr.ready_to_exec = !instr.wait_for_rs1 && !instr.wait_for_rs2 && !instr.wait_for_rs3;

    // updating the reg_status_file
    if(instr.reg_write){
        vm_core.reg_status_file_.UpdateTableRobIdx(instr.rd, !instr.reg_write_to_fpr, instr.rob_idx);
    }

    que_.push_back(instr);
}


DualIssueInstrContext ReservationStation::GetReadyInstr(){
    if(que_.empty()){
        DualIssueInstrContext t;
        t.illegal = true;
        return t;
    }

    if(que_.front().ready_to_exec || que_.front().illegal){
        DualIssueInstrContext instr = que_.front();
        que_.pop_front();
        return instr;
    }

    DualIssueInstrContext t;
    t.illegal = true;
    return t;
}


void ReservationStation::Reset(){
    for(auto& p : que_){
        p.illegal = true;
    }
}

std::vector<std::unique_ptr<const InstrContext>> ReservationStation::GetQue(){
    std::vector<std::unique_ptr<const InstrContext>> ret;

    for(auto& p : que_){
        ret.push_back(std::make_unique<const DualIssueInstrContext>(p));
    }

    return ret;
}


} // namespace dual_issue