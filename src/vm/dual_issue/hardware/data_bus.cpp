#include "vm/dual_issue/hardware/data_bus.h"

namespace dual_issue
{
    
void CommonDataBus::Reset(){
    broadcast_msgs.clear();
}

void CommonDataBus::BroadCast(uint64_t rob_idx, uint64_t val, bool clear_dependency, uint64_t epoch){
    broadcast_msgs.push_back(BroadCastMessage(rob_idx, val, clear_dependency, epoch));
}

} // namespace dual_issue
