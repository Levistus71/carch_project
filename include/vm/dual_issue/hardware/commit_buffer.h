#pragma once
#include "./data_bus.h"
#include <set>

namespace dual_issue{

struct BufferInstrCompare{
    bool operator()(DualIssueInstrContext& instr1, DualIssueInstrContext& instr2){
        return instr1.tag < instr2.tag;
    }
};

class CommitBuffer{
public:
    bool IsFull();
    bool IsEmpty();

    void Push(DualIssueInstrContext& instr, DualIssueCore& core);
    
    std::pair<bool, uint64_t> Query(uint64_t tag);
    
private:
    std::set<DualIssueInstrContext, BufferInstrCompare> buffer;
    size_t max_buffer_size = 8;

    void Commit(DualIssueCore& core);
    void BroadCastMsgs(DualIssueInstrContext& instr, CommonDataBus& data_bus);
};

} // namespace dual_issue