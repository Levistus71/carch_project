#pragma once

namespace rv5s {
class PipelinedCore;

class HazardDetector {    
public:

    bool DetectDataHazard(PipelinedCore& vm_core);

    void HandleDataHazard(PipelinedCore& vm_core);

    bool DetectControlHazard(PipelinedCore& vm_core);

    void HandleControlHazard(PipelinedCore& vm_core);
    
private:

    bool DetectDataHazardWithoutForwarding(PipelinedCore& vm_core);
    bool DetectDataHazardWithForwarding(PipelinedCore& vm_core);

};

} // namespace rv5s