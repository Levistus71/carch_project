#include "vm/rv5s/single_cycle/executor/executor.h"

namespace rv5s{

void SingleCycleExecutor::RunSingleCycle(SingleCycleCore& vm_core){
	vm_core.ClearStop();
	uint64_t instruction_executed = 0;

	while (!vm_core.stop_requested_ && vm_core.program_counter_ < vm_core.program_size_) {
		if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
		    break;
		
        StepSingleCycle(vm_core, false);

		instruction_executed++;
		globals::vm_cout_file << "Program Counter: " << vm_core.program_counter_ << std::endl;
	}

	if (vm_core.program_counter_ >= vm_core.program_size_) {
		globals::vm_cout_file << "Vm : Program Has Ended!" << std::endl;
	}
}


void SingleCycleExecutor::DebugRunSingleCycle(SingleCycleCore& vm_core){
    vm_core.ClearStop();

    uint64_t instruction_executed = 0;
    while (!vm_core.stop_requested_ && vm_core.program_counter_ < vm_core.program_size_) {
        if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
            break;
        
        if (std::find(vm_core.breakpoints_.begin(), vm_core.breakpoints_.end(), vm_core.program_counter_) == vm_core.breakpoints_.end()) {
            StepSingleCycle(vm_core, false);
            unsigned int delay_ms = vm_config::config.getRunStepDelay();
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }        
        else {
            globals::vm_cout_file << " Breakpoint was hit. pc : " << vm_core.program_counter_ << std::endl;
            break;
        }
    }
    if (vm_core.program_counter_ >= vm_core.program_size_) {
        globals::vm_cout_file << "VM : Program Has Ended" << std::endl;
    }

    // DumpRegisters(globals::registers_dump_file_path, registers_);
    // DumpState(globals::vm_state_dump_file_path);
}



void SingleCycleExecutor::StepSingleCycle(SingleCycleCore& vm_core, bool dump){

    if (vm_core.program_counter_ < vm_core.program_size_) {

        // Fetch
		SingleCycleStages::Fetch(vm_core);

		// Decode
		SingleCycleStages::Decode(vm_core);

		// Execute
		SingleCycleStages::Execute(vm_core);

		// MemoryAccess
		SingleCycleStages::MemoryAccess(vm_core);

		// WriteBack
		SingleCycleStages::WriteBack(vm_core);

        // Debug Store
        if(vm_core.debug_mode_){
    
            while(vm_core.undo_instruction_stack_.size()>=vm_core.max_undo_stack_size_){
                vm_core.undo_instruction_stack_.pop_back();
            }
            vm_core.undo_instruction_stack_.push_front(vm_core.instr);
        }

    }
    else{
        if(dump){
            globals::vm_cout_file << "Vm : Program Has ended!" << std::endl;
            return;
        }
    }
    
    if(dump){
        globals::vm_cout_file << "Program Counter: " << vm_core.program_counter_ << std::endl;
        globals::vm_cout_file << "Step completed!" << std::endl;
        // DumpRegisters(globals::registers_dump_file_path, registers_);
        // DumpState(globals::vm_state_dump_file_path);
    }  

}


void SingleCycleExecutor::UndoSingleCycle(SingleCycleCore& vm_core){
    if (vm_core.undo_instruction_stack_.empty()) {
        globals::vm_cout_file << "VM : Cannot undo." << std::endl;
        return;
    }

    SingleCycleInstrContext last_instruction = vm_core.undo_instruction_stack_.front();
    vm_core.undo_instruction_stack_.pop_front();

    // Changes in wb stage:
    if(last_instruction.reg_write){
        if(last_instruction.fcsr_update){
            vm_core.register_file_.WriteCsr(0x003, last_instruction.fcsr_status);
        }
        if(last_instruction.csr_op){
            vm_core.register_file_.WriteCsr(last_instruction.csr_rd, last_instruction.csr_overwritten);
            vm_core.register_file_.WriteGpr(last_instruction.rd, last_instruction.reg_overwritten);
        }
        else if(last_instruction.reg_write_to_fpr){
            vm_core.register_file_.WriteFpr(last_instruction.rd, last_instruction.reg_overwritten);
        }
        else{
            vm_core.register_file_.WriteGpr(last_instruction.rd, last_instruction.reg_overwritten);
        }
    }

    // Changes in mem Stage:
    if(last_instruction.mem_write){
        for(size_t i=0;i<last_instruction.mem_access_bytes;i++){
            vm_core.memory_controller_.WriteByte(last_instruction.alu_out, last_instruction.mem_overwritten[i]);
        }
    }

    vm_core.program_counter_ = last_instruction.pc;

    globals::vm_cout_file << "Program Counter: " << vm_core.program_counter_ << std::endl;
    globals::vm_cout_file << "VM : Undo Complete!" << std::endl;

    // DumpRegisters(globals::register_file_dump_file_path, registers_);
    // DumpState(globals::vm_state_dump_file_path);
}
} // namespace rv5s
