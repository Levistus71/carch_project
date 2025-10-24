#pragma once

#include "vm/alu.h"

struct InstrContext{

    // instruction:
    uint32_t instruction;
    
    // alu operation
    alu::AluOp alu_op = alu::AluOp::kNone;
    
    // opcode and funct values:
    uint8_t opcode = 0;
    uint8_t funct2 = 0;
    uint8_t funct3 = 0;
    uint8_t funct5 = 0;
    uint8_t funct7 = 0;
    
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
    uint8_t rs1 = 0;
    uint8_t rs2 = 0;
    uint8_t frs3 = 0;
    uint8_t rd = 0;

    // bools for whether the instruction is reading from the fpr file
    bool rs1_from_fprf = false; // if this is true, g_rs1 is false;
    bool rs2_from_fprf = false; // if this is true, g_rs2 is false;
    bool uses_rs1 = false;
    bool uses_rs2 = false;
    bool uses_rs3 = false;
    
    // register values:
    // base register values:
    uint64_t rs1_value = 0;
    uint64_t rs2_value = 0;
    // fpr register val = 0ues:
    uint64_t frs1_value;
    uint64_t frs2_value = 0;
    uint64_t frs3_value = 0;
    
    // immediate:
    int32_t immediate = 0;
    
    // alu output
    uint64_t alu_out = 0;
    bool alu_overflow = false;
    
    // memory_output
    uint64_t mem_out = 0;
    
    // csr related
    uint16_t csr_rd = 0;
    uint64_t csr_value = 0;
    uint8_t csr_uimm = 0;
    uint64_t csr_write_val = 0;
    bool csr_op = false;
    uint64_t fcsr_status = 0;
    bool fcsr_update = false;

    // nop flag
    bool nopped = false;
    bool bubbled = false;

    // DEBUG:
    std::vector<uint8_t> mem_overwritten;
    uint64_t reg_overwritten = 0;
    uint64_t csr_overwritten = 0;
    bool is_csr_overwritten = false;
    
    
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


    void reset_id_vars(){
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
        uses_rs1 = false;
        uses_rs2 = false;
        uses_rs3 = false;
        
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

        // nop flag
        bool nopped = false;
    }
};