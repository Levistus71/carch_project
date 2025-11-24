#include "vm/triple_issue/stages/stages.h"
#include "vm/triple_issue/hardware/commit_buffer.h"
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

            // FIXME: Same issue as discussed in the dual issue.
            vm_core.decode_unit_.SetRegImmValues(instr, vm_core.register_file_);

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

                // FIXME: Same issue as discussed in the dual issue.
                vm_core.decode_unit_.SetRegImmValues(instr, vm_core.register_file_);

                vm_core.alu_que_.Push(instr, vm_core);

                return true;
            }
        }
        else{
            if(falu_empty_slots>0 && commit_buffer_empty_slots>0){

                auto[idx, epoch] = vm_core.commit_buffer_.Reserve();
                instr.rob_idx = idx;
                instr.epoch = epoch;

                // FIXME: Same issue as discussed in the dual issue.
                vm_core.decode_unit_.SetRegImmValues(instr, vm_core.register_file_);

                vm_core.falu_que_.Push(instr, vm_core);

                return true;
            }
        }
    }

    return false;
}


bool check_dependency(TripleIssueInstrContext& first_instr, TripleIssueInstrContext& second_instr){
        bool se_rs1__fi_rd_clash = (second_instr.uses_rs1) && (second_instr.rs1 == first_instr.rd) && (second_instr.rs1_from_fprf == first_instr.reg_write_to_fpr) && (second_instr.rs1 != 0 && !second_instr.rs1_from_fprf);
        bool se_rs2__fi_rd_clash = (second_instr.uses_rs2) && (second_instr.rs2 == first_instr.rd) && (second_instr.rs2_from_fprf == first_instr.reg_write_to_fpr) && (second_instr.rs2 != 0 && !second_instr.rs2_from_fprf);
        bool se_rs3__fi_rd_clash = (second_instr.uses_rs3) && (second_instr.frs3 == first_instr.rd) && (first_instr.reg_write_to_fpr);
        bool clash = se_rs1__fi_rd_clash || se_rs2__fi_rd_clash || se_rs3__fi_rd_clash;

        return clash;
}

    
int TripleIssueStages::Issue(TripleIssueCore& vm_core){
    TripleIssueInstrContext instr1 = vm_core.pipeline_reg_instrs_.id_issue_1;
    TripleIssueInstrContext instr2 = vm_core.pipeline_reg_instrs_.id_issue_2;
    TripleIssueInstrContext instr3 = vm_core.pipeline_reg_instrs_.id_issue_3;

    // std::cout << "alu slots before pushing : " << vm_core.alu_que_.EmptySlots() << std::endl;
    bool issued_1 = issue_single(vm_core, instr1);    
    bool issued_2 = false;
    bool issued_3 = false;

    if(issued_1){
        issued_2 = issue_single(vm_core, instr2);
    }
    else{
        if(!check_dependency(instr1, instr2)){
            issued_2 = issue_single(vm_core, instr2);
        }
    }

    if(issued_1){
        if(issued_2){
            issued_3 = issue_single(vm_core, instr3);
        }
        else{
            if(!check_dependency(instr2, instr3)){
                issued_3 = issue_single(vm_core, instr3);
            }
        }
    }
    else{
        if(!check_dependency(instr1, instr3)){
            if(issued_2){
                issued_3 = issue_single(vm_core, instr3);
            }
            else{
                if(!check_dependency(instr2, instr3)){
                    issued_3 = issue_single(vm_core, instr3);
                }
            }
        }
    }
    
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
