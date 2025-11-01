#pragma once

namespace rv5s {
class Core;

class HazardDetector {    
public:

    bool DetectDataHazard(Core& vm_core);

    void HandleDataHazard(Core& vm_core);

    bool DetectControlHazard(Core& vm_core);

    void HandleControlHazard(Core& vm_core);
    
private:

    bool DetectDataHazardWithoutForwarding(Core& vm_core);
    bool DetectDataHazardWithForwarding(Core& vm_core);

};

} // namespace rv5s