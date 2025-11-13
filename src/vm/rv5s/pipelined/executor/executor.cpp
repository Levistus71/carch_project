#include "vm/rv5s/pipelined/executor/executor.h"
#include "sim_state.h"

// anonymous namespace
namespace {

/**
 * Common functions
 */
void InsertIfInstruction(rv5s::PipelinedCore& vm_core){
    // Pushing a new instruction infront of the instruction deque
    if(vm_core.program_counter_ < vm_core.program_size_)
        vm_core.instruction_deque_.push_front(rv5s::PipelinedInstrContext{vm_core.program_counter_});
    else{
        vm_core.AddToProgramCounter(4);   // so that the nop we insert now doesn't cause any probs in fetch
        rv5s::PipelinedInstrContext nop;
        nop.nopify();
        nop.bubbled = true;
        vm_core.instruction_deque_.push_front(nop);
    }
}


void PopWbInstruction(rv5s::PipelinedCore& vm_core){
    if(vm_core.debug_mode_){
        rv5s::PipelinedInstrContext retired_instruction = vm_core.instruction_deque_.back();
        vm_core.instruction_deque_.pop_back();
        while(vm_core.undo_instruction_stack_.size() >= vm_core.max_undo_stack_size_){
            vm_core.undo_instruction_stack_.pop_back();
        }
        vm_core.undo_instruction_stack_.push_front(retired_instruction);
    }
    else{
        vm_core.instruction_deque_.pop_back();
    }
}

void DrivePipeline(rv5s::PipelinedCore& vm_core){
    // Fetch
    rv5s::PipelinedStages::Fetch(vm_core);

    /**
     * WriteBack() happens before decode, following "write first"
     */
    // WriteBack
    rv5s::PipelinedStages::WriteBack(vm_core);

    // Decode
    rv5s::PipelinedStages::Decode(vm_core);

    // Execute
    rv5s::PipelinedStages::Execute(vm_core);

    // MemoryAccess
    rv5s::PipelinedStages::MemoryAccess(vm_core);
}



/**
 * Pipeline without hazard detection and forwarding
 */
void StepPipelinedNoHazard(rv5s::PipelinedCore& vm_core){
    // FIXME: stop when the program is over
    if(vm_core.program_counter_ >= vm_core.program_size_){
        if(vm_core.GetIdInstruction().nopped && vm_core.GetExInstruction().nopped && vm_core.GetMemInstruction().nopped && vm_core.GetWbInstruction().nopped){
            globals::vm_cout_file << "Cannot step further." << std::endl;
            return;
        }
    }

    InsertIfInstruction(vm_core);

    PopWbInstruction(vm_core);

    DrivePipeline(vm_core);
}

void RunPipelinedNoHazard(rv5s::PipelinedCore& vm_core){
    vm_core.ClearStop();
    uint64_t instruction_executed = 0;
    
    while (!vm_core.stop_requested_) {
        if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
            break;

        
        StepPipelinedNoHazard(vm_core);

        // FIXME:
        if(vm_core.program_counter_ >= vm_core.program_size_){
            if(vm_core.GetIdInstruction().nopped && vm_core.GetExInstruction().nopped && vm_core.GetMemInstruction().nopped && vm_core.GetWbInstruction().nopped)
                break;
        }
    
        instruction_executed++;
        globals::vm_cout_file << "Program Counter: " << vm_core.program_counter_ << std::endl;
    }
    
    globals::vm_cout_file << "Vm: the loaded program has ended!" << std::endl;
    
    // DumpRegisters(globals::registers_dump_file_path, registers_);
    // DumpState(globals::vm_state_dump_file_path);
}



/**
 * Pipelined With hazard detection
 */
void StepPipelinedWithHazard(rv5s::PipelinedCore& vm_core){
    // FIXME: do this properly
    if(vm_core.program_counter_ >= vm_core.program_size_){
        if(vm_core.GetIdInstruction().nopped && vm_core.GetExInstruction().nopped && vm_core.GetMemInstruction().nopped && vm_core.GetWbInstruction().nopped){
            globals::vm_cout_file << "Cannot step further." << std::endl;
            return;
        }
    }

    static bool data_hazard_detected = false;
    if(data_hazard_detected){
        vm_core.hazard_detector_.HandleDataHazard(vm_core);
    }
    else{
        InsertIfInstruction(vm_core);
        PopWbInstruction(vm_core);
    }

    DrivePipeline(vm_core);

    // FIXME:
    if(vm_core.program_counter_ >= vm_core.program_size_){
        if(vm_core.GetIdInstruction().nopped && vm_core.GetExInstruction().nopped && vm_core.GetMemInstruction().nopped && vm_core.GetWbInstruction().nopped)
            return;
    }
    
    if(vm_core.hazard_detector_.DetectControlHazard(vm_core)){
        vm_core.hazard_detector_.HandleControlHazard(vm_core);
    }

    data_hazard_detected = vm_core.hazard_detector_.DetectDataHazard(vm_core);
}
void RunPipelinedWithHazard(rv5s::PipelinedCore& vm_core){
    vm_core.ClearStop();
    uint64_t instruction_executed = 0;
    
    while (!vm_core.stop_requested_) {
        if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
            break;

        StepPipelinedWithHazard(vm_core);

        // FIXME:
        if(vm_core.program_counter_ >= vm_core.program_size_){
            if(vm_core.GetIdInstruction().nopped && vm_core.GetExInstruction().nopped && vm_core.GetMemInstruction().nopped && vm_core.GetWbInstruction().nopped)
                break;
        }

        instruction_executed++;
        globals::vm_cout_file << "Program Counter: " << vm_core.program_counter_ << std::endl;
    }
    
    globals::vm_cout_file << "Vm: the loaded program has ended!" << std::endl;
    
    // DumpRegisters(globals::registers_dump_file_path, registers_);
    // DumpState(globals::vm_state_dump_file_path);
}
}


namespace rv5s{

void PipelinedExecutor::RunPipelined(PipelinedCore& vm_core){
    if(!vm_core.hazard_detection_enabled_){
        RunPipelinedNoHazard(vm_core);
    }
    else{
        RunPipelinedWithHazard(vm_core);
    }
}

void PipelinedExecutor::StepPipelined(PipelinedCore& vm_core){
    if(!vm_core.hazard_detection_enabled_){
        StepPipelinedNoHazard(vm_core);
    }
    else{
        StepPipelinedWithHazard(vm_core);
    }
}

void PipelinedExecutor::DebugRunPipelined(PipelinedCore& vm_core){
    vm_core.ClearStop();

    uint64_t instruction_executed = 0;
    while (!vm_core.stop_requested_ && vm_core.program_counter_ < vm_core.program_size_) {
        if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
            break;
        
        // the program stops when the if instruction hits a breakpoint
        if (std::find(vm_core.breakpoints_.begin(), vm_core.breakpoints_.end(), vm_core.GetIfInstruction().pc) == vm_core.breakpoints_.end()) {
            StepPipelined(vm_core);
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
}

void PipelinedExecutor::UndoPipelined(PipelinedCore& vm_core){
    if (vm_core.undo_instruction_stack_.empty()) {
        globals::vm_cout_file << "Cannot undo." << std::endl;
        return;
    }

    PipelinedInstrContext last_instruction = vm_core.undo_instruction_stack_.front();
    vm_core.undo_instruction_stack_.pop_front();

    // not by reference, might get popped
    PipelinedInstrContext curr_if_instruction_copy = vm_core.GetIfInstruction();
    PipelinedInstrContext curr_ex_instruction_copy = vm_core.GetExInstruction();
    PipelinedInstrContext curr_mem_instruction_copy = vm_core.GetMemInstruction();
    PipelinedInstrContext curr_wb_instruction_copy = vm_core.GetWbInstruction();

    // Throwing the dirty instruction (if the ex instruction was bubbled, then we pop the ex bubble)
    vm_core.program_counter_ = curr_if_instruction_copy.pc;
    if(!curr_ex_instruction_copy.bubbled){
        vm_core.instruction_deque_.pop_front();
    }
    else{
        vm_core.instruction_deque_.pop_back();
        vm_core.instruction_deque_.pop_back();
        vm_core.instruction_deque_.pop_back();
        vm_core.instruction_deque_.push_back(curr_mem_instruction_copy);
        vm_core.instruction_deque_.push_back(curr_wb_instruction_copy);
    }

    // Changes in wb stage:
    if(curr_wb_instruction_copy.reg_write){
        if(curr_wb_instruction_copy.fcsr_update){
            vm_core.register_file_.WriteCsr(0x003, curr_wb_instruction_copy.fcsr_status);
        }
        if(curr_wb_instruction_copy.csr_op){
            vm_core.register_file_.WriteCsr(curr_wb_instruction_copy.csr_rd, curr_wb_instruction_copy.csr_overwritten);
            vm_core.register_file_.WriteGpr(curr_wb_instruction_copy.rd, curr_wb_instruction_copy.reg_overwritten);
        }
        else if(curr_wb_instruction_copy.reg_write_to_fpr){
            vm_core.register_file_.WriteFpr(curr_wb_instruction_copy.rd, curr_wb_instruction_copy.reg_overwritten);
        }
        else{
            vm_core.register_file_.WriteGpr(curr_wb_instruction_copy.rd, curr_wb_instruction_copy.reg_overwritten);
        }
    }

    // Changes in mem stage:
    if(curr_mem_instruction_copy.mem_write){
        for(size_t i=0;i<curr_mem_instruction_copy.mem_access_bytes;i++){
            vm_core.memory_controller_.WriteByte(curr_mem_instruction_copy.alu_out, curr_mem_instruction_copy.mem_overwritten[i]);
        }
    }

    // Pushing last_instruction into the queue
    vm_core.instruction_deque_.push_back(last_instruction);

    // new if instruction == old id instrucion (if no bubble) OR new if instruction == old if instruction (if bubble).
    // resetting either one doesn't matter
    // same logic appies to id instruction
    PipelinedInstrContext& new_if_instruction = vm_core.GetIfInstruction();
    new_if_instruction.reset_id_vars();
    PipelinedInstrContext& new_id_instruction = vm_core.GetIdInstruction();
    new_id_instruction.alu_out = 0;
    PipelinedInstrContext& new_ex_instruction = vm_core.GetExInstruction();
    new_ex_instruction.mem_out = 0;


    globals::vm_cout_file << "Program Counter: " << vm_core.program_counter_ << std::endl;
    globals::vm_cout_file << "VM : Undo Complete!" << std::endl;

    // DumpRegisters(globals::registers_dump_file_path, registers_);
    // DumpState(globals::vm_state_dump_file_path);
}

} // namespace rv5s