#include "../core/core.h"
#include "../stages/stages.h"
#include "utils.h"
#include <thread>
#include <chrono>

namespace rv5s{

class SingleCycleExecutor{
public:
    static void RunSingleCycle(SingleCycleCore& vm_core);

    static void DebugRunSingleCycle(SingleCycleCore& vm_core);

    static void StepSingleCycle(SingleCycleCore& vm_core, bool dump);

    static void UndoSingleCycle(SingleCycleCore& vm_core);
};


} // namespace rv5s
