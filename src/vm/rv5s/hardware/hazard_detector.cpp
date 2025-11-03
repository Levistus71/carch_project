#include "vm/rv5s/hardware/hazard_detector.h"
#include "vm/rv5s/core/core.h"

/*
Data hazards:

    Case 1: no data forwarding

        the instruction under focus is the one which is being decoded. i.e. the one which is in the decode stage.

        if the RD of instructions in ex / mem stage (NOT WB STAGE, 'write first') is rs1 or rs2 of the focused instruction, we do:
            stall(if_instruction):
                pc = pc - 4 -> results in the same instruction being fetched
            stall(id_instruction):
                do nothing -> the instruction queue contains the same instruction in the ID position, decodes the same instruction again
            insert_bubble(ex_instruction):
                initial instruction deque : a b c d e
                next instruction deque : a b BUBBLE(nopped instruction) c d
            
        we bring the id_instruction again for inspection
        that's it.

*/

namespace rv5s{

bool HazardDetector::DetectDataHazard(Core& vm_core){
    if(!vm_core.data_forwarding_enabled_){
        return DetectDataHazardWithoutForwarding(vm_core);
    }
    else{
        return DetectDataHazardWithForwarding(vm_core);
    }
}

bool HazardDetector::DetectDataHazardWithoutForwarding(Core& vm_core){
    InstrContext& id_instruction = vm_core.GetIdInstruction();
    InstrContext& ex_instruction = vm_core.GetExInstruction();
    InstrContext& mem_instruction = vm_core.GetMemInstruction();

    // anything which messes with the csr, we stall the pipeline (cause idk what these do)
    if(ex_instruction.csr_op || mem_instruction.csr_op){
        return true;
    }

    // if id instruction is nopped, there's no data hazard
    if(id_instruction.nopped)
        return false;
    
    // if(ex_instruction.nopped) then ex_instruction.reg_write is false, idk why we check the nopped variable but ok
    if(!ex_instruction.nopped && ex_instruction.reg_write){   // checking if the ex instruction changes the register file
        bool id_rs1__ex_rd_clash = (id_instruction.uses_rs1) && (id_instruction.rs1 == ex_instruction.rd) && (id_instruction.rs1_from_fprf == ex_instruction.reg_write_to_fpr);
        bool id_rs2__ex_rd_clash = (id_instruction.uses_rs2) && (id_instruction.rs2 == ex_instruction.rd) && (id_instruction.rs2_from_fprf == ex_instruction.reg_write_to_fpr);
        bool id_rs3__ex_rd_clash = (id_instruction.uses_rs3) && (id_instruction.frs3 == ex_instruction.rd) && (ex_instruction.reg_write_to_fpr);

        if(id_rs1__ex_rd_clash || id_rs2__ex_rd_clash || id_rs3__ex_rd_clash){
            return true;
        }
    }

    if(!mem_instruction.nopped && mem_instruction.reg_write){  // checking if the mem instruction changes the register file
        bool id_rs1__mem_rd_clash = (id_instruction.uses_rs1) && (id_instruction.rs1 == mem_instruction.rd) && (id_instruction.rs1_from_fprf == mem_instruction.reg_write_to_fpr);
        bool id_rs2__mem_rd_clash = (id_instruction.uses_rs2) && (id_instruction.rs2 == mem_instruction.rd) && (id_instruction.rs2_from_fprf == mem_instruction.reg_write_to_fpr);
        bool id_rs3__mem_rd_clash = (id_instruction.uses_rs3) && (id_instruction.frs3 == mem_instruction.rd) && (mem_instruction.reg_write_to_fpr);

        if(id_rs1__mem_rd_clash || id_rs2__mem_rd_clash || id_rs3__mem_rd_clash){
            return true;
        }
    }

    return false;
}


bool HazardDetector::DetectDataHazardWithForwarding(Core& vm_core){
    InstrContext& id_instruction = vm_core.GetIdInstruction();
    InstrContext& ex_instruction = vm_core.GetExInstruction();
    InstrContext& mem_instruction = vm_core.GetMemInstruction();

    // anything which messes with the csr, we stall the pipeline (cause idk what these do)
    if(ex_instruction.csr_op || mem_instruction.csr_op){
        return true;
    }

    // if id instruction is nopped, there's no data hazard
    if(id_instruction.nopped)
        return false;
    
    if(!ex_instruction.nopped && ex_instruction.reg_write){   // checking if the ex instruction changes the register file
        bool id_rs1__ex_rd_clash = (id_instruction.uses_rs1) && (id_instruction.rs1 == ex_instruction.rd) && (id_instruction.rs1_from_fprf == ex_instruction.reg_write_to_fpr);
        bool id_rs2__ex_rd_clash = (id_instruction.uses_rs2) && (id_instruction.rs2 == ex_instruction.rd) && (id_instruction.rs2_from_fprf == ex_instruction.reg_write_to_fpr);
        bool id_rs3__ex_rd_clash = (id_instruction.uses_rs3) && (id_instruction.frs3 == ex_instruction.rd) && (ex_instruction.reg_write_to_fpr);
        bool clash = id_rs1__ex_rd_clash || id_rs2__ex_rd_clash || id_rs3__ex_rd_clash;

        if(clash && ex_instruction.mem_read) // data hazard
            return true;

        if(id_rs1__ex_rd_clash){            
            if(id_instruction.rs1_from_fprf)
                id_instruction.frs1_value = ex_instruction.alu_out;
            else
                id_instruction.rs1_value = ex_instruction.alu_out;
        }

        if(id_rs2__ex_rd_clash){
            if(id_instruction.rs2_from_fprf)
                id_instruction.frs2_value = ex_instruction.alu_out;
            else
                id_instruction.rs2_value = ex_instruction.alu_out;
        }

        if(id_rs3__ex_rd_clash){
            id_instruction.frs3_value = ex_instruction.alu_out;
        }
    }

    if(!mem_instruction.nopped && mem_instruction.reg_write){  // checking if the mem instruction changes the register file
        bool id_rs1__mem_rd_clash = (id_instruction.uses_rs1) && (id_instruction.rs1 == mem_instruction.rd) && (id_instruction.rs1_from_fprf == mem_instruction.reg_write_to_fpr);
        bool id_rs2__mem_rd_clash = (id_instruction.uses_rs2) && (id_instruction.rs2 == mem_instruction.rd) && (id_instruction.rs2_from_fprf == mem_instruction.reg_write_to_fpr);
        bool id_rs3__mem_rd_clash = (id_instruction.uses_rs3) && (id_instruction.frs3 == mem_instruction.rd) && (mem_instruction.reg_write_to_fpr);

        if(id_rs1__mem_rd_clash){
            if(id_instruction.rs1_from_fprf)
                id_instruction.frs1_value = mem_instruction.mem_to_reg ? mem_instruction.mem_out : mem_instruction.alu_out;
            else
                id_instruction.rs1_value = mem_instruction.mem_to_reg ? mem_instruction.mem_out : mem_instruction.alu_out;
        }

        if(id_rs2__mem_rd_clash){
            if(id_instruction.rs1_from_fprf)
                id_instruction.frs1_value = mem_instruction.mem_to_reg ? mem_instruction.mem_out : mem_instruction.alu_out;
            else
                id_instruction.rs1_value = mem_instruction.mem_to_reg ? mem_instruction.mem_out : mem_instruction.alu_out;
        }

        if(id_rs3__mem_rd_clash){
            id_instruction.frs3_value = mem_instruction.mem_to_reg ? mem_instruction.mem_out : mem_instruction.alu_out;
        }
    }

    return false;
}


void HazardDetector::HandleDataHazard(Core& vm_core){
    vm_core.AddToProgramCounter(-4);

    std::deque<InstrContext> new_instruction_deque;
    new_instruction_deque.push_back(vm_core.GetIfInstruction());
    new_instruction_deque.push_back(vm_core.GetIdInstruction());
    InstrContext bubble;
    bubble.nopify();
    bubble.bubbled = true;
    new_instruction_deque.push_back(bubble);
    new_instruction_deque.push_back(vm_core.GetExInstruction());
    new_instruction_deque.push_back(vm_core.GetMemInstruction());

    vm_core.instruction_deque_ = new_instruction_deque;
}


bool HazardDetector::DetectControlHazard(Core& vm_core){
    InstrContext& ex_instruction = vm_core.GetExInstruction();

    /*
        we don't care if the ex_instruction.immediate is 4 or not.

        take this for eg:
            jal x0,Label
            Label:
                addi x3,x0,1
                addi x4,x0,1
        
        when jal reaches ex stage and if the btb predicts not taken but immediate was 4, the "addi x4,x0,1" would be in id and "addi x3,x0,1" would be in if (because the value was updated in Stages::Execute())
    */
    // return ex_instruction.branch && (ex_instruction.branch_predicted_taken != ex_instruction.branch_taken) && (ex_instruction.immediate != 4);

    return ex_instruction.branch && (ex_instruction.branch_predicted_taken != ex_instruction.branch_taken);
}


void HazardDetector::HandleControlHazard(Core& vm_core){
    InstrContext& ex_instruction = vm_core.GetExInstruction();

    if(vm_core.branch_prediction_enabled_){
        vm_core.branch_predictor_.update_btb(ex_instruction.pc, ex_instruction.branch_taken, vm_core.program_counter_);
    }
    
    InstrContext& if_instruction = vm_core.GetIfInstruction();
    if_instruction.nopify();
    InstrContext& id_instruction = vm_core.GetIdInstruction();
    id_instruction.nopify();
}

} // namespace rv5s