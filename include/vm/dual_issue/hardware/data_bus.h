#pragma once
#include "../core/instruction_context/instruction_context.h"

namespace dual_issue{

class CommonDataBus{
public:
    struct BroadCastMessage{
        uint64_t rob_idx;
        uint64_t value;

        BroadCastMessage(uint64_t rob_idx, uint64_t value) : rob_idx{rob_idx}, value{value} {}
    };

    std::deque<BroadCastMessage> broadcast_msgs;

    void Reset();

    void BroadCast(uint64_t rob_idx, uint64_t value);
};


} // namespace dual_issue