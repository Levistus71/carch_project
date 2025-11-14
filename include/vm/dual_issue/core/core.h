#pragma once
#include "./instruction_context/instruction_context.h"
#include "../hardware/reservation_station.h"
#include "vm/rv5s/pipelined/hardware/branch_predictor.h"
#include "../hardware/decode_unit.h"
#include "vm/registers.h"
#include "vm/memory_controller.h"
#include "vm_asm_mw.h"
#include "vm/vm_base.h"


namespace dual_issue{

class DualIssueCore{
public:
    struct PipelineRegInstrs{
        DualIssueInstrContext if_id_1;
        DualIssueInstrContext if_id_2;

        DualIssueInstrContext id_issue_1;
        DualIssueInstrContext id_issue_2;

        DualIssueInstrContext rsrvstn_alu;
        DualIssueInstrContext rsrvstn_lsu;

        DualIssueInstrContext alu_commit;
        DualIssueInstrContext lsu_commit;

        void FlushPreIssueRegs();
        void Reset();
    };

    virtual ~DualIssueCore() = default;

    void FlushPreIssueRegs();

    PipelineRegInstrs pipeline_reg_instrs_;
    uint64_t pc = 0;

    bool is_stop_requested_ = false;

    // brach prediction
    bool branch_prediction_enabled_ = false;
	bool branch_prediction_static_ = false;

    // Debug vars
    bool debug_mode_{true};
    std::deque<DualIssueInstrContext> undo_instruction_stack_;
    size_t max_undo_stack_size_{256};
    bool stop_requested_ = false;
    std::vector<uint64_t> breakpoints_;

    // Hardware
    alu::Alu alu_;
    register_file::RegisterFile register_file_;
    memory_controller::MemoryController memory_controller_;
    DualIssueDecodeUnit decode_unit_;
    ReservationStation alu_que_;
    ReservationStation lsu_que_;
    CommonDataBus broadcast_bus_;
    ReorderBuffer commit_buffer_;
    RegisterStatusFile reg_status_file_;
    rv5s::BranchPredictor branch_predictor_;

    // for input handling in syscalls:
	std::mutex input_mutex_;
	std::condition_variable input_cv_;
	std::queue<std::string> input_queue_;

    uint64_t program_size_ = 0;

    VmBase::Stats core_stats_;

    uint64_t GetProgramCounter() const;
    void AddToProgramCounter(int64_t value);
    void SetProgramCounter(uint64_t value);

    void Reset();

    void Load();
    void Load(AssembledProgram& program);

};


} // namespace dual_issue