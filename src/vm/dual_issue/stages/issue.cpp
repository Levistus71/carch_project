#include "vm/dual_issue/stages/stages.h"


namespace dual_issue
{

std::pair<bool, bool> can_issue(DualIssueInstrContext& instr1, DualIssueInstrContext& instr2, DualIssueCore& vm_core){
    bool can_issue_1 = false;
    bool can_issue_2 = false;

    int num_lsu_que = 0;
    if(instr1.mem_read || instr1.mem_write){
        num_lsu_que++;
    }
    if(instr2.mem_read || instr2.mem_write){
        num_lsu_que++;
    }

    if(num_lsu_que==0){
        int num_slots = vm_core.alu_que_.EmptySlots();
        if(num_slots>=2){
            can_issue_1 = true;
            can_issue_2 = true;
        }
        else if(num_slots==1){
            can_issue_1 = true;
        }
    }
    else if(num_lsu_que==1){
        int num_slots_alu = vm_core.alu_que_.EmptySlots();
        int num_slots_lsu = vm_core.lsu_que_.EmptySlots();
        if(num_slots_alu>=1){
            if(instr1.mem_read || instr1.mem_write)
                can_issue_2 = true;
            else
                can_issue_1 = true;
        }
        if(num_slots_lsu>=1){
            if(instr1.mem_read || instr1.mem_write)
                can_issue_1 = true;
            else
                can_issue_2 = true;
        }
    }
    else{
        int num_slots = vm_core.lsu_que_.EmptySlots();
        if(num_slots>=2){
            can_issue_1 = true;
            can_issue_2 = true;
        }
        else if(num_slots==1){
            can_issue_1 = true;
        }
    }

    int can_issues = can_issue_1 + can_issue_2;
    if(can_issues==2){
        int empty_slots = vm_core.commit_buffer_.EmptySlots();
        if(empty_slots==1){
            can_issue_2 = false;
        }
        else if(empty_slots==0){
            can_issue_1 = false;
            can_issue_2 = false;
        }
    }
    else if(can_issues==1){
        int empty_slots = vm_core.commit_buffer_.EmptySlots();
        if(empty_slots==0){
            can_issue_1 = false;
            can_issue_2 = false;
        }
    }
    else{
        can_issue_1 = false;
        can_issue_2 = false;
    }

    if(instr1.illegal)
        can_issue_1 = true;
    if(instr2.illegal)
        can_issue_2 = true;
    
    return {can_issue_1, can_issue_2};
}


void reserve_push(DualIssueInstrContext& instr, DualIssueCore& vm_core){
    if(instr.illegal){
        return;
    }
    
    auto [rob_idx, epoch] = vm_core.commit_buffer_.Reserve();
    instr.rob_idx = rob_idx;
    instr.epoch = epoch;

    if(instr.mem_read || instr.mem_write){
        vm_core.lsu_que_.Push(instr, vm_core);
    }
    else{
        vm_core.alu_que_.Push(instr, vm_core);
    }
}

bool check_dependency(DualIssueInstrContext& first_instr, DualIssueInstrContext& second_instr){
        bool se_rs1__fi_rd_clash = (second_instr.uses_rs1) && (second_instr.rs1 == first_instr.rd) && (second_instr.rs1_from_fprf == first_instr.reg_write_to_fpr) && (second_instr.rs1 != 0 && !second_instr.rs1_from_fprf);
        bool se_rs2__fi_rd_clash = (second_instr.uses_rs2) && (second_instr.rs2 == first_instr.rd) && (second_instr.rs2_from_fprf == first_instr.reg_write_to_fpr) && (second_instr.rs2 != 0 && !second_instr.rs2_from_fprf);
        bool se_rs3__fi_rd_clash = (second_instr.uses_rs3) && (second_instr.frs3 == first_instr.rd) && (first_instr.reg_write_to_fpr);
        bool clash = se_rs1__fi_rd_clash || se_rs2__fi_rd_clash || se_rs3__fi_rd_clash;

        return clash;
}

int DualIssueStages::Issue(DualIssueCore& vm_core){
    DualIssueInstrContext instr1 = vm_core.pipeline_reg_instrs_.id_issue_1;
    DualIssueInstrContext instr2 = vm_core.pipeline_reg_instrs_.id_issue_2;

    auto [can_issue_1, can_issue_2] = can_issue(instr1, instr2, vm_core);

    if(can_issue_1 && can_issue_2){
        if(!instr1.illegal){
            reserve_push(instr1, vm_core);
        }
        if(!instr2.illegal){
            reserve_push(instr2, vm_core);
        }
        return 2;
    }
    else if(!can_issue_1 && !can_issue_2){
        return 0;
    }
    
    if(can_issue_1){
        if(!instr1.illegal){
            reserve_push(instr1, vm_core);
        }
        vm_core.pipeline_reg_instrs_.id_issue_1 = instr2;
        return 1;
    }
    else{
        if(!instr2.illegal && !check_dependency(instr1, instr2)){
            reserve_push(instr2, vm_core);
        }
        else if(check_dependency(instr1, instr2)){
            return 0;
        }
        return 1;
    }
}


    
} // namespace dual_issue
