#include "vm/rv5s/rv5s_vm.h"

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


bool RV5SVM::DetectDataHazardWithoutForwarding(){
    InstrContext& id_instruction = GetIdInstruction();
    InstrContext& ex_instruction = GetExInstruction();
    InstrContext& mem_instruction = GetMemInstruction();

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


bool RV5SVM::DetectDataHazardWithForwarding(){
    InstrContext& id_instruction = GetIdInstruction();
    InstrContext& ex_instruction = GetExInstruction();
    InstrContext& mem_instruction = GetMemInstruction();

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


void RV5SVM::HandleDataHazard(){
    AddToProgramCounter(-4);

    std::deque<InstrContext> new_instruction_deque;
    new_instruction_deque.push_back(GetIfInstruction());
    new_instruction_deque.push_back(GetIdInstruction());
    InstrContext bubble;
    bubble.nopify();
    bubble.bubbled = true;
    new_instruction_deque.push_back(bubble);
    new_instruction_deque.push_back(GetExInstruction());
    new_instruction_deque.push_back(GetMemInstruction());

    this->instruction_deque = new_instruction_deque;
}




bool RV5SVM::DetectControlHazard(){
    InstrContext& ex_instruction = GetExInstruction();

    return ex_instruction.branch && (ex_instruction.branch_predicted_taken != ex_instruction.branch_taken);
}


void RV5SVM::HandleControlHazard(){
    InstrContext& ex_instruction = GetExInstruction();
    branch_predictor.update_btb(ex_instruction.pc, ex_instruction.branch_taken, this->program_counter_);
    
    InstrContext& if_instruction = GetIfInstruction();
    if_instruction.nopify();
    InstrContext& id_instruction = GetIdInstruction();
    id_instruction.nopify();
}