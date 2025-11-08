#include "vm/dual_issue/hardware/data_bus.h"

namespace dual_issue
{
    
void CommonDataBus::Reset(){
    broadcast_msgs.clear();
}

void CommonDataBus::BroadCast(uint64_t rob_idx, uint64_t val){
    broadcast_msgs.push_back(BroadCastMessage(rob_idx, val));
}

} // namespace dual_issue
