#pragma once
#include "../core/core.h"
#include "./commit_buffer.h"
#include "./reg_status_file.h"
#include "./data_bus.h"

namespace dual_issue{

class ReservationStation{
public:
    size_t EmptySlots();

    void ListenToBroadCast(CommonDataBus& data_bus);

    void Push(DualIssueInstrContext instr, DualIssueCore& vm_core);

    DualIssueInstrContext GetReadyInstr(DualIssueCore& vm_core);
    
private:
    static const size_t max_size_ = 4;
    std::deque<DualIssueInstrContext> que_;
};


} // namespace dual_issue