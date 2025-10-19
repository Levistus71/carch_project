#include "vm/rv5s/rv5s_vm.h"

void RV5SVM::RunSingleCycle(){
	ClearStop();
	uint64_t instruction_executed = 0;

	while (!stop_requested && program_counter_ < program_size_) {
		// this line mannn. why???
		if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
		    break;
		
        // Fetch
        instruction_deque.pop_back();
        InstrContext current_instruction{this->program_counter_};
        instruction_deque.push_front(current_instruction);
		Fetch();

		// Decode
        instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		Decode();

		// Execute
		instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		Execute();

		// MemoryAccess
		instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		MemoryAccess();

		// WriteBack
		instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		WriteBack();

		instruction_executed++;
		std::cout << "Program Counter: " << program_counter_ << std::endl;
	}

	if (program_counter_ >= program_size_) {
		std::cout << "VM_PROGRAM_END" << std::endl;
		output_status_ = "VM_PROGRAM_END";
	}

	DumpRegisters(globals::registers_dump_file_path, registers_);
	DumpState(globals::vm_state_dump_file_path);
}



void RV5SVM::DebugRunSingleCycle(){
    ClearStop();
    uint64_t instruction_executed = 0;
    while (!this->stop_requested && this->program_counter_ < this->program_size_) {
        if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
            break;
        
        if (std::find(breakpoints_.begin(), breakpoints_.end(), program_counter_) == breakpoints_.end()) {
            SingleCycleStep(false);
            unsigned int delay_ms = vm_config::config.getRunStepDelay();
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }        
        else {
            std::cout << " Breakpoint was hit. pc : " << program_counter_ << std::endl;
            output_status_ = "VM_BREAKPOINT_HIT";
            break;
        }
    }
    if (program_counter_ >= program_size_) {
        std::cout << "VM_PROGRAM_END" << std::endl;
        output_status_ = "VM_PROGRAM_END";
    }
    DumpRegisters(globals::registers_dump_file_path, registers_);
    DumpState(globals::vm_state_dump_file_path);
}


void RV5SVM::SingleCycleStep(bool dump){
    this->current_delta.old_pc = this->program_counter_;

    if (this->program_counter_ < this->program_size_) {

        this->current_delta.old_pc = this->program_counter_;

        InstrContext current_instruction{this->program_counter_};

        // Fetch
        instruction_deque.pop_back();
        InstrContext current_instruction{this->program_counter_};
        instruction_deque.push_front(current_instruction);
		DebugFetch();

		// Decode
        instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		DebugDecode();

		// Execute
		instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		DebugExecute();

		// MemoryAccess
		instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		DebugMemoryAccess();

		// WriteBack
		instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		DebugWriteBack();

        
        this->current_delta.new_pc = program_counter_;
        while(this->undo_stack.size()>=this->max_undo_stack_size){
            this->undo_stack.pop_back();
        }
        this->undo_stack.push_front(this->current_delta);

        // resetting current delta
        this->current_delta = StepDelta();
    }
    else{
        std::cout << "vm program end!" << std::endl;
        this->output_status_ = "VM_PROGRAM_END";
    }
    
    if(dump){
        std::cout << "Program Counter: " << program_counter_ << std::endl;
        std::cout << "Step completed!" << std::endl;
        this->output_status_ = "VM_STEP_COMPLETED";
        DumpRegisters(globals::registers_dump_file_path, registers_);
        DumpState(globals::vm_state_dump_file_path);
    }  
}



void RV5SVM::SingleCycleUndo(){
    if (this->undo_stack.empty()) {
        std::cout << "Cannot undo." << std::endl;
        output_status_ = "VM_NO_MORE_UNDO";
        return;
    }


    StepDelta last = this->undo_stack.front();
    this->undo_stack.pop_front();

    for (const auto &change : last.register_changes) {
        switch (change.reg_type) {
        case 0: { // GPR
            registers_.WriteGpr(change.reg_index, change.old_value);
            break;
        }
        case 1: { // CSR
            registers_.WriteCsr(change.reg_index, change.old_value);
            break;
        }
        case 2: { // FPR
            registers_.WriteFpr(change.reg_index, change.old_value);
            break;
        }
        default:std::cerr << "Invalid register type: " << change.reg_type << std::endl;
            break;
        }
    }

    for (const auto &change : last.memory_changes) {
        for (size_t i = 0; i < change.old_bytes_vec.size(); ++i) {
        memory_controller_.WriteByte(change.address + i, change.old_bytes_vec[i]);
        }
    }

    this->program_counter_ = last.old_pc;
    std::cout << "Program Counter: " << program_counter_ << std::endl;

    output_status_ = "Undo complete!";
    std::cout << "VM_UNDO_COMPLETED" << std::endl;

    DumpRegisters(globals::registers_dump_file_path, registers_);
    DumpState(globals::vm_state_dump_file_path);
}
