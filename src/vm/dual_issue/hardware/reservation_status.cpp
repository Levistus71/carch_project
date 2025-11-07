#include "vm/dual_issue/hardware/reservation_station.h"

namespace dual_issue
{

bool ReservationStation::IsEmpty(){
    return que_.empty();
}

bool ReservationStation::IsFull(){
    return que_.size()>=max_size_;
}

void ReservationStation::ListenToBroadCast(CommonDataBus& data_bus){

    for(auto& broadcast_msg : data_bus.broadcast_msgs){

        for(auto& instr : que_){
            if(instr.ready_to_exec)
                continue;

            if(instr.wait_for_rs1 && instr.rs1_tag==broadcast_msg.tag){
                instr.wait_for_rs1 = false;

                if(instr.rs1_from_fprf)
                    instr.frs1_value = broadcast_msg.value;
                else
                    instr.rs1_value = broadcast_msg.value;
            }
            if(instr.wait_for_rs2 && instr.rs2_tag==broadcast_msg.tag){
                instr.wait_for_rs2 = false;

                if(instr.rs2_from_fprf)
                    instr.frs2_value = broadcast_msg.value;
                else
                    instr.rs2_value = broadcast_msg.value;
            }
            if(instr.wait_for_rs3 && instr.rs3_tag==broadcast_msg.tag){
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
        auto [dependency, dependent_tag] = vm_core.reg_status_file_.QueryTableTag(instr.rs1, !instr.rs1_from_fprf);

        if(!dependency){
            instr.wait_for_rs1 = false;
        }
        else{
            auto [value_ready, value] = vm_core.reg_status_file_.QueryTableValue(instr.rs1, !instr.rs1_from_fprf);

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
        auto [dependency, dependent_tag] = vm_core.reg_status_file_.QueryTableTag(instr.rs2, !instr.rs2_from_fprf);

        if(!dependency){
            instr.wait_for_rs2 = false;
        }
        else{
            auto [value_ready, value] = vm_core.reg_status_file_.QueryTableValue(instr.rs2, !instr.rs2_from_fprf);

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
        auto [dependency, dependent_tag] = vm_core.reg_status_file_.QueryTableTag(instr.frs3, false);

        if(!dependency){
            instr.wait_for_rs3 = false;
        }
        else{
            auto [value_ready, val] = vm_core.reg_status_file_.QueryTableValue(instr.frs3, false);

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
}



} // namespace dual_issue