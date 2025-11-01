#include "../core/core.h"
#include "../stages/stages.h"
#include "utils.h"
#include <thread>
#include <chrono>

namespace rv5s{

class Executor{
public:
    static void Run(Core& vm_core);

    static void DebugRun(Core& vm_core);

    static void Step(Core& vm_core);

    static void Undo(Core& vm_core);

private:

    static void RunPipelined(Core& vm_core);
    static void RunSingleCycle(Core& vm_core);

    static void DebugRunPipelined(Core& vm_core);
    static void DebugRunSingleCycle(Core& vm_core);

    static void StepPipelined(Core& vm_core);
    static void StepSingleCycle(Core& vm_core, bool dump);

    static void UndoPipelined(Core& vm_core);
    static void UndoSingleCycle(Core& vm_core);
    
};


} // namespace rv5s
