#pragma once
#include "vm/dual_issue/hardware/data_bus.h"
#include "vm/dual_issue/hardware/commit_buffer.h"
#include "vm/triple_issue/core/instruction_context/instruction_context.h"

namespace triple_issue
{

class TripleIssueCore;

class ReorderBuffer{
public:
    ReorderBuffer(){}
    ReorderBuffer(size_t slots) : buffer(slots) {}

    size_t EmptySlots();

    void Pull(TripleIssueCore& vm_core);
    void Push(dual_issue::DualIssueInstrContext& instr, TripleIssueCore& vm_core);
    
    void Commit(TripleIssueCore& vm_core);

    std::pair<size_t, size_t> Reserve();

    std::tuple<bool, uint64_t, uint64_t> QueryVal(uint64_t rob_idx);

    void Reset();

    std::vector<std::unique_ptr<const InstrContext>> GetInstrs();
    std::vector<bool> GetStatus();
    std::pair<size_t, size_t> GetHeadTail();

    void ResetTailTillIdx(size_t new_head, TripleIssueCore& vm_core);
    
private:
    dual_issue::ROBBuffer buffer;

    void BroadCastMsgs(dual_issue::DualIssueInstrContext& instr, dual_issue::CommonDataBus& data_bus, bool clear_dependency);
};



} // namespace triple_issue
