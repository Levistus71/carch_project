#include "vm/rv5s/rv5s_vm.h"

void RV5SVM::RunPipelined(){
    if(!this->hazard_detection_enabled){
        RunPipelinedWithoutHazardDetection(false);
    }
    else if(this->hazard_detection_enabled && !this->data_forwarding_enabled){
        RunPipelinedWithHazardWithoutForwarding(false);
    }
    else if(this->hazard_detection_enabled && this->data_forwarding_enabled){
        RunPipelinedWithHazardWithForwarding(false);
    }
}

void RV5SVM::DebugRunPipelined(){
    if(!this->hazard_detection_enabled){
        RunPipelinedWithoutHazardDetection(true);
    }
    else if(this->hazard_detection_enabled && !this->data_forwarding_enabled){
        RunPipelinedWithHazardWithoutForwarding(true);
    }
    else if(this->hazard_detection_enabled && this->data_forwarding_enabled){
        RunPipelinedWithHazardWithForwarding(true);
    }
}

void RV5SVM::RunPipelinedWithoutHazardDetection(bool debug_mode){
    ClearStop();
    uint64_t instruction_executed = 0;
    
    while (!stop_requested) {
        if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
            break;

        
        StepPipelined(debug_mode);

        if(this->program_counter_ >= this->program_size_){
            if(GetIdInstruction().nopped && GetExInstruction().nopped && GetMemInstruction().nopped && GetWbInstruction().nopped)
                break;
        }
    
        instruction_executed++;
        std::cout << "Program Counter: " << program_counter_ << std::endl;
    }
    
    std::cout << "Vm: the loaded program has ended!" << std::endl;
    output_status_ = "VM_PROGRAM_END";
    
    DumpRegisters(globals::registers_dump_file_path, registers_);
    DumpState(globals::vm_state_dump_file_path);
}

/*

while loop start:
    1 2 3 4 5 -> the write back of 5 will be over

    pop 5, insert 0 into the pipeline
    0 1 2 3 4 
    now AFTER fetching 0, writing back 4 and decoding 1, we check if 1 collides with 2 / 3
    DECODE can happen AFTER writing back of 4, not necessarily last.
    HAZARD DETECTION should happen at the end though

*/
void RV5SVM::RunPipelinedWithHazardWithoutForwarding(bool debug_mode){
    ClearStop();
    uint64_t instruction_executed = 0;
    
    while (!stop_requested) {
        if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
            break;


        StepPipelined(debug_mode);

        if(this->program_counter_ >= this->program_size_){
            if(GetIdInstruction().nopped && GetExInstruction().nopped && GetMemInstruction().nopped && GetWbInstruction().nopped)
                break;
        }
        
        if(DetectControlHazard()){
            HandleControlHazard();
        }

        if(DetectDataHazardWithoutForwarding()){
            HandleDataHazard();
        }

        instruction_executed++;
        std::cout << "Program Counter: " << program_counter_ << std::endl;
    }
    
    std::cout << "Vm: the loaded program has ended!" << std::endl;
    output_status_ = "VM_PROGRAM_END";
    
    DumpRegisters(globals::registers_dump_file_path, registers_);
    DumpState(globals::vm_state_dump_file_path);
}


void RV5SVM::RunPipelinedWithHazardWithForwarding(bool debug_mode){
    ClearStop();
    uint64_t instruction_executed = 0;
       
    while (!stop_requested) {
        if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
            break;
        
        
        StepPipelined(debug_mode);

        if(this->program_counter_ >= this->program_size_){
            if(GetIdInstruction().nopped && GetExInstruction().nopped && GetMemInstruction().nopped && GetWbInstruction().nopped)
                break;
        }

        if(DetectControlHazard()){
            HandleControlHazard();
        }
        
        if(DetectDataHazardWithForwarding()){
            HandleDataHazard();
        }
    
        instruction_executed++;
        std::cout << "Program Counter: " << program_counter_ << std::endl;
    }
    
    std::cout << "Vm: the loaded program has ended!" << std::endl;
    output_status_ = "VM_PROGRAM_END";
    
    DumpRegisters(globals::registers_dump_file_path, registers_);
    DumpState(globals::vm_state_dump_file_path);
}


void RV5SVM::StepPipelined(bool debug_mode){    
    // Popping the last 'cycle' wb
    this->instruction_deque.pop_back();


    if(debug_mode){
        InstrContext retired_instruction = instruction_deque.back();
        this->instruction_deque.pop_back();
        while(this->undo_instruction_stack.size() >= this->max_undo_stack_size){
            undo_instruction_stack.pop_back();
        }
        this->undo_instruction_stack.push_front(retired_instruction);
    }

    // Pushing a new instruction infront of the instruction deque
    if(this->program_counter_ < this->program_size_)
        this->instruction_deque.push_front(InstrContext{this->program_counter_});
    else{
        AddToProgramCounter(-4);   // so that the nop we insert now doesn't cause any probs in fetch
        InstrContext nop;
        nop.nopify();
        this->instruction_deque.push_front(nop);
    }

    // Fetch
    Fetch();

    /**
     * WriteBack() happens before decode, following "write first"
     */
    // WriteBack
    WriteBack(debug_mode);

    // Decode
    Decode(debug_mode);

    // Execute
    Execute();

    // MemoryAccess
    MemoryAccess(debug_mode);
}



void RV5SVM::PipelinedUndo(){
    if (this->undo_instruction_stack.empty()) {
        std::cout << "Cannot undo." << std::endl;
        output_status_ = "VM_NO_MORE_UNDO";
        return;
    }

    InstrContext last_instruction = undo_instruction_stack.front();
    this->undo_instruction_stack.pop_front();

    InstrContext curr_if_instruction = GetIfInstruction();
    InstrContext curr_id_instruction = GetIdInstruction();
    InstrContext curr_ex_instruction = GetExInstruction();
    InstrContext curr_mem_instruction = GetMemInstruction();
    InstrContext curr_wb_instruction = GetWbInstruction();

    // Throwing the if_instruction
    this->program_counter_ = curr_if_instruction.pc;
    this->instruction_deque.pop_front();


    // Changes in wb stage:
    if(curr_wb_instruction.reg_write){
        if(curr_wb_instruction.fcsr_update){
            this->registers_.WriteCsr(0x003, curr_wb_instruction.fcsr_status);
        }
        if(curr_wb_instruction.csr_op){
            this->registers_.WriteCsr(curr_wb_instruction.csr_rd, curr_wb_instruction.csr_overwritten);
            this->registers_.WriteGpr(curr_wb_instruction.rd, curr_wb_instruction.reg_overwritten);
        }
        else if(curr_wb_instruction.reg_write_to_fpr){
            this->registers_.WriteFpr(curr_wb_instruction.rd, curr_wb_instruction.reg_overwritten);
        }
        else{
            this->registers_.WriteGpr(curr_wb_instruction.rd, curr_wb_instruction.reg_overwritten);
        }
    }

    // Changes in mem stage:
    if(curr_mem_instruction.mem_write){
        for(size_t i=0;i<curr_mem_instruction.mem_access_bytes;i++){
            this->memory_controller_.WriteByte(curr_mem_instruction.alu_out, curr_mem_instruction.mem_overwritten[i]);
        }
    }
    else{
        curr_mem_instruction.mem_out = 0;
    }

    // Changes in the exec stage:
    curr_ex_instruction.alu_out = 0;

    // Changes in the id stage:
    curr_id_instruction.reset_id_vars();

    // Pushing last_instruction into the queue
    this->instruction_deque.push_back(last_instruction);

    std::cout << "Program Counter: " << program_counter_ << std::endl;

    output_status_ = "Undo complete!";
    std::cout << "VM_UNDO_COMPLETED" << std::endl;

    DumpRegisters(globals::registers_dump_file_path, registers_);
    DumpState(globals::vm_state_dump_file_path);
}
