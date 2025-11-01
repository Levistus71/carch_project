#pragma once
#include "../core/core.h"

namespace rv5s {

class HazardDetector {    
public:

    bool DetectDataHazard(Core& vm_core);

    void HandleDataHazard(Core& vm_core);

    bool DetectControlHazard(Core& vm_core);

    void HandleControlHazard(Core& vm_core);
    
private:


};

} // namespace rv5s