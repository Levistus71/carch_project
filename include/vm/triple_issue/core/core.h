#pragma once
#include "vm/triple_issue/core/instruction_context/instruction_context.h"
#include "vm/dual_issue/hardware/reservation_station.h"
#include "vm/rv5s/pipelined/hardware/branch_predictor.h"
#include "vm/triple_issue/hardware/decode_unit.h"
#include "vm/triple_issue/hardware/commit_buffer.h"
#include "vm/registers.h"
#include "vm/memory_controller.h"
#include "vm_asm_mw.h"
#include "vm/vm_base.h"
#include "vm/dual_issue/core/core.h"

namespace triple_issue{

class TripleIssueCore : public dual_issue::DualIssueCore{
public:
    struct PipelineRegInstrs : public dual_issue::DualIssueCore::PipelineRegInstrs{
        TripleIssueInstrContext if_id_1;
        TripleIssueInstrContext if_id_2;
        TripleIssueInstrContext if_id_3;

        TripleIssueInstrContext id_issue_1;
        TripleIssueInstrContext id_issue_2;
        TripleIssueInstrContext id_issue_3;

        dual_issue::DualIssueInstrContext rsrvstn_falu;

        dual_issue::DualIssueInstrContext falu_commit;

        void FlushPreIssueRegs();
        void Reset();
    };

    TripleIssueCore() : commit_buffer_(32) {}

    void FlushPreIssueRegs();
    void Reset();

    PipelineRegInstrs pipeline_reg_instrs_;

    dual_issue::ReservationStation falu_que_;

    // Hardware
    TripleIssueDecodeUnit decode_unit_;
    triple_issue::ReorderBuffer commit_buffer_;
};


} // namespace dual_issue"