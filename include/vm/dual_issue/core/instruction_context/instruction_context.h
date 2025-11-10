#pragma once
#include "vm/instruction_context.h"

namespace dual_issue{

struct DualIssueInstrContext : InstrContext {
    DualIssueInstrContext();
    DualIssueInstrContext(uint64_t pc);
    ~DualIssueInstrContext() = default;
    
    // branch signals:
    bool branch_predicted_taken = false; // job of the 'FETCH' stage to update this. (For branch prediction)

    uint64_t rs1_tag;
    uint64_t rs1_epoch;
    uint64_t rs2_tag;
    uint64_t rs2_epoch;
    uint64_t rs3_tag;
    uint64_t rs3_epoch;
    size_t rob_idx;
    bool wait_for_rs1 = true;
    bool wait_for_rs2 = true;
    bool wait_for_rs3 = true;

    // is illegal instruction (pc was greater than program_size_)
    bool illegal = false;
    size_t epoch;

    // forwarding vars
    bool uses_rs1 = false;
    bool uses_rs2 = false;
    bool uses_rs3 = false;

    // bools for status
    bool ready_to_exec = false;

    void reset_id_vars();
};


} // namespace dual_issue