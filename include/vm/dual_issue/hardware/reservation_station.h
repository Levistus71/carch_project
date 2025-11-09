#pragma once
#include "./commit_buffer.h"
#include "./reg_status_file.h"
#include "./data_bus.h"

namespace dual_issue{
class DualIssueCore;

class ReservationStation{
public:
    ReservationStation();

    size_t EmptySlots();

    void ListenToBroadCast(CommonDataBus& data_bus);

    void Push(DualIssueInstrContext instr, DualIssueCore& vm_core);

    DualIssueInstrContext GetReadyInstr();

    void Reset();

    std::vector<std::unique_ptr<const InstrContext>> GetQue();
    
private:
    static const size_t max_size_ = 4;
    std::deque<DualIssueInstrContext> que_;
};


} // namespace dual_issue