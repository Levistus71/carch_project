#pragma once

struct SimState{
    bool MEMORY_DIRTY = false;
    bool DATA_FORWARD = false;
    bool HAZARD_DETECTED = false;
    bool LIT_UP = false;

    enum class DataForwardPaths{
        MEM_WB_RS1,
        MEM_WB_RS2,
        EXEC_MEM_RS1,
        EXEC_MEM_RS2
    };
    DataForwardPaths DF_PATH;
    bool DF_ALL;

    enum class HazardPaths{
        MEM_WB,
        EXEC_MEM
    };
    HazardPaths HZ_PATH;
};


extern SimState SimState_;