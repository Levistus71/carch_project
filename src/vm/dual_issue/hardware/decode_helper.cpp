#include "vm/dual_issue/hardware/decode_helper.h"

namespace DecoderHelper
{

using namespace dual_issue;

void SetContextValues(DualIssueInstrContext& instr_context){
    uint8_t& opcode = instr_context.opcode;
    uint8_t& funct2 = instr_context.funct2;
    uint8_t& funct3 = instr_context.funct3;
    uint8_t& funct5 = instr_context.funct5;
    uint8_t& funct7 = instr_context.funct7;

    switch (opcode)
    {
    case 0b0110011: {// R-Type

        instr_context.reg_write = true;
        instr_context.uses_rs1 = true;
        instr_context.uses_rs2 = true;

        switch (funct3)
        {
            case 0b000:{ // kAdd, kSub, kMul
                switch (funct7)
                {
                    case 0x0000000: {// kAdd
                        instr_context.alu_op = alu::AluOp::kAdd;
                        return;
                    }
                    case 0b0100000: {// kSub
                        instr_context.alu_op = alu::AluOp::kSub;
                        return;
                    }
                    case 0b0000001: {// kMul
                        instr_context.alu_op = alu::AluOp::kMul;
                        return;
                    }
                }
                break;
            }
            case 0b001: {// kSll, kMulh
                switch (funct7)
                {
                    case 0b0000000: {// kSll
                        instr_context.alu_op = alu::AluOp::kSll;
                        return;
                    }
                    case 0b0000001: {// kMulh
                        instr_context.alu_op = alu::AluOp::kMulh;
                        return;
                    }
                }
                break;
            }
            case 0b010: {// kSlt, kMulhsu
                switch (funct7)
                {
                    case 0b0000000: {// kSlt
                        instr_context.alu_op = alu::AluOp::kSlt;
                        return;
                    }
                    case 0b0000001: {// kMulhsu
                        instr_context.alu_op = alu::AluOp::kMulhsu;
                        return;
                    }
                }
                break;
            }
            case 0b011: {// kSltu, kMulhu
                switch (funct7)
                {
                    case 0b0000000: {// kSltu
                        instr_context.alu_op = alu::AluOp::kSltu;
                        return;
                    }
                    case 0b0000001: {// kMulhu
                        instr_context.alu_op = alu::AluOp::kMulhu;
                        return;
                    }
                }
                break;
            }
            case 0b100: {// kXor, kDiv
                switch (funct7)
                {
                    case 0b0000000: {// kXor
                        instr_context.alu_op = alu::AluOp::kXor;
                        return;
                    }
                    case 0b0000001: {// kDiv
                        instr_context.alu_op = alu::AluOp::kDiv;
                        return;
                    }
                }
                break;
            }
            case 0b101: {// kSrl, kSra, kDivu
                switch (funct7)
                {
                    case 0b0000000: {// kSrl
                        instr_context.alu_op = alu::AluOp::kSrl;
                        return;
                    }
                    case 0b0100000: {// kSra
                        instr_context.alu_op = alu::AluOp::kSra;
                        return;
                    }
                    case 0b0000001: {// kDivu
                        instr_context.alu_op = alu::AluOp::kDivu;
                        return;
                    }
                }
                break;
            }
            case 0b110: {// kOr, kRem
                switch (funct7)
                {
                    case 0b0000000: {// kOr
                        instr_context.alu_op = alu::AluOp::kOr;
                        return;
                    }
                    case 0b0000001: {// kRem
                        instr_context.alu_op = alu::AluOp::kRem;
                        return;
                    }
                }
                break;
            }
            case 0b111: {// kAnd, kRemu
                switch (funct7)
                {
                    case 0b0000000: {// kAnd
                        instr_context.alu_op = alu::AluOp::kAnd;
                        return;
                    }
                    case 0b0000001: {// kRemu
                        instr_context.alu_op = alu::AluOp::kRemu;
                        return;
                    }
                }
                break;
            }
        }
        break;
    }

    case 0b0010011: {// I-Type
        instr_context.imm_to_alu = true;
        instr_context.reg_write = true;
        instr_context.uses_rs1 = true;

        switch (funct3)
        {
            case 0b000: {// ADDI
                instr_context.alu_op = alu::AluOp::kAdd;
                return;
            }
            case 0b001: {// SLLI
                instr_context.alu_op = alu::AluOp::kSll;
                return;
            }
            case 0b010: {// SLTI
                instr_context.alu_op = alu::AluOp::kSlt;
                return;
            }
            case 0b011: {// SLTIU
                instr_context.alu_op = alu::AluOp::kSltu;
                return;
            }
            case 0b100: {// XORI
                instr_context.alu_op = alu::AluOp::kXor;
                return;
            }
            case 0b101: {// SRLI & SRAI
                switch (funct7)
                {
                    case 0b0000000: {// SRLI
                        instr_context.alu_op = alu::AluOp::kSrl;
                        return;
                    }
                    case 0b0100000: {// SRAI
                        instr_context.alu_op = alu::AluOp::kSra;
                        return;
                    }
                }
                break;
            }
            case 0b110: {// ORI
                instr_context.alu_op = alu::AluOp::kOr;
                return;
            }
            case 0b111: {// ANDI
                instr_context.alu_op = alu::AluOp::kAnd;
                return;
            }
        }
        break;
    }
    case 0b1100011: {// B-Type
        instr_context.branch = true;
        instr_context.uses_rs1 = true;
        instr_context.uses_rs2 = true;

        switch (funct3)
        {
            case 0b000: {// BEQ
                instr_context.alu_op = alu::AluOp::kSub;
                return;
            }
            case 0b001: {// BNE
                instr_context.alu_op = alu::AluOp::kSub;
                return;
            }
            case 0b100: {// BLT
                instr_context.alu_op = alu::AluOp::kSlt;
                return;
            }
            case 0b101: {// BGE
                instr_context.alu_op = alu::AluOp::kSlt;
                return;
            }
            case 0b110: {// BLTU
                instr_context.alu_op = alu::AluOp::kSltu;
                return;
            }
            case 0b111: {// BGEU
                instr_context.alu_op = alu::AluOp::kSltu;
                return;
            }
        }
        break;
    }

    case 0b0000011: {// Load
        instr_context.imm_to_alu = true;
        instr_context.mem_to_reg = true;
        instr_context.reg_write = true;
        instr_context.mem_read= true;
        instr_context.uses_rs1 = true;
        
        switch(instr_context.funct3){
            case(0b000):{     // LB
                instr_context.sign_extend = true;
                instr_context.mem_access_bytes = 1;
                break;
            }
            case (0b001): {   // LH
                instr_context.sign_extend = true;
                instr_context.mem_access_bytes = 2;
                break;
            }
            case (0b010): {   // LW
                instr_context.sign_extend = true;
                instr_context.mem_access_bytes = 4;
                break;
            }
            case (0b011) : {    // LD
                instr_context.mem_access_bytes = 8;
                break;
            }
            case (0b100) : {    // LBU
                instr_context.mem_access_bytes = 1;
                break;
            }
            case (0b101) : {    // LHU
                instr_context.mem_access_bytes = 2;
                break;
            }
            case(0b110) : {     // LWU
                instr_context.mem_access_bytes = 4;
                break;
            }
        }

        instr_context.alu_op = alu::AluOp::kAdd;
        return;
    }
    case 0b0100011: {// Store
        instr_context.imm_to_alu = true;
        instr_context.mem_write = true;
        instr_context.mem_write_data_from_gpr = true;
        instr_context.uses_rs1 = true;
        instr_context.uses_rs2 = true;

        switch(funct3){
            case 0b000 : {  // SB
                instr_context.mem_access_bytes = 1;
                break;
            }
            case 0b001 : { // SH
                instr_context.mem_access_bytes = 2;
                break;
            }
            case 0b010 : {  // SW
                instr_context.mem_access_bytes = 4;
                break;
            }
            case 0b011: {   // SD
                instr_context.mem_access_bytes = 8;
                break;
            }
        }

        instr_context.alu_op = alu::AluOp::kAdd;
        return;
    }
    case 0b1100111: {// JALR
        instr_context.imm_to_alu = true;
        instr_context.reg_write = true;
        instr_context.branch = true;
        instr_context.uses_rs1 = true;

        instr_context.alu_op = alu::AluOp::kAdd;
        return;
    }
    case 0b1101111: {// JAL
        instr_context.reg_write = true;
        instr_context.branch = true;
        instr_context.uses_rs1 = true;

        instr_context.alu_op = alu::AluOp::kAdd;
        return;
    }
    case 0b0110111: {// LUI
        instr_context.imm_to_alu = true;
        instr_context.reg_write = true;

        instr_context.alu_op = alu::AluOp::kLui;
        return;
    }
    case 0b0010111: {// AUIPC
        instr_context.imm_to_alu = true;
        instr_context.reg_write = true;
        instr_context.auipc = true;

        instr_context.alu_op = alu::AluOp::kAuipc;
        return;
    }
    case 0b0000000: {// FENCE
        instr_context.alu_op = alu::AluOp::kNone;
        return;
    }
    case 0b1110011: {// SYSTEM
        switch (funct3) 
        {
        case 0b000: // ECALL
            instr_context.alu_op = alu::AluOp::kNone;
            return;
        case 0b001: // CSRRW
            instr_context.alu_op = alu::AluOp::kNone;
            return;
        default:
            break;
        }
        break;
    }
    case 0b0011011: {// R4-Type
        instr_context.imm_to_alu = true;
        instr_context.uses_rs1 = true;
        instr_context.reg_write = true;

        switch (funct3) 
        {
            case 0b000: {// ADDIW
                instr_context.alu_op = alu::AluOp::kAddw;
                return;
            }
            case 0b001: {// SLLIW
                instr_context.alu_op = alu::AluOp::kSllw;
                return;
            }
            case 0b101: {// SRLIW & SRAIW
                switch (funct7) 
                {
                case 0b0000000: {// SRLIW
                    instr_context.alu_op = alu::AluOp::kSrlw;
                    return;
                }
                case 0b0100000: {// SRAIW
                        instr_context.alu_op = alu::AluOp::kSraw;
                        return;
                    }
                }
                break;
            }
        }
        break;
    }
    case 0b0111011: {// R4-Type
        instr_context.reg_write = true;
        instr_context.uses_rs1 = true;
        instr_context.uses_rs2 = true;

        switch (funct3) {
            case 0b000: {// kAddw, kSubw, kMulw
                switch (funct7) 
                {
                    case 0b0000000: {// kAddw
                        instr_context.alu_op = alu::AluOp::kAddw;
                        return;
                    }
                    case 0b0100000: {// kSubw
                        instr_context.alu_op = alu::AluOp::kSubw;
                        return;
                    }
                    case 0b0000001: {// kMulw
                        instr_context.alu_op = alu::AluOp::kMulw;
                        return;
                    }
                }
                break;
            }
            case 0b001: {// kSllw
                instr_context.alu_op = alu::AluOp::kSllw;
                return;
            }
            case 0b100: {// kDivw
                switch (funct7) {// kDivw
                    case 0b0000001: {// kDivw
                        instr_context.alu_op = alu::AluOp::kDivw;
                        return;
                    }
                }
                break;
            }
            case 0b101: {// kSrlw, kSraw, kDivuw
                switch (funct7) {
                    case 0b0000000: {// kSrlw
                        instr_context.alu_op = alu::AluOp::kSrlw;
                        return;
                    }
                    case 0b0100000: {// kSraw
                        instr_context.alu_op = alu::AluOp::kSraw;
                        return;
                    }
                    case 0b0000001: {// kDivuw
                        instr_context.alu_op = alu::AluOp::kDivuw;
                        return;
                    }
                }
                break;
            }
            case 0b110: {// kRemw
                switch (funct7) 
                {
                    case 0b0000001: {// kRemw
                        instr_context.alu_op = alu::AluOp::kRemw;
                        return;
                    }
                }
                break;
            }
            case 0b111: {// kRemuw
                    switch (funct7) {
                        case 0b0000001: {// kRemuw
                            instr_context.alu_op = alu::AluOp::kRemuw;
                            return;
                        }
                    }
                    break;
                }
        }
        break;
    }
    
    // F extension + D extension

    case 0b1010011: {
        instr_context.reg_write = true;
        instr_context.reg_write_to_fpr = true;
        instr_context.uses_rs1 = true;
        instr_context.uses_rs2 = true;
        instr_context.rs1_from_fprf = true;
        instr_context.rs2_from_fprf = true;

        switch (funct7) {
            case 0b0000000: {// FADD_S
                instr_context.alu_op = alu::AluOp::FADD_S;
                return;
            }
            case 0b0000001: {// FADD_D
                instr_context.alu_op = alu::AluOp::FADD_D;
                return;
            }
            case 0b0000100: {// FSUB_S
                instr_context.alu_op = alu::AluOp::FSUB_S;
                return;
            }
            case 0b0000101: {// FSUB_D
                instr_context.alu_op = alu::AluOp::FSUB_D;
                return;
            }
            case 0b0001000: {// FMUL_S
                instr_context.alu_op = alu::AluOp::FMUL_S;
                return;
            }
            case 0b0001001: {// FMUL_D
                instr_context.alu_op = alu::AluOp::FMUL_D;
                return;
            }
            case 0b0001100: {// FDIV_S
                instr_context.alu_op = alu::AluOp::FDIV_S;
                return;
            }
            case 0b0001101: {// FDIV_D
                instr_context.alu_op = alu::AluOp::FDIV_D;
                return;
            }
            case 0b0101100: {// FSQRT_S
                instr_context.uses_rs2 = false;
                instr_context.alu_op = alu::AluOp::FSQRT_S;
                return;
            }
            case 0b0101101: {// FSQRT_D
                instr_context.uses_rs2 = false;
                instr_context.alu_op = alu::AluOp::FSQRT_D;
                return;
            }
            case 0b1100000: { // FCVT.(W|WU|L|LU).S
                instr_context.reg_write_to_fpr = false;
                instr_context.uses_rs2 = false;

                switch (funct5) {
                    case 0b00000: {// FCVT_W_S
                        instr_context.alu_op = alu::AluOp::FCVT_W_S;
                        return;
                    }
                    case 0b00001: {// FCVT_WU_S
                        instr_context.alu_op = alu::AluOp::FCVT_WU_S;
                        return;
                    }
                    case 0b00010: {// FCVT_L_S
                        instr_context.alu_op = alu::AluOp::FCVT_L_S;
                        return;
                    }
                    case 0b00011: {// FCVT_LU_S
                        instr_context.alu_op = alu::AluOp::FCVT_LU_S;
                        return;
                    }
                }
                break;
            }
            case 0b1100001: { // FCVT.(W|WU|L|LU).D
                instr_context.reg_write_to_fpr = false;
                instr_context.uses_rs2 = false;

                switch (funct5) {
                    case 0b00000: {// FCVT_W_D
                        instr_context.alu_op = alu::AluOp::FCVT_W_D;
                        return;
                    }
                    case 0b00001: {// FCVT_WU_D
                        instr_context.alu_op = alu::AluOp::FCVT_WU_D;
                        return;
                    }
                    case 0b00010: {// FCVT_L_D
                        instr_context.alu_op = alu::AluOp::FCVT_L_D;
                        return;
                    }
                    case 0b00011: {// FCVT_LU_D
                        instr_context.alu_op = alu::AluOp::FCVT_LU_D;
                        return;
                    }
                }
                break;
            }
            case 0b1101000: { // FCVT.S.(W|WU|L|LU)
                instr_context.uses_rs2 = false;
                instr_context.rs1_from_fprf = false;

                switch (funct5) {
                    case 0b00000: {// FCVT_S_W
                        instr_context.alu_op = alu::AluOp::FCVT_S_W;
                        return;
                    }
                    case 0b00001: {// FCVT_S_WU
                        instr_context.alu_op = alu::AluOp::FCVT_S_WU;
                        return;
                    }
                    case 0b00010: {// FCVT_S_L
                        instr_context.alu_op = alu::AluOp::FCVT_S_L;
                        return;
                    }
                    case 0b00011: {// FCVT_S_LU
                        instr_context.alu_op = alu::AluOp::FCVT_S_LU;
                        return;
                    }
                }
                break;
            }
            case 0b1101001: { // FCVT.D.(W|WU|L|LU)
                instr_context.uses_rs2 = false;
                instr_context.rs1_from_fprf = false;

                switch (funct5) {
                    case 0b00000: {// FCVT_D_W
                        instr_context.alu_op = alu::AluOp::FCVT_D_W;
                        return;
                    }
                    case 0b00001: {// FCVT_D_WU
                        instr_context.alu_op = alu::AluOp::FCVT_D_WU;
                        return;
                    }
                    case 0b00010: {// FCVT_D_L
                        instr_context.alu_op = alu::AluOp::FCVT_D_L;
                        return;
                    }
                    case 0b00011: {// FCVT_D_LU
                        instr_context.alu_op = alu::AluOp::FCVT_D_LU;
                        return;
                    }
                }
                break;
            }
            case 0b0010000: { // FSGNJ(N|X).S
                switch (funct3) {
                    case 0b000: {// FSGNJ
                        instr_context.alu_op = alu::AluOp::FSGNJ_S;
                        return;
                    }
                    case 0b001: {// FSGNJN
                        instr_context.alu_op = alu::AluOp::FSGNJN_S;
                        return;
                    }
                    case 0b010: {// FSGNJX
                        instr_context.alu_op = alu::AluOp::FSGNJX_S;
                        return;
                    }
                }
                break;
            }
            case 0b0010001: { // FSGNJ(N|X).D
                switch (funct3) {
                    case 0b000: {// FSGNJ
                        instr_context.alu_op = alu::AluOp::FSGNJ_D;
                        return;
                    }
                    case 0b001: {// FSGNJN
                        instr_context.alu_op = alu::AluOp::FSGNJN_D;
                        return;
                    }
                    case 0b010: {// FSGNJX
                        instr_context.alu_op = alu::AluOp::FSGNJX_D;
                        return;
                    }
                }
                break;
            }
            case 0b0010100: { // F(MIN|MAX).S
                switch (funct3) {
                    case 0b000: {// FMIN
                        instr_context.alu_op = alu::AluOp::FMIN_S;
                        return;
                    }
                    case 0b001: {// FMAX
                        instr_context.alu_op = alu::AluOp::FMAX_S;
                        return;
                    }
                }
                break;
            }
            case 0b0010101: { // F(MIN|MAX).D
                switch (funct3) {
                    case 0b000: {// FMIN
                        instr_context.alu_op = alu::AluOp::FMIN_D;
                        return;
                    }
                    case 0b001: {// FMAX
                        instr_context.alu_op = alu::AluOp::FMAX_D;
                        return;
                    }
                }
                break;
            }
            case 0b1010000: { // F(EQ|LT|LE).S
                instr_context.reg_write_to_fpr = false;

                switch (funct3) {
                    case 0b010: {// FEQ
                        instr_context.alu_op = alu::AluOp::FEQ_S;
                        return;
                    }
                    case 0b001: {// FLT
                        instr_context.alu_op = alu::AluOp::FLT_S;
                        return;
                    }
                    case 0b000: {// FLE
                        instr_context.alu_op = alu::AluOp::FLE_S;
                        return;
                    }
                }
                break;
            }
            case 0b1010001: { // F(EQ|LT|LE).D
                instr_context.reg_write_to_fpr = false;

                switch (funct3) {
                    case 0b010: {// FEQ
                        instr_context.alu_op = alu::AluOp::FEQ_D;
                        return;
                    }
                    case 0b001: {// FLT
                        instr_context.alu_op = alu::AluOp::FLT_D;
                        return;
                    }
                    case 0b000: {// FLE
                        instr_context.alu_op = alu::AluOp::FLE_D;
                        return;
                    }
                }
                break;
            }
            case 0b1111000: { // FMV.W.X
                instr_context.uses_rs2 = false;
                instr_context.rs1_from_fprf = false;

                instr_context.alu_op = alu::AluOp::FMV_W_X;
                return;
            }
            case 0b1111001: { //FMV.D.X
                instr_context.uses_rs2 = false;
                instr_context.rs1_from_fprf = false;

                instr_context.alu_op = alu::AluOp::FMV_D_X;
                return;
            }
            case 0b1110000: { // FMV.X.W, FCLASS.S
                instr_context.reg_write_to_fpr = false;
                instr_context.uses_rs2 = false;

                switch (funct3) {
                    case 0b000: {
                        instr_context.alu_op = alu::AluOp::FMV_X_W;
                        return;
                    }
                    case 0b001: {
                        instr_context.alu_op = alu::AluOp::FCLASS_S;
                        return;
                    }
                }
                break;
            }
            case 0b1110001: { // FMV.X.D, FCLASS.D
                instr_context.reg_write_to_fpr = false;
                instr_context.uses_rs2 = false;

                switch (funct3) {
                    case 0b000: {
                        instr_context.alu_op = alu::AluOp::FMV_X_D;
                        return;
                    }
                    case 0b001: {
                        instr_context.alu_op = alu::AluOp::FCLASS_D;
                        return;
                    }
                }
                break;
            }
            // TODO: what the fuck
            case 0b1000011: { // FMADD.S, FMADD.D

                switch (funct2) {
                    case 0b00: {// FMADD.S
                        instr_context.alu_op = alu::AluOp::kFmadd_s;
                        return;
                    }
                    case 0b01: {// FMADD.D
                        instr_context.alu_op = alu::AluOp::FMADD_D;
                        return;
                    }
                }
                break;
            }
            case 0b1000111: { // FMSUB.S, FMSUB.D
                switch (funct2) {
                    case 0b00: {// FMSUB.S
                        instr_context.alu_op = alu::AluOp::kFmsub_s;
                        return;
                    }
                    case 0b01: {// FMSUB.D
                        instr_context.alu_op = alu::AluOp::FMSUB_D;
                        return;
                    }
                }
                break;
            }
            case 0b1001011: { // FNMADD.S, FNMADD.D
                switch (funct2) {
                    case 0b00: {// FNMADD.S
                        instr_context.alu_op = alu::AluOp::kFnmadd_s;
                        return;
                    }
                    case 0b01: {// FNMADD.D
                        instr_context.alu_op = alu::AluOp::FNMADD_D;
                        return;
                    }
                }
                break;
            }
            case 0b1001111: { // FNMSUB.S, FNMSUB.D
                switch (funct2) {
                    case 0b00: {// FNMSUB.S
                        instr_context.alu_op = alu::AluOp::kFnmsub_s;
                        return;
                    }
                    case 0b01: {// FNMSUB.D
                        instr_context.alu_op = alu::AluOp::FNMSUB_D;
                        return;
                    }
                }
                break;
            }
        }
        break;
    }
    
    case 0b1000011 : { // FMADD.S, FMADD.D
        instr_context.uses_rs1 = true;
        instr_context.uses_rs2 = true;
        instr_context.uses_rs3 = true;
        instr_context.reg_write = true;
        instr_context.reg_write_to_fpr = true;
        instr_context.rs1_from_fprf = true;
        instr_context.rs2_from_fprf = true;

        switch (funct2) {
            case 0b00: {// FMADD.S
                instr_context.alu_op = alu::AluOp::kFmadd_s;
                return;
            }
            case 0b01: {// FMADD.D
                instr_context.alu_op = alu::AluOp::FMADD_D;
                return;
            }
        }
        break;
    }
    case 0b1000111 : {  // FMSUB.S, FMSUB.D
        instr_context.uses_rs1 = true;
        instr_context.uses_rs2 = true;
        instr_context.uses_rs3 = true;
        instr_context.reg_write = true;
        instr_context.reg_write_to_fpr = true;
        instr_context.rs1_from_fprf = true;
        instr_context.rs2_from_fprf = true;

        switch (funct2) {
            case 0b00: {// FMSUB.S
                instr_context.alu_op = alu::AluOp::kFmsub_s;
                return;
            }
            case 0b01: {// FMSUB.D
                instr_context.alu_op = alu::AluOp::FMSUB_D;
                return;
            }
        }
        break;
    }
    case 0b1001011 : { // FNMADD.S, FNMADD.D
        instr_context.uses_rs1 = true;
        instr_context.uses_rs2 = true;
        instr_context.uses_rs3 = true;
        instr_context.reg_write = true;
        instr_context.reg_write_to_fpr = true;
        instr_context.rs1_from_fprf = true;
        instr_context.rs2_from_fprf = true;

        switch (funct2) {
            case 0b00: {// FNMADD.S
                instr_context.alu_op = alu::AluOp::kFnmadd_s;
                return;
            }
            case 0b01: {// FNMADD.D
                instr_context.alu_op = alu::AluOp::FNMADD_D;
                return;
            }
        }
        break;
    }
    case 0b1001111 : { // FNMSUB.S, FNMSUB.D
        instr_context.uses_rs1 = true;
        instr_context.uses_rs2 = true;
        instr_context.uses_rs3 = true;
        instr_context.reg_write = true;
        instr_context.reg_write_to_fpr = true;
        instr_context.rs1_from_fprf = true;
        instr_context.rs2_from_fprf = true;

        switch (funct2) {
            case 0b00: {// FNMSUB.S
                instr_context.alu_op = alu::AluOp::kFnmsub_s;
                return;
            }
            case 0b01: {// FNMSUB.D
                instr_context.alu_op = alu::AluOp::FNMSUB_D;
                return;
            }
        }
        break;
    }
    
    case 0b0000111: {// F-Type Load
        instr_context.imm_to_alu = true;
        instr_context.reg_write = true;
        instr_context.reg_write_to_fpr = true;
        instr_context.mem_read = true;
        instr_context.mem_to_reg = true;
        instr_context.uses_rs1 = true;

        switch (funct3) {
            case 0b010: {// FLW
                instr_context.alu_op = alu::AluOp::kAdd;
                instr_context.mem_access_bytes = 4;
                return;
            }
            case 0b011: {// FLD
                instr_context.alu_op = alu::AluOp::kAdd;
                instr_context.mem_access_bytes = 8;
                return;
            }
        }
        break;
    }

    case 0b0100111: {// F-Type Store
        instr_context.imm_to_alu = true;
        instr_context.mem_write = true;
        instr_context.rs2_from_fprf = true;
        instr_context.uses_rs1 = true;
        instr_context.uses_rs2 = true;

        switch (funct3) {
        case 0b010: {// FSW
            instr_context.mem_access_bytes = 4;
            instr_context.alu_op = alu::AluOp::kAdd;
            return;
        }
        case 0b011: {// FSD
            instr_context.mem_access_bytes = 8;
            instr_context.alu_op = alu::AluOp::kAdd;
            return;
        }
        default:
            break;
        }
        break;
    }
    }   // switch opcode
    
    instr_context.alu_op = alu::AluOp::kNone;
    return;
}

} // namespace DecoderHelper