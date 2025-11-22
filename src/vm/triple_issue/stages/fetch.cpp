#include "vm/triple_issue/stages/stages.h"

namespace triple_issue
{

void fetch3(TripleIssueCore& vm_core){
    TripleIssueInstrContext instr1;
    TripleIssueInstrContext instr2;
    TripleIssueInstrContext instr3;

    if(vm_core.pc>=vm_core.program_size_){
        instr1.illegal = true;
    }

    instr1.pc = vm_core.pc;
    instr1.instruction = vm_core.memory_controller_.ReadWord(vm_core.pc);
    vm_core.AddToProgramCounter(4);

    if(vm_core.pc>=vm_core.program_size_){
        instr2.illegal = true;
    }

    instr2.pc = vm_core.pc;
    instr2.instruction = vm_core.memory_controller_.ReadWord(vm_core.pc);
    vm_core.AddToProgramCounter(4);

    if(vm_core.pc>=vm_core.program_size_){
        instr3.illegal = true;
    }

    instr3.pc = vm_core.pc;
    instr3.instruction = vm_core.memory_controller_.ReadWord(vm_core.pc);

    if(vm_core.branch_prediction_enabled_){
        if(vm_core.branch_prediction_static_){
            // instr1 is a branch prediction
            {
                auto [take_branch, new_pc] = vm_core.branch_predictor_.static_predict(instr1.pc);
                if(take_branch){
                    vm_core.SetProgramCounter(new_pc);
                    instr1.branch_predicted_taken = true;
                    instr2.illegal = true;
                    instr3.illegal = true;

                    vm_core.pipeline_reg_instrs_.if_id_1 = instr1;
                    vm_core.pipeline_reg_instrs_.if_id_2 = instr2;
                    vm_core.pipeline_reg_instrs_.if_id_3 = instr3;
                    return;
                }
            }

            // instr2 is a branch prediction
            {
                auto [take_branch, new_pc] = vm_core.branch_predictor_.static_predict(instr2.pc);
                if(take_branch){
                    vm_core.SetProgramCounter(new_pc);
                    instr3.illegal = true;

                    instr2.branch_predicted_taken = true;

                    vm_core.pipeline_reg_instrs_.if_id_1 = instr1;
                    vm_core.pipeline_reg_instrs_.if_id_2 = instr2;
                    vm_core.pipeline_reg_instrs_.if_id_3 = instr3;
                    return;
                }
            }

            // instr3 is a branch prediction
            {
                auto [take_branch, new_pc] = vm_core.branch_predictor_.static_predict(instr3.pc);
                vm_core.SetProgramCounter(new_pc);

                if(take_branch){
                    instr3.branch_predicted_taken = true;

                    vm_core.pipeline_reg_instrs_.if_id_1 = instr1;
                    vm_core.pipeline_reg_instrs_.if_id_2 = instr2;
                    vm_core.pipeline_reg_instrs_.if_id_3 = instr3;
                    return;
                }
            }

            // nothing is a branch
            vm_core.pipeline_reg_instrs_.if_id_1 = instr1;
            vm_core.pipeline_reg_instrs_.if_id_2 = instr2;
            vm_core.pipeline_reg_instrs_.if_id_3 = instr3;
            return;
        }
        else{
            // instr1 is a branch prediction
            {
                auto [take_branch, new_pc] = vm_core.branch_predictor_.dynamic_predict(instr1.pc);
                if(take_branch){
                    vm_core.SetProgramCounter(new_pc);
                    instr1.branch_predicted_taken = true;
                    instr2.illegal = true;
                    instr3.illegal = true;

                    vm_core.pipeline_reg_instrs_.if_id_1 = instr1;
                    vm_core.pipeline_reg_instrs_.if_id_2 = instr2;
                    vm_core.pipeline_reg_instrs_.if_id_3 = instr3;
                    return;
                }
            }

            // instr2 is a branch prediction
            {
                auto [take_branch, new_pc] = vm_core.branch_predictor_.dynamic_predict(instr2.pc);
                if(take_branch){
                    vm_core.SetProgramCounter(new_pc);
                    instr3.illegal = true;

                    instr2.branch_predicted_taken = true;

                    vm_core.pipeline_reg_instrs_.if_id_1 = instr1;
                    vm_core.pipeline_reg_instrs_.if_id_2 = instr2;
                    vm_core.pipeline_reg_instrs_.if_id_3 = instr3;
                    return;
                }
            }

            // instr3 is a branch prediction
            {
                auto [take_branch, new_pc] = vm_core.branch_predictor_.dynamic_predict(instr3.pc);
                vm_core.SetProgramCounter(new_pc);

                if(take_branch){
                    instr3.branch_predicted_taken = true;

                    vm_core.pipeline_reg_instrs_.if_id_1 = instr1;
                    vm_core.pipeline_reg_instrs_.if_id_2 = instr2;
                    vm_core.pipeline_reg_instrs_.if_id_3 = instr3;
                    return;
                }
            }

            // nothing is a branch
            vm_core.pipeline_reg_instrs_.if_id_1 = instr1;
            vm_core.pipeline_reg_instrs_.if_id_2 = instr2;
            vm_core.pipeline_reg_instrs_.if_id_3 = instr3;
            return;
        }
    }
    else{
        vm_core.pipeline_reg_instrs_.if_id_1 = instr1;
        vm_core.pipeline_reg_instrs_.if_id_2 = instr2;
        vm_core.pipeline_reg_instrs_.if_id_3 = instr3;
        vm_core.AddToProgramCounter(4);
        return;
    }

}
    

void fetch2(TripleIssueCore& vm_core){
    TripleIssueInstrContext instr1;
    TripleIssueInstrContext instr2;

    if(vm_core.pc>=vm_core.program_size_){
        instr1.illegal = true;
    }

    instr1.pc = vm_core.pc;
    instr1.instruction = vm_core.memory_controller_.ReadWord(vm_core.pc);
    vm_core.AddToProgramCounter(4);

    if(vm_core.pc>=vm_core.program_size_){
        instr2.illegal = true;
    }

    instr2.pc = vm_core.pc;
    instr2.instruction = vm_core.memory_controller_.ReadWord(vm_core.pc);
    
    if(vm_core.branch_prediction_enabled_){
        if(vm_core.branch_prediction_static_){
            // instr1 is a branch instruction
            {
                auto [take_branch, new_pc] = vm_core.branch_predictor_.static_predict(instr1.pc);
                if(take_branch){
                    vm_core.SetProgramCounter(new_pc);
                    instr1.branch_predicted_taken = true;
                    instr2.illegal = true;
    
                    vm_core.pipeline_reg_instrs_.if_id_2 = instr1;
                    vm_core.pipeline_reg_instrs_.if_id_3 = instr2;
                    return;
                }
            }
            
            // instr2 is a branch instruction
            {
                auto [take_branch, new_pc] = vm_core.branch_predictor_.static_predict(instr2.pc);
                vm_core.SetProgramCounter(new_pc);
                if(take_branch){
                    instr2.branch_predicted_taken = true;
                    vm_core.pipeline_reg_instrs_.if_id_2 = instr1;
                    vm_core.pipeline_reg_instrs_.if_id_3 = instr2;
                    return;
                }
            }

            // nothing is a branch
            vm_core.pipeline_reg_instrs_.if_id_2 = instr1;
            vm_core.pipeline_reg_instrs_.if_id_3 = instr2;
            return;
        }
        else{
            // instr1 is a branch instruction
            {
                auto [take_branch, new_pc] = vm_core.branch_predictor_.dynamic_predict(instr1.pc);
                if(take_branch){
                    vm_core.SetProgramCounter(new_pc);
                    instr1.branch_predicted_taken = true;
                    instr2.illegal = true;
                    vm_core.pipeline_reg_instrs_.if_id_2 = instr1;
                    vm_core.pipeline_reg_instrs_.if_id_3 = instr2;
                    return;
                }
            }

            // instr2 is a branch instruction
            {
                auto [take_branch, new_pc] = vm_core.branch_predictor_.dynamic_predict(instr2.pc);
                vm_core.SetProgramCounter(new_pc);
                if(take_branch){
                    instr2.branch_predicted_taken = true;
                    vm_core.pipeline_reg_instrs_.if_id_2 = instr1;
                    vm_core.pipeline_reg_instrs_.if_id_3 = instr2;
                    return;
                }
            }

            // nothing is a branch
            vm_core.pipeline_reg_instrs_.if_id_2 = instr1;
            vm_core.pipeline_reg_instrs_.if_id_3 = instr2;
            return;
        }
    }
    else{
        vm_core.pipeline_reg_instrs_.if_id_2 = instr1;
        vm_core.pipeline_reg_instrs_.if_id_3 = instr2;
        vm_core.AddToProgramCounter(4);
    }
}


void fetch1(TripleIssueCore& vm_core){
    TripleIssueInstrContext instr;
    instr.pc = vm_core.pc;
    instr.instruction = vm_core.memory_controller_.ReadWord(vm_core.pc);

    if(instr.pc>=vm_core.program_size_){
        instr.illegal = true;
    }

    if(vm_core.branch_prediction_enabled_){
        if(vm_core.branch_prediction_static_){
            auto [take_branch, new_pc] = vm_core.branch_predictor_.static_predict(instr.pc);
            vm_core.SetProgramCounter(new_pc);

            if(take_branch){
                instr.branch_predicted_taken = true;
            }
        }
        else{
            auto[take_branch, new_pc] = vm_core.branch_predictor_.dynamic_predict(instr.pc);
            vm_core.SetProgramCounter(new_pc);
    
            if(take_branch){
                instr.branch_predicted_taken = true;
            }
        }
    }
    else{
        vm_core.AddToProgramCounter(4);
    }

    vm_core.pipeline_reg_instrs_.if_id_3 = instr;
}

void TripleIssueStages::Fetch(TripleIssueCore& vm_core, int num_issued){
    if(num_issued==3){
        fetch3(vm_core);
    }
    else if(num_issued==2){
        fetch2(vm_core);
    }
    else{
        fetch1(vm_core);
    }
}



} // namespace triple_issue
