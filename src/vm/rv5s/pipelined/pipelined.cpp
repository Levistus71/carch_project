#include "vm/rv5s/rv5s_vm.h"

// TODO: SIMULATE PIPELINING WITH MULTITHREADING
// TODO: pipelining in debug mode

void RV5SVM::RunPipelined(){
    if(!this->hazard_detection_enabled){
        RunPipelinedWithoutHazardDetection();
    }
    else if(this->hazard_detection_enabled && !this->data_forwarding_enabled){
        RunPipelinedWithHazardWithoutForwarding();
    }
    else if(this->hazard_detection_enabled && this->data_forwarding_enabled){
        RunPipelinedWithHazardWithForwarding();
    }
}

void RV5SVM::RunPipelinedWithoutHazardDetection(){
    ClearStop();
    uint64_t instruction_executed = 0;
    
    while (!stop_requested) {
        // this line mannn. why???
        if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
            break;
    
        // Popping the last 'cycle' wb
        this->instruction_deque.pop_back();

        // Pushing a new instruction infront of the instruction deque
        if(this->program_counter_ < this->program_size_)
            this->instruction_deque.push_front(InstrContext{this->program_counter_});
        else{
            UpdateProgramCounter(-4);   // so that the nop we insert now doesn't cause any probs
            InstrContext nop;
            nop.nopify();
            this->instruction_deque.push_front(nop);

            if(GetIdInstruction().nopped && GetExInstruction().nopped && GetMemInstruction().nopped && GetWbInstruction().nopped)
                break;
        }
    
        // Fetch
        Fetch();
    
        /**
         * WriteBack() happens before decode, following "write first"
         */
        // WriteBack
        WriteBack();

        // Decode
        Decode();
    
        // Execute
        Execute();

        // MemoryAccess
        MemoryAccess();
    
        instruction_executed++;
        std::cout << "Program Counter: " << program_counter_ << std::endl;
    }
    
    if (program_counter_ >= program_size_) {
        std::cout << "Vm: the loaded program has ended!" << std::endl;
        output_status_ = "VM_PROGRAM_END";
    }
    
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
void RV5SVM::RunPipelinedWithHazardWithoutForwarding(){
    ClearStop();
    uint64_t instruction_executed = 0;
    
    while (!stop_requested) {
        // this line mannn. why???
        if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
            break;
        
        // popping previous cycle's wb_instruction
        this->instruction_deque.pop_back();
        
        // inserting a new instruction into the pipeline
        if(this->program_counter_ < this->program_size_)
            this->instruction_deque.push_front(InstrContext{this->program_counter_});
        else{
            UpdateProgramCounter(-4);   // so that the nop we insert now doesn't cause any problems
            InstrContext nop;
            nop.nopify();
            this->instruction_deque.push_front(nop);

            if(GetIdInstruction().nopped && GetExInstruction().nopped && GetMemInstruction().nopped && GetWbInstruction().nopped)
                break;
        }
        
        // Fetch
        Fetch();
        
        /**
         * WriteBack() happens before decode, following "write first"
         */
        // WriteBack
        WriteBack();
        
        // Decode
        Decode();
        
        // Execute
        Execute();
        
        // MemoryAccess
        MemoryAccess();
        
        if(DetectDataHazardWithoutForwarding()){
            UpdateProgramCounter(-4);

            std::deque<InstrContext> new_instruction_deque;
            new_instruction_deque.push_back(GetIfInstruction());
            new_instruction_deque.push_back(GetIdInstruction());
            InstrContext bubble;
            bubble.nopify();
            new_instruction_deque.push_back(bubble);
            new_instruction_deque.push_back(GetExInstruction());
            new_instruction_deque.push_back(GetMemInstruction());

            this->instruction_deque = new_instruction_deque;
        }

        instruction_executed++;
        std::cout << "Program Counter: " << program_counter_ << std::endl;
    }
    
    if (program_counter_ >= program_size_) {
        std::cout << "Vm: the loaded program has ended!" << std::endl;
        output_status_ = "VM_PROGRAM_END";
    }
    
    DumpRegisters(globals::registers_dump_file_path, registers_);
    DumpState(globals::vm_state_dump_file_path);
}



/*

while loop start:
    1 2 3 4 5 -> the write back of 5 will be over

    pop 5, insert 0 into the pipeline
    0 1 2 3 4 
    now AFTER fetching 0, writing back 4, executing 2, accessing memory by 3 and decoding 1, we check if 1 collides with 2 / 3
    DECODE AND HAZARD DETECTION HAPPEN LAST

*/
void RV5SVM::RunPipelinedWithHazardWithForwarding(){
    ClearStop();
    uint64_t instruction_executed = 0;
    
    while (!stop_requested) {
        // this line mannn. why???
        if (instruction_executed > vm_config::config.getInstructionExecutionLimit())
            break;
        
        // popping previous cycle's wb_instruction
        this->instruction_deque.pop_back();
        // inserting a new instruction into the pipeline
        if(this->program_counter_ < this->program_size_)
            this->instruction_deque.push_front(InstrContext{this->program_counter_});
        else{
            UpdateProgramCounter(-4);   // so that the nop we insert now doesn't cause any problems
            InstrContext nop;
            nop.nopify();
            this->instruction_deque.push_front(nop);

            if(GetIdInstruction().nopped && GetExInstruction().nopped && GetMemInstruction().nopped && GetWbInstruction().nopped)
                break;
        }
        
        // Fetch
        Fetch();
    
        // WriteBack
        WriteBack();
        
        // Execute
        Execute();
        
        // MemoryAccess
        MemoryAccess();

        // Decode
        Decode();

        if(DetectDataHazardWithForwarding()){
            UpdateProgramCounter(-4);

            std::deque<InstrContext> new_instruction_deque;
            new_instruction_deque.push_back(GetIfInstruction());
            new_instruction_deque.push_back(GetIdInstruction());
            InstrContext bubble;
            bubble.nopify();
            new_instruction_deque.push_back(bubble);
            new_instruction_deque.push_back(GetExInstruction());
            new_instruction_deque.push_back(GetMemInstruction());

            this->instruction_deque = new_instruction_deque;
        }
    
        instruction_executed++;
        std::cout << "Program Counter: " << program_counter_ << std::endl;
    }
    
    if (program_counter_ >= program_size_) {
        std::cout << "Vm: the loaded program has ended!" << std::endl;
        output_status_ = "VM_PROGRAM_END";
    }
    
    DumpRegisters(globals::registers_dump_file_path, registers_);
    DumpState(globals::vm_state_dump_file_path);
}
