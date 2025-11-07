#include "../core/core.h"
#include "../stages/stages.h"
#include "utils.h"
#include <thread>
#include <chrono>

namespace rv5s{

class PipelinedExecutor{
public:
    static void RunPipelined(PipelinedCore& vm_core);

    static void DebugRunPipelined(PipelinedCore& vm_core);

    static void StepPipelined(PipelinedCore& vm_core);

    static void UndoPipelined(PipelinedCore& vm_core);
    
};


} // namespace rv5s
