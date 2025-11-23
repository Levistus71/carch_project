#include "vm/dual_issue/hardware/reservation_station.h"
#include "vm/dual_issue/core/core.h"
#include "vm/triple_issue/core/core.h"

namespace dual_issue
{

ReservationStation::ReservationStation() : que_(max_size_){
    Reset();
}

size_t ReservationStation::EmptySlots(){
    size_t empty_slots = max_size_-que_.size();

    for(size_t i=0;i<que_.size();i++){
        if(que_[i].illegal){
            empty_slots++;
        }
    }
    return empty_slots;
}

void ReservationStation::ListenToBroadCast(CommonDataBus& data_bus){

    for(auto& broadcast_msg : data_bus.broadcast_msgs){

        for(auto& instr : que_){
            if(instr.ready_to_exec)
                continue;

            if(instr.wait_for_rs1 && instr.rs1_tag==broadcast_msg.rob_idx && instr.rs1_epoch==broadcast_msg.epoch){
                instr.wait_for_rs1 = false;

                if(!broadcast_msg.clear_dependency){
                    if(instr.rs1_from_fprf)
                        instr.frs1_value = broadcast_msg.value;
                    else
                        instr.rs1_value = broadcast_msg.value;
                }
            }
            if(instr.wait_for_rs2 && instr.rs2_tag==broadcast_msg.rob_idx && instr.rs2_epoch==broadcast_msg.epoch){
                instr.wait_for_rs2 = false;

                if(!broadcast_msg.clear_dependency){
                    if(instr.rs2_from_fprf)
                        instr.frs2_value = broadcast_msg.value;
                    else
                        instr.rs2_value = broadcast_msg.value;
                }
            }
            if(instr.wait_for_rs3 && instr.rs3_tag==broadcast_msg.rob_idx && instr.rs3_epoch==broadcast_msg.epoch){
                instr.wait_for_rs3 = false;

                if(!broadcast_msg.clear_dependency)
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

    auto* upcasted_triple = dynamic_cast<triple_issue::TripleIssueCore*>(&vm_core);

    if(instr.uses_rs1){
        auto [dependency, dependent_idx] = vm_core.reg_status_file_.QueryTableRobIdx(instr.rs1, !instr.rs1_from_fprf);

        if(!dependency){
            instr.wait_for_rs1 = false;
        }
        else{
            auto [value_ready, value, epoch] = (upcasted_triple) ? upcasted_triple->commit_buffer_.QueryVal(dependent_idx) : vm_core.commit_buffer_.QueryVal(dependent_idx);

            if(value_ready){
                instr.wait_for_rs1 = false;
                
                if(instr.rs1_from_fprf)
                    instr.frs1_value = value;
                else
                    instr.rs1_value = value;
            }
            else{
                instr.rs1_tag = dependent_idx;
                instr.rs1_epoch = epoch;
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
            auto [value_ready, value, epoch] = (upcasted_triple) ? upcasted_triple->commit_buffer_.QueryVal(dependent_idx) : vm_core.commit_buffer_.QueryVal(dependent_idx);

            if(value_ready){
                instr.wait_for_rs2 = false;

                if(instr.rs2_from_fprf)
                    instr.frs2_value = value;
                else
                    instr.rs2_value = value;
            }
            else{
                instr.rs2_tag = dependent_idx;
                instr.rs2_epoch = epoch;
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
            auto [value_ready, val, epoch] = (upcasted_triple) ? upcasted_triple->commit_buffer_.QueryVal(dependent_idx) : vm_core.commit_buffer_.QueryVal(dependent_idx);

            if(value_ready){
                instr.wait_for_rs3 = false;
                instr.frs3_value = val;
            }
            else{
                instr.rs3_tag = dependent_idx;
                instr.rs3_epoch = epoch;
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

    // pushing into the first illegal entry
    for(size_t i=0;i<que_.size();i++){
        if(que_[i].illegal){
            que_[i] = instr;
            break;
        }
    }
}


DualIssueInstrContext ReservationStation::GetReadyInstr(){
    if(que_.empty()){
        DualIssueInstrContext t;
        t.illegal = true;
        return t;
    }

    for(int i=0;i<static_cast<int>(que_.size());i++){
        if(que_[i].ready_to_exec && !que_[i].illegal){
            DualIssueInstrContext instr = que_[i];

            std::deque<DualIssueInstrContext> new_que_;
            for(int j=0;j<static_cast<int>(que_.size());j++){
                if(j==i)
                    continue;
                new_que_.push_back(que_[j]);
            }

            DualIssueInstrContext t;
            t.illegal = true;
            t.ready_to_exec = false;
            new_que_.push_back(t);

            que_ = new_que_;
            return instr;
        }
    }

    DualIssueInstrContext t;
    t.illegal = true;
    return t;
}

DualIssueInstrContext ReservationStation::GetInorderInstr(){
    if(que_.empty()){
        DualIssueInstrContext t;
        t.illegal = true;
        return t;
    }

    if(que_[0].ready_to_exec && !que_[0].illegal){
        DualIssueInstrContext instr = que_[0];
        que_.pop_front();

        DualIssueInstrContext t;
        t.illegal = true;
        t.ready_to_exec = false;
        que_.push_back(t);

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