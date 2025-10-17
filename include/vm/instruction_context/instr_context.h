#pragma once

#include "alu.h"    // FIXME: uint8_t not working on mac without this (or any other header)

struct InstrContext{
    InstrContext(uint64_t pc) : pc{pc} {}

    // instruction:
    uint32_t instruction;

    // alu operation
    alu::AluOp alu_op;

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

    // registers:
    uint8_t rs1;
    uint8_t rs2;
    uint8_t frs3;
    uint8_t rd;

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
};