#include "vm/dual_issue/core/instruction_context/instruction_context.h"

namespace dual_issue{

DualIssueInstrContext::DualIssueInstrContext() : InstrContext() {}

DualIssueInstrContext::DualIssueInstrContext(uint64_t pc) : InstrContext(pc){}

void DualIssueInstrContext::reset_id_vars(){
    alu_op = alu::AluOp::kNone;
    
    opcode = 0;
    funct2 = 0;
    funct3 = 0;
    funct5 = 0;
    funct7 = 0;
    
    auipc = false;
    
    mem_to_reg = false;
    imm_to_alu = false;
    
    mem_read = false;
    mem_write = false;
    mem_write_data_from_gpr = false;
    mem_access_bytes = 0;
    sign_extend = false;
    
    reg_write = false;
    reg_write_to_fpr = false;
    
    branch = false;
    branch_predicted_taken = false;
    branch_taken = false;
    
    rs1 = 0;
    rs2 = 0;
    frs3 = 0;
    rd = 0;

    rs1_from_fprf = false;
    rs2_from_fprf = false;
    
    rs1_value = 0;
    rs2_value = 0;
    frs1_value = 0;
    frs2_value = 0;
    frs3_value = 0;
    
    immediate = 0;
    
    alu_overflow = false;
    
    // csr related
    csr_rd = 0;
    csr_value = 0;
    csr_uimm = 0;
    csr_write_val = 0;
    csr_op = false;
    fcsr_status = 0;
    fcsr_update = false;

    // bools for status
    ready_to_exec = false;
}
}