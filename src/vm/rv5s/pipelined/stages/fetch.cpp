#include "vm/rv5s/pipelined/stages/stages.h"

namespace rv5s {

void PipelinedStages::Fetch(PipelinedCore& vm_core){
    PipelinedInstrContext& if_instruction = vm_core.GetIfInstruction();
	if(if_instruction.nopped)
		return;
	if_instruction.pc = vm_core.program_counter_;
  	if_instruction.instruction = vm_core.memory_controller_.ReadWord(vm_core.program_counter_);
	if_instruction.branch_predicted_taken = false;

	if(vm_core.branch_prediction_enabled_){
		if(vm_core.branch_prediction_static_){
			auto [take_branch, new_pc] = vm_core.branch_predictor_.static_predict(vm_core.program_counter_);
			vm_core.SetProgramCounter(new_pc);
			if(take_branch){
				if_instruction.branch_predicted_taken = true;
			}
		}
		else{
			auto [take_branch, new_pc] = vm_core.branch_predictor_.dynamic_predict(vm_core.program_counter_);
			vm_core.SetProgramCounter(new_pc);
			if(take_branch){
				if_instruction.branch_predicted_taken = true;
			}
		}
	}
	// no branch prediction, we go with the flow.
	// if the branch was taken, we flush the 2 cycles
	else{
		vm_core.AddToProgramCounter(4);
	}
}

} // namespace rv5s