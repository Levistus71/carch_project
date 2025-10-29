/**
 * @file rvss_vm.cpp
 * @brief RVSS VM implementation
 * @author Vishank Singh, https://github.com/VishankSingh
 */

#include "vm/rv5s/rv5s_vm.h"


using instruction_set::Instruction;
using instruction_set::get_instr_encoding;




void RV5SVM::LoadVM() {
	// resseting vm
	Reset();

	// updating VM state
	this->pipelining_enabled = vm_config::config.getPipeliningStatus();
	if(this->pipelining_enabled){
		this->data_forwarding_enabled = vm_config::config.getDataFowardingStatus();
		this->hazard_detection_enabled = vm_config::config.getHazardDetectionStatus();
	}
	this->max_undo_stack_size = vm_config::config.getMaxUndoStackSize();

	std::vector<bool> t = vm_config::config.getBranchPredictionStatus();
	this->branch_prediction_enabled = t[0];
	this->branch_prediction_static = t[1];
	this->branch_prediction_dynamic = t[2];

	// Loading the instructions (machine code) into memory
	unsigned int counter = 0;
	for (const auto &instruction: program_.text_buffer) {
		memory_controller_.WriteWord(counter, instruction);
		counter += 4;
	}
	this->program_size_ = counter;
	AddBreakpoint(program_size_, false);  // address

	// Loading data section into memory
	unsigned int data_counter = 0;
	uint64_t base_data_address = vm_config::config.getDataSectionStart();

	auto align = [&](unsigned int alignment) {
		if (data_counter % alignment != 0)
		data_counter += alignment - (data_counter % alignment);
	};

	for (const auto& data : program_.data_buffer) {
		std::visit([&](auto&& value) {
			using T = std::decay_t<decltype(value)>; 
			

			if constexpr (std::is_same_v<T, uint8_t>) {
				align(1);
				memory_controller_.WriteByte(base_data_address + data_counter, value);  // Write a byte
				data_counter += 1;
			}
			else if constexpr (std::is_same_v<T, uint16_t>) {
				align(2);
				memory_controller_.WriteHalfWord(base_data_address + data_counter, value);  // Write a halfword (16 bits)
				data_counter += 2;
			}
			else if constexpr (std::is_same_v<T, uint32_t>) {
				align(4);
				memory_controller_.WriteWord(base_data_address + data_counter, value);  // Write a word (32 bits)
				data_counter += 4;
			}
			else if constexpr (std::is_same_v<T, uint64_t>) {
				align(8);
				memory_controller_.WriteDoubleWord(base_data_address + data_counter, value);  // Write a double word (64 bits)
				data_counter += 8;
			}
			else if constexpr (std::is_same_v<T, float>) {
				align(4);
				uint32_t float_as_int;
				std::memcpy(&float_as_int, &value, sizeof(float));
				memory_controller_.WriteWord(base_data_address + data_counter, float_as_int);  // Write the float as a word
				data_counter += 4;
			}
			else if constexpr (std::is_same_v<T, double>) {
				align(8);
				uint64_t double_as_int;
				std::memcpy(&double_as_int, &value, sizeof(double));
				memory_controller_.WriteDoubleWord(base_data_address + data_counter, double_as_int);  // Write the double as a double word
				data_counter += 8;
			}
			else if constexpr (std::is_same_v<T, std::string>) {
				align(1);
				for (size_t i = 0; i < value.size(); i++) {
					memory_controller_.WriteByte(base_data_address + data_counter, static_cast<uint8_t>(value[i]));  // Write each byte of the string
					data_counter += 1;
				}
			}
		}, data);
	}

	globals::vm_cout_file << "VM Program Loaded!" << std::endl;
	output_status_ = "VM_PROGRAM_LOADED";

	DumpState(globals::vm_state_dump_file_path);
}


// TODO: implement pipelining (multiple instructions in this function, data forwarding, hazard detection unit, branch prediction)
void RV5SVM::Run() {

	if(!pipelining_enabled){
		RunSingleCycle();
	}
	else {
		RunPipelined();
	}

}

// TODO: complete these:
void RV5SVM::DebugRun() {

	if(!pipelining_enabled){
		DebugRunSingleCycle();
	}
	else{
		DebugRunPipelined();
	}
}

void RV5SVM::Step() {
	if(!pipelining_enabled){
		SingleCycleStep(true);
	}
	else{
		StepPipelined(true);
	}
}

void RV5SVM::Undo() {
	if(!pipelining_enabled){
		SingleCycleUndo();
	}
	else{
		globals::vm_cout_file << "Still working on pipelining." << std::endl; 
	}
}


void RV5SVM::Reset() {
	this->program_counter_ = 0;
	this->registers_.Reset();
	this->memory_controller_.Reset();
}
