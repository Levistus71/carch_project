#include "vm/triple_issue/stages/stages.h"
#include "vm/triple_issue/hardware/commit_buffer.h"


namespace triple_issue
{

bool issue_single(TripleIssueCore& vm_core, TripleIssueInstrContext& instr){
    if(instr.illegal)
        return true;

    int alu_empty_slots = vm_core.alu_que_.EmptySlots();
    int falu_empty_slots = vm_core.falu_que_.EmptySlots();
    int lsu_empty_slots = vm_core.lsu_que_.EmptySlots();
    int commit_buffer_empty_slots = vm_core.commit_buffer_.EmptySlots();

    if(instr.mem_read || instr.mem_write){
        if(lsu_empty_slots>0 && commit_buffer_empty_slots>0){

            auto[idx, epoch] = vm_core.commit_buffer_.Reserve();
            instr.rob_idx = idx;
            instr.epoch = epoch;

            vm_core.lsu_que_.Push(instr, vm_core);

            return true;
        }
    }
    else{
        if(!instr.into_falu){
            if(alu_empty_slots>0 && commit_buffer_empty_slots>0){

                auto[idx, epoch] = vm_core.commit_buffer_.Reserve();
                instr.rob_idx = idx;
                instr.epoch = epoch;

                vm_core.alu_que_.Push(instr, vm_core);

                return true;
            }
        }
        else{
            if(falu_empty_slots>0 && commit_buffer_empty_slots>0){

                auto[idx, epoch] = vm_core.commit_buffer_.Reserve();
                instr.rob_idx = idx;
                instr.epoch = epoch;

                vm_core.falu_que_.Push(instr, vm_core);

                return true;
            }
        }
    }

    return false;
}

    
int TripleIssueStages::Issue(TripleIssueCore& vm_core){
    TripleIssueInstrContext instr1 = vm_core.pipeline_reg_instrs_.id_issue_1;
    TripleIssueInstrContext instr2 = vm_core.pipeline_reg_instrs_.id_issue_2;
    TripleIssueInstrContext instr3 = vm_core.pipeline_reg_instrs_.id_issue_3;

    // std::cout << "alu slots before pushing : " << vm_core.alu_que_.EmptySlots() << std::endl;
    bool issued_1 = issue_single(vm_core, instr1);
    bool issued_2 = issue_single(vm_core, instr2);
    bool issued_3 = issue_single(vm_core, instr3);
    
    int num_issued = issued_1 + issued_2 + issued_3;
    // std::cout << "pushed : " << num_issued << "instrs" << std::endl;
    // std::cout << "alu slots : " << vm_core.alu_que_.EmptySlots() << std::endl << std::endl;

    if(num_issued==3)
        return 3;

    if(num_issued==2){
        if(!issued_1)
            return 2;
        else if(!issued_2){
            vm_core.pipeline_reg_instrs_.id_issue_1 = instr2;
            return 2;
        }
        else{
            vm_core.pipeline_reg_instrs_.id_issue_1 = instr3;
            return 2;
        } 
    }

    if(num_issued==1){
        // 1 was issued
            // 1 -> 2, 2 -> 3
        // 2 was issued
            // 1 -> 1, 2 -> 3
        // 3 was issued
            // 1 -> 1, 2 -> 2
        vm_core.pipeline_reg_instrs_.id_issue_1 = (issued_1) ? instr2 : instr1;
        vm_core.pipeline_reg_instrs_.id_issue_2 = (issued_3) ? instr2 : instr3;
    }

    return num_issued;
}



} // namespace triple_issue
