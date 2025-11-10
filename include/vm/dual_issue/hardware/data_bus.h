#pragma once
#include "../core/instruction_context/instruction_context.h"

namespace dual_issue{

class CommonDataBus{
public:
    struct BroadCastMessage{
        uint64_t rob_idx;
        uint64_t value;
        bool clear_dependency = false;
        uint64_t epoch;

        BroadCastMessage(uint64_t rob_idx, uint64_t value, bool clear_dependency, uint64_t epoch): 
            rob_idx{rob_idx}, 
            value{value}, 
            clear_dependency{clear_dependency}, 
            epoch{epoch} 
        {}
    };

    std::deque<BroadCastMessage> broadcast_msgs;

    void Reset();

    void BroadCast(uint64_t rob_idx, uint64_t value, bool clear_dependency, uint64_t epoch);
};


} // namespace dual_issue