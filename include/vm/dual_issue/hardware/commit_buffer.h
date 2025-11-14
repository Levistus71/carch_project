#pragma once
#include "./data_bus.h"
#include <set>

namespace dual_issue{

class DualIssueCore;

class ROBBuffer{
public:
    struct ROBBufferEntry{
        bool ready_to_commit = false;
        DualIssueInstrContext instr;
        size_t epoch_number;
    };

    ROBBuffer();
    ROBBuffer(size_t slots);

    size_t EmptySlots();
    bool HeadReady();

    std::pair<size_t, size_t> Reserve();
    bool Push(DualIssueInstrContext instr);
    DualIssueInstrContext Top();
    void Pop();

    std::tuple<bool, uint64_t, uint64_t> QueryVal(uint64_t rob_idx);

    void Reset();

    std::vector<std::unique_ptr<const InstrContext>> GetInstrs();
    std::vector<bool> GetStatus();
    std::pair<size_t, size_t> GetHeadTail();

    void ResetTailTillIdx(size_t new_head, DualIssueCore& vm_core);

private:
    // most optimal is 16 (4 + 4 from reservation stations, 8 in 4 pipeline registers (each register has 2)) for dual issue
    // optimal is 24 for triple, we go with 32 instead
    size_t max_size;

    size_t head = 0;
    size_t tail = 0;

    std::vector<ROBBufferEntry> buffer;

    size_t epoch_counter = 0;

    bool InLimits(size_t idx);

};

class ReorderBuffer{
public:
    ReorderBuffer(){}
    ReorderBuffer(size_t slots) : buffer(slots) {}

    size_t EmptySlots();

    void Pull(DualIssueCore& vm_core);
    void Push(DualIssueInstrContext& instr, DualIssueCore& vm_core);
    
    void Commit(DualIssueCore& vm_core);

    std::pair<size_t, size_t> Reserve();

    std::tuple<bool, uint64_t, uint64_t> QueryVal(uint64_t rob_idx);

    void Reset();

    std::vector<std::unique_ptr<const InstrContext>> GetInstrs();
    std::vector<bool> GetStatus();
    std::pair<size_t, size_t> GetHeadTail();

    void ResetTailTillIdx(size_t new_head, DualIssueCore& vm_core);
    
private:
    ROBBuffer buffer;

    void BroadCastMsgs(DualIssueInstrContext& instr, CommonDataBus& data_bus, bool clear_dependency);
};

} // namespace dual_issue