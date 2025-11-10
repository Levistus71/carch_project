#include "vm/rv5s/pipelined/core/core.h"
#include <cassert>

namespace rv5s{

PipelinedCore::PipelinedCore() {
    Reset();
}

void PipelinedCore::AddToProgramCounter(int64_t value){
    program_counter_ = static_cast<uint64_t>(static_cast<int64_t>(program_counter_) + value);
}

uint64_t PipelinedCore::GetProgramCounter() const {
    return program_counter_;
}

void PipelinedCore::SetProgramCounter(uint64_t value){
    program_counter_ = value;
}

PipelinedInstrContext& PipelinedCore::GetIfInstruction(){
    return instruction_deque_[0];
}
PipelinedInstrContext& PipelinedCore::GetIdInstruction(){
    return instruction_deque_[1];
}
PipelinedInstrContext& PipelinedCore::GetExInstruction(){
    return instruction_deque_[2];
}
PipelinedInstrContext& PipelinedCore::GetMemInstruction(){
    return instruction_deque_[3];
}
PipelinedInstrContext& PipelinedCore::GetWbInstruction(){
    return instruction_deque_[4];
}

void PipelinedCore::ClearStop(){
    stop_requested_ = false;
}

void PipelinedCore::Reset(){
	this->stop_requested_ = false;

    this->program_counter_ = 0;
	this->register_file_.Reset();
	this->memory_controller_.Reset();
	// assert(instruction_deque_.size()==5);
	instruction_deque_.clear();

	branch_predictor_.reset();

    for(int i=0;i<5;i++){
        // instruction_deque_.pop_back();
        
        PipelinedInstrContext nop{};
        nop.nopify();
		nop.bubbled = true;

        instruction_deque_.push_front(nop);
    }
}

void PipelinedCore::Load(AssembledProgram& program){
    Reset();

    // updating core state
	this->data_forwarding_enabled_ = vm_config::config.getDataFowardingStatus();
	this->hazard_detection_enabled_ = vm_config::config.getHazardDetectionStatus();
	this->max_undo_stack_size_ = vm_config::config.getMaxUndoStackSize();

	std::vector<bool> t = vm_config::config.getBranchPredictionStatus();
	this->branch_prediction_enabled_ = t[0];
	this->branch_prediction_static_ = t[1];


    // Loading the instructions (machine code) into memory
	unsigned int counter = 0;
	for (const auto &instruction: program.text_buffer) {
		this->memory_controller_.WriteWord(counter, instruction);
		counter += 4;
	}
	this->program_size_ = counter;


    // Loading data section into memory
    unsigned int data_counter = 0;
	uint64_t base_data_address = vm_config::config.getDataSectionStart();

	auto align = [&](unsigned int alignment) {
		if (data_counter % alignment != 0)
		data_counter += alignment - (data_counter % alignment);
	};

    for (const auto& data : program.data_buffer) {
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
}


void PipelinedCore::Load(){
	Reset();

    // updating core state
	this->data_forwarding_enabled_ = vm_config::config.getDataFowardingStatus();
	this->hazard_detection_enabled_ = vm_config::config.getHazardDetectionStatus();
	this->max_undo_stack_size_ = vm_config::config.getMaxUndoStackSize();

	std::vector<bool> t = vm_config::config.getBranchPredictionStatus();
	this->branch_prediction_enabled_ = t[0];
	this->branch_prediction_static_ = t[1];

	core_stats_.branch_instrs = 0;
	core_stats_.branch_mispredicts = 0;
	core_stats_.cycles = 0;
	core_stats_.instrs_retired = 0;
}


VmBase::Stats& PipelinedCore::GetStats(){
	return core_stats_;
}

} // namespace rv5s