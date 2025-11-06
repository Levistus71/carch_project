#pragma once

#include <vector>
#include <bitset>


namespace rv5s {

class BranchPredictor {
private:
    struct BTBValue{
        bool valid = false;
        uint64_t tag;
        uint64_t target_address;
        bool take_branch;
    };

    size_t btb_size = 256;
    int btb_index_mask = 0xFF;
    int sll_pc = 2; // last 2 bits of pc is always 0

    std::vector<BTBValue> branch_target_buffer;
    
public:

    BranchPredictor() : branch_target_buffer{btb_size} {}

    std::pair<bool, uint64_t> static_predict(uint64_t branch_instruction_address);  

    std::pair<bool, uint64_t> dynamic_predict(uint64_t branch_instruction_address);

    void update_btb(uint64_t branch_instruction_address, bool branch_taken, uint64_t target_address);

    void reset();
};

} // namespace rv5s