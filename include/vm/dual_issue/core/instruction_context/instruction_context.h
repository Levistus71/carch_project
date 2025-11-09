#pragma once
#include "vm/instruction_context.h"

namespace dual_issue{

struct DualIssueInstrContext : InstrContext {
    DualIssueInstrContext();
    DualIssueInstrContext(uint64_t pc);
    ~DualIssueInstrContext() = default;
    
    // branch signals:
    bool branch_predicted_taken = false; // job of the 'FETCH' stage to update this. (For branch prediction)
    bool branch_taken = false; // job of the 'EXEC' stage to update this. (For branch prediction), checked by the DetectControlHazard() in hazard_detector_

    uint64_t rs1_tag;
    uint64_t rs2_tag;
    uint64_t rs3_tag;
    size_t rob_idx;
    bool wait_for_rs1;
    bool wait_for_rs2;
    bool wait_for_rs3;

    // is illegal instruction (pc was greater than program_size_)
    bool illegal = false;

    // forwarding vars
    bool uses_rs1;
    bool uses_rs2;
    bool uses_rs3;

    // bools for status
    bool ready_to_exec = false;

    void reset_id_vars();
};


} // namespace dual_issue