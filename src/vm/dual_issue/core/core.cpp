#include "vm/dual_issue/core/core.h"

namespace dual_issue
{

void DualIssueCore::PipelineRegInstrs::FlushPreIssueRegs(){
	if_id_1.illegal = true;
	if_id_2.illegal = true;

	id_issue_1.illegal = true;
	id_issue_2.illegal = true;
}

void DualIssueCore::PipelineRegInstrs::Reset(){
    if_id_1.illegal = true;
    if_id_2.illegal = true;

    id_issue_1.illegal = true;
    id_issue_2.illegal = true;

    rsrvstn_alu.illegal = true;
    rsrvstn_lsu.illegal = true;

    alu_commit.illegal = true;
    lsu_commit.illegal = true;
}


void DualIssueCore::FlushPreIssueRegs(){
	pipeline_reg_instrs_.FlushPreIssueRegs();
}



uint64_t DualIssueCore::GetProgramCounter() const{
    return pc;
}
void DualIssueCore::AddToProgramCounter(int64_t value){
    pc = static_cast<uint64_t>(static_cast<int64_t>(pc) + value);
}
void DualIssueCore::SetProgramCounter(uint64_t value){
    pc = value;
}


void DualIssueCore::Reset(){
    pipeline_reg_instrs_.Reset();

    pc = 0;

    branch_prediction_enabled_ = false;
	branch_prediction_static_ = false;


    debug_mode_ = true;
    undo_instruction_stack_.clear();
    max_undo_stack_size_ = 256;
    stop_requested_ = false;
    breakpoints_.clear();

    register_file_.Reset();
    memory_controller_.Reset();
    alu_que_.Reset();
    lsu_que_.Reset();
    broadcast_bus_.Reset();
    commit_buffer_.Reset();
    reg_status_file_.Reset();
    branch_predictor_.reset();

    program_size_ = 0;

	core_stats_.cycles = 0;
	core_stats_.instrs_retired = 0;
	core_stats_.branch_instrs = 0;
	core_stats_.branch_mispredicts = 0;
}


void DualIssueCore::Load(){
    Reset();

    // updating core state
	max_undo_stack_size_ = vm_config::config.getMaxUndoStackSize();

	std::vector<bool> t = vm_config::config.getBranchPredictionStatus();
	this->branch_prediction_enabled_ = t[0];
	this->branch_prediction_static_ = t[1];
}

void DualIssueCore::Load(AssembledProgram& program){
    Reset();

    // updating core state
	max_undo_stack_size_ = vm_config::config.getMaxUndoStackSize();

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

    
} // namespace dual_issue
