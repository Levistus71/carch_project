#pragma once

#include "vm/alu.h"

struct InstrContext{

    // instruction:
    uint32_t instruction;
    
    // alu operation
    alu::AluOp alu_op = alu::AluOp::kNone;
    
    // opcode and funct values:
    uint8_t opcode;
    uint8_t funct2;
    uint8_t funct3;
    uint8_t funct5;
    uint8_t funct7;
    
    // pc
    uint64_t pc;
    bool auipc = false;
    
    // mux signals:
    bool mem_to_reg = false; // "alu_to_reg" is false if this is true
    bool imm_to_alu = false; // selects the input to the alu (from immediate and register),"reg_to_alu" is false if this is true
    
    // memory signals:
    bool mem_read = false;
    bool mem_write = false;
    bool mem_write_data_from_gpr = false; // is the write data from fprs2 / rs2
    size_t mem_access_bytes;
    bool sign_extend = false;
    
    // register signals:
    bool reg_write = false;
    bool reg_write_to_fpr = false; // is the write data to gpr / fpr
    
    // branch signals:
    bool branch = false;
    bool branch_predicted_taken = false; // job of the 'FETCH' stage to update this. (For branch prediction)
    bool branch_taken = false; // job of the 'EXEC' stage to update this. (For branch prediction)
    
    // registers:
    uint8_t rs1;
    uint8_t rs2;
    uint8_t frs3;
    uint8_t rd;

    // bools for whether the instruction is reading from the fpr file
    bool rs1_from_fprf; // if this is true, g_rs1 is false;
    bool rs2_from_fprf; // if this is true, g_rs2 is false;
    bool uses_rs1;
    bool uses_rs2;
    bool uses_rs3;
    
    // register values:
    // base register values:
    uint64_t rs1_value;
    uint64_t rs2_value;
    // fpr register values:
    uint64_t frs1_value;
    uint64_t frs2_value;
    uint64_t frs3_value;
    
    // immediate:
    int32_t immediate;
    
    // alu output
    uint64_t alu_out;
    bool alu_overflow = false;
    
    // memory_output
    uint64_t mem_out;
    
    // csr related
    uint16_t csr_rd;
    uint64_t csr_value;
    uint8_t csr_uimm;
    uint64_t csr_write_val;
    bool csr_op = false;

    // nop flag
    bool nopped = false;
    
    
    // Constructors:
    InstrContext() = default;
    InstrContext(uint64_t pc) : pc{pc} {}
    InstrContext(const InstrContext& other_instr_context) = default;
    InstrContext(InstrContext&& other_instr_context) = default;
    InstrContext& operator=(const InstrContext& other_instr_context) = default;
    InstrContext& operator=(InstrContext&& other_instr_context) = default;
    ~InstrContext() = default;

    // convert this instruction to nop
    void nopify(){
        mem_read = false;
        mem_write = false;
        reg_write = false;
        branch = false;
        nopped = true;
    }
};