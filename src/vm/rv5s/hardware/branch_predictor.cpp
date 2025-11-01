#include "vm/rv5s/hardware/branch_predictor.h"


namespace rv5s
{

std::pair<bool, uint64_t> BranchPredictor::static_predict(uint64_t branch_instruction_address){
    uint8_t index = (branch_instruction_address >> sll_pc) & btb_index_mask;
    BTBValue& btb_value = branch_target_buffer[index];

    if(!btb_value.valid) return {false, branch_instruction_address + 4};

    // target_address is less than current address, branch backward (loops)
    if(btb_value.target_address < branch_instruction_address){
        return {true, btb_value.target_address};
    }
    else{
        return {false, branch_instruction_address + 4};
    }
}

std::pair<bool, uint64_t> BranchPredictor::dynamic_predict(uint64_t branch_instruction_address){
    uint8_t index = (branch_instruction_address >> sll_pc) & btb_index_mask;
    BTBValue& btb_value = branch_target_buffer[index];

    if(!btb_value.valid) return {false, branch_instruction_address + 4};

    if(btb_value.take_branch){
        return {true, btb_value.target_address};
    }
    else{
        return {false, branch_instruction_address + 4};
    }
}


void BranchPredictor::update_btb(uint64_t branch_instruction_address, bool branch_taken, uint64_t target_address){
    uint8_t index = (branch_instruction_address >> sll_pc) & btb_index_mask;
    BTBValue& btb_value = branch_target_buffer[index];

    if(!btb_value.valid){
        btb_value.valid = true;
    }

    btb_value.take_branch = branch_taken;
    btb_value.target_address = target_address;
}

} // namespace rv5s