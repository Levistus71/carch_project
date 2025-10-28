#include "vm/rv5s/rv5s_vm.h"

void RV5SVM::RunSingleCycle(){
	ClearStop();
	uint64_t instruction_executed = 0;

	while (!stop_requested && program_counter_ < program_size_) {
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
		Decode(false);

		// Execute
		instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		Execute();

		// MemoryAccess
		instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		MemoryAccess(false);

		// WriteBack
		instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		WriteBack(false);

		instruction_executed++;
		globals::vm_cout_file << "Program Counter: " << program_counter_ << std::endl;
	}

	if (program_counter_ >= program_size_) {
		globals::vm_cout_file << "VM_PROGRAM_END" << std::endl;
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
            globals::vm_cout_file << " Breakpoint was hit. pc : " << program_counter_ << std::endl;
            output_status_ = "VM_BREAKPOINT_HIT";
            break;
        }
    }
    if (program_counter_ >= program_size_) {
        globals::vm_cout_file << "VM_PROGRAM_END" << std::endl;
        output_status_ = "VM_PROGRAM_END";
    }
    DumpRegisters(globals::registers_dump_file_path, registers_);
    DumpState(globals::vm_state_dump_file_path);
}


void RV5SVM::SingleCycleStep(bool dump){

    if (this->program_counter_ < this->program_size_) {


        // Fetch
        instruction_deque.pop_back();
        InstrContext current_instruction{this->program_counter_};
        instruction_deque.push_front(current_instruction);
		Fetch();

		// Decode
        instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		Decode(true);

		// Execute
		instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		Execute();

		// MemoryAccess
		instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		MemoryAccess(true);

		// WriteBack
		instruction_deque.pop_back();
        instruction_deque.push_front(InstrContext{});
		WriteBack(true);

        // get updated current_instruction
        current_instruction = instruction_deque.back();

        while(this->undo_instruction_stack.size()>=this->max_undo_stack_size){
            this->undo_instruction_stack.pop_back();
        }
        this->undo_instruction_stack.push_front(current_instruction);

    }
    else{
        globals::vm_cout_file << "vm program end!" << std::endl;
        this->output_status_ = "VM_PROGRAM_END";
    }
    
    if(dump){
        globals::vm_cout_file << "Program Counter: " << program_counter_ << std::endl;
        globals::vm_cout_file << "Step completed!" << std::endl;
        this->output_status_ = "VM_STEP_COMPLETED";
        DumpRegisters(globals::registers_dump_file_path, registers_);
        DumpState(globals::vm_state_dump_file_path);
    }  
}



void RV5SVM::SingleCycleUndo(){
    if (this->undo_instruction_stack.empty()) {
        globals::vm_cout_file << "Cannot undo." << std::endl;
        output_status_ = "VM_NO_MORE_UNDO";
        return;
    }

    InstrContext last_instruction = undo_instruction_stack.front();
    this->undo_instruction_stack.pop_front();

    // Changes in wb stage:
    if(last_instruction.reg_write){
        if(last_instruction.fcsr_update){
            this->registers_.WriteCsr(0x003, last_instruction.fcsr_status);
        }
        if(last_instruction.csr_op){
            this->registers_.WriteCsr(last_instruction.csr_rd, last_instruction.csr_overwritten);
            this->registers_.WriteGpr(last_instruction.rd, last_instruction.reg_overwritten);
        }
        else if(last_instruction.reg_write_to_fpr){
            this->registers_.WriteFpr(last_instruction.rd, last_instruction.reg_overwritten);
        }
        else{
            this->registers_.WriteGpr(last_instruction.rd, last_instruction.reg_overwritten);
        }
    }

    // Changes in mem Stage:
    if(last_instruction.mem_write){
        for(size_t i=0;i<last_instruction.mem_access_bytes;i++){
            this->memory_controller_.WriteByte(last_instruction.alu_out, last_instruction.mem_overwritten[i]);
        }
    }

    this->program_counter_ = last_instruction.pc;
    globals::vm_cout_file << "Program Counter: " << program_counter_ << std::endl;

    output_status_ = "Undo complete!";
    globals::vm_cout_file << "VM_UNDO_COMPLETED" << std::endl;

    DumpRegisters(globals::registers_dump_file_path, registers_);
    DumpState(globals::vm_state_dump_file_path);
}
