#include "vm/rv5s/rv5s_vm.h"

void RV5SVM::Fetch() {
	InstrContext& if_instruction = GetIfInstruction();
	if_instruction.pc = this->program_counter_;
  	if_instruction.instruction = this->memory_controller_.ReadWord(program_counter_);
	if_instruction.branch_predicted_taken = false;

	if(this->branch_prediction_enabled){
		if(this->branch_prediction_static){
			auto [take_branch, new_pc] = this->branch_predictor.static_predict(this->program_counter_);
			SetProgramCounter(new_pc);
			if(take_branch){
				if_instruction.branch_taken = true;
			}
		}
		else{
			auto [take_branch, new_pc] = this->branch_predictor.dynamic_predict(this->program_counter_);
			SetProgramCounter(new_pc);
			if(take_branch){
				if_instruction.branch_taken = true;
			}
		}
	}
	// no branch prediction, we go with the flow.
	// if the branch was taken, we flush the 2 cycles
	else{
		AddToProgramCounter(4);
	}
}