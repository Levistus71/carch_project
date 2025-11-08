#pragma once
#include "./data_bus.h"
#include <set>

namespace dual_issue{


class ROBBuffer{
public:
    struct ROBBufferEntry{
        bool ready_to_commit = false;
        DualIssueInstrContext instr;
    };

    ROBBuffer();

    size_t EmptySlots();
    bool HeadReady();

    size_t Reserve();
    void Push(DualIssueInstrContext instr);
    DualIssueInstrContext Top();
    void Pop();

    std::pair<bool, uint64_t> QueryVal(uint64_t rob_idx);

private:
    static constexpr size_t max_size = 8;

    size_t head = 0;
    size_t tail = 0;

    std::vector<ROBBufferEntry> buffer;

};

class ReorderBuffer{
public:
    size_t EmptySlots();

    void Pull(DualIssueCore& vm_core);
    void Push(DualIssueInstrContext& instr, DualIssueCore& vm_core);
    
    void Commit(DualIssueCore& vm_core);

    size_t Reserve();

    std::pair<bool, uint64_t> QueryVal(uint64_t rob_idx);
    
private:
    ROBBuffer buffer;

    void BroadCastMsgs(DualIssueInstrContext& instr, CommonDataBus& data_bus);
};

} // namespace dual_issue