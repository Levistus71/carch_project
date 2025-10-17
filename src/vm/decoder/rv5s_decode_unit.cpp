/**
 * @file rvss_control_unit.cpp
 * @brief RVSS Control Unit implementation
 * @author Vishank Singh, https://github.com/VishankSingh
 */

#include "vm/decoder/rv5s_decode_unit.h"

using instruction_set::Instruction;
using instruction_set::get_instr_encoding;


void RV5SDecodeUnit::DecodeInstruction(InstrContext& instr_context, RegisterFile& rf){
    DecodeInstrFields(instr_context);
    SetRegImmValues(instr_context, rf);
    SetContextValues(instr_context);

    // this function should be called after SetContextValues(InstrContext);
    SetMemValues(instr_context);
}


void RV5SDecodeUnit::DecodeInstrFields(InstrContext& instr_context){
    instr_context.opcode = instr_context.instruction & 0b1111111;

    instr_context.funct2 = (instr_context.instruction >> 25) & 0b11;
    instr_context.funct3 = (instr_context.instruction >> 12) & 0b111;
    instr_context.funct5 = (instr_context.instruction >> 20) & 0b11111;
    instr_context.funct7 = (instr_context.instruction >> 25) & 0b1111111;

    instr_context.rs1 = (instr_context.instruction >> 15) & 0b11111;
    instr_context.rs2 = (instr_context.instruction >> 20) & 0b11111;
    instr_context.frs3 = (instr_context.instruction >> 27) & 0b11111;

    instr_context.rd = (instr_context.instruction >> 7) & 0b11111; 
}


void RV5SDecodeUnit::SetRegImmValues(InstrContext& instr_context, RegisterFile& rf){
    // the values are correct because the register is uint8_t, the first 3 bits are always 0. (32 : 00011111)
    // ReadGpr uses size_t. implicit sign extension retains the correct register value.
    
    instr_context.rs1_value = rf.ReadGpr(instr_context.rs1);
    instr_context.rs2_value = rf.ReadGpr(instr_context.rs1);

    instr_context.frs1_value = rf.ReadFpr(instr_context.rs1);
    instr_context.frs2_value = rf.ReadFpr(instr_context.rs2);
    instr_context.frs3_value = rf.ReadFpr(instr_context.frs3);

    instr_context.immediate = ImmGenerator(instr_context);
}


void RV5SDecodeUnit::SetMemValues(InstrContext& instr_context){
    instr_context.mem_access_bytes = 0;
    instr_context.mem_write_data_from_gpr = true;

    if(!instr_context.mem_read && !instr_context.mem_write)
        return;
    
    if(instruction_set::isFInstruction(instr_context.instruction)){
        instr_context.mem_access_bytes = 4;
        instr_context.mem_write_data_from_gpr = false;
    }
    else if(instruction_set::isDInstruction(instr_context.instruction)){
        instr_context.mem_access_bytes = 8;
        instr_context.mem_write_data_from_gpr = false;
    }
    else{
        switch (instr_context.funct3) {
            case 0b000: {// SB
                instr_context.mem_access_bytes = 1;
                break;
            }
            case 0b001: {// SH
                instr_context.mem_access_bytes = 2;
                break;
            }
            case 0b010: {// SW
                instr_context.mem_access_bytes = 4;
                break;
            }
            case 0b011: {// SD
                instr_context.mem_access_bytes = 8;
                break;
            }
		}
    }
}


void RV5SDecodeUnit::SetContextValues(InstrContext& instr_context) {

  switch (instr_context.opcode) {
    case 0b0110011:{ // R-type (kAdd, kSub, kAnd, kOr, kXor, kSll, kSrl, etc.)
      instr_context.reg_write = true;
      break;
    }
    case 0b0000011: { // Load instructions (LB, LH, LW, LD)
      instr_context.imm_to_alu = true;
      instr_context.mem_to_reg = true;
      instr_context.reg_write = true;
      instr_context.mem_read= true;
    
      switch(instr_context.funct3){
        case(0x0):{     // LB
            instr_context.sign_extend = true;
            break;
        }
        case (0x1): {   // LH
            instr_context.sign_extend = true;
            break;
        }
        case (0x2): {   // LW
            instr_context.sign_extend = true;
        }
        default:    // LD, LBU, LHU, LWU
            break;
      }

      break;
    }
    case 0b0100011: {// Store instructions (SB, SH, SW, SD)
      instr_context.imm_to_alu = true;
      instr_context.mem_write = true;
      break;
    }
    case 0b1100011: {// branch_ instructions (BEQ, BNE, BLT, BGE)
      instr_context.branch = true;
      break;
    }
    case 0b0010011: {// I-type alu instructions (ADDI, ANDI, ORI, XORI, SLTI, SLLI, SRLI)
      instr_context.imm_to_alu = true;
      instr_context.reg_write = true;
      break;
    }
    case 0b0110111: {// LUI (Load Upper Immediate)
      instr_context.imm_to_alu = true;
      instr_context.reg_write = true;
      break;
    }
    case 0b0010111: {// AUIPC (Add Upper Immediate to PC)
      instr_context.imm_to_alu = true;
      instr_context.reg_write = true;
      instr_context.auipc = true;
      break;
    }
    case 0b1101111: {// JAL (Jump and Link)
      instr_context.reg_write = true;
      instr_context.branch = true;
      break;
    }
    case 0b1100111: {// JALR (Jump and Link Register)
      instr_context.imm_to_alu = true;
      instr_context.reg_write = true;
      instr_context.branch = true;
      break;
    }
    // WHAT? this is definetly not kMUL lol
    case 0b0000001: {// kMul
      instr_context.reg_write = true;
      break;
    }

    // F extension + D extension
    case 0b0000111: {// F-Type Load instructions (FLW, FLD)
      instr_context.imm_to_alu = true;
      instr_context.mem_to_reg = true;
      instr_context.reg_write = true;
      instr_context.reg_write_to_fpr = true;
      instr_context.mem_read = true;
      break;
    }
    case 0b0100111: {// F-Type Store instructions (FSW, FSD)
      instr_context.imm_to_alu = true;
      instr_context.mem_write = true;
      break;
    }
    case 0b1010011: {// F-Type R-type instructions (FADD, FSUB, FMUL, FDIV, etc.)
      instr_context.reg_write = true;
      instr_context.reg_write_to_fpr = true;
      
      switch(instr_context.funct7){
        case(0b1010000) : { // kfle_s, kflt_s, kfeq_s
            instr_context.reg_write_to_fpr = false;
            break;
        }
        case(0b1100000) : { // kfcvt_w_s, kfcvt_wu_s, kfcvt_l_s, kfcvt_lu_s
            instr_context.reg_write_to_fpr = false;
            break;
        }
        case(0b1110000) : { // kfmv_x_w, kfclass_s
            instr_context.reg_write_to_fpr = false;
            break;
        }
        case(0b1010001) : { // kfle_d, kflt_d, kfeq_d
            instr_context.reg_write_to_fpr = false;
            break;
        }
        case(0b1100001) : { // kfcvt_w_d, kfcvt_wu_d, kfcvt_l_d, kfcvt_lu_d
            instr_context.reg_write_to_fpr = false;
            break;
        }
        case (0b1110001) : { // kfmv_x_d, kfclass_d
            instr_context.reg_write_to_fpr = false;
            break;
        }
      }
      break;
    }

    default:
      break;
  }

    
}

alu::AluOp RV5SDecodeUnit::GetAluSignal(InstrContext& instr_context) {
    uint8_t& opcode = instr_context.opcode;
    uint8_t& funct2 = instr_context.funct2;
    uint8_t& funct3 = instr_context.funct3;
    uint8_t& funct5 = instr_context.funct5;
    uint8_t& funct7 = instr_context.funct7;

    switch (opcode)
    {
    case 0b0110011: {// R-Type
        switch (funct3)
        {
        case 0b000:{ // kAdd, kSub, kMul
            switch (funct7)
            {
            case 0x0000000: {// kAdd
                return alu::AluOp::kAdd;
                break;
            }
            case 0b0100000: {// kSub
                return alu::AluOp::kSub;
                break;
            }
            case 0b0000001: {// kMul
                return alu::AluOp::kMul;
                break;
            }
            }
            break;
        }
        case 0b001: {// kSll, kMulh
            switch (funct7)
            {
            case 0b0000000: {// kSll
                return alu::AluOp::kSll;
                break;
            }
            case 0b0000001: {// kMulh
                return alu::AluOp::kMulh;
                break;
            }
            }
            break;
        }
        case 0b010: {// kSlt, kMulhsu
            switch (funct7)
            {
            case 0b0000000: {// kSlt
                return alu::AluOp::kSlt;
                break;
            }
            case 0b0000001: {// kMulhsu
                return alu::AluOp::kMulhsu;
                break;
            }
            }
            break;
        }
        case 0b011: {// kSltu, kMulhu
            switch (funct7)
            {
            case 0b0000000: {// kSltu
                return alu::AluOp::kSltu;
                break;
            }
            case 0b0000001: {// kMulhu
                return alu::AluOp::kMulhu;
                break;
            }
            }
            break;
        }
        case 0b100: {// kXor, kDiv
            switch (funct7)
            {
            case 0b0000000: {// kXor
                return alu::AluOp::kXor;
                break;
            }
            case 0b0000001: {// kDiv
                return alu::AluOp::kDiv;
                break;
            }
            }
            break;
        }
        case 0b101: {// kSrl, kSra, kDivu
            switch (funct7)
            {
            case 0b0000000: {// kSrl
                return alu::AluOp::kSrl;
                break;
            }
            case 0b0100000: {// kSra
                return alu::AluOp::kSra;
                break;
            }
            case 0b0000001: {// kDivu
                return alu::AluOp::kDivu;
                break;
            }
            }
            break;
        }
        case 0b110: {// kOr, kRem
            switch (funct7)
            {
            case 0b0000000: {// kOr
                return alu::AluOp::kOr;
                break;
            }
            case 0b0000001: {// kRem
                return alu::AluOp::kRem;
                break;
            }
            }
            break;
        }
        case 0b111: {// kAnd, kRemu
            switch (funct7)
            {
            case 0b0000000: {// kAnd
                return alu::AluOp::kAnd;
                break;
            }
            case 0b0000001: {// kRemu
                return alu::AluOp::kRemu;
                break;
            }
            }
            break;
        }
        }
        break;
    }
    case 0b0010011: {// I-Type
        switch (funct3)
        {
        case 0b000: {// ADDI
            return alu::AluOp::kAdd;
            break;
        }
        case 0b001: {// SLLI
            return alu::AluOp::kSll;
            break;
        }
        case 0b010: {// SLTI
            return alu::AluOp::kSlt;
            break;
        }
        case 0b011: {// SLTIU
            return alu::AluOp::kSltu;
            break;
        }
        case 0b100: {// XORI
            return alu::AluOp::kXor;
            break;
        }
        case 0b101: {// SRLI & SRAI
            switch (funct7)
            {
            case 0b0000000: {// SRLI
                return alu::AluOp::kSrl;
                break;
            }
            case 0b0100000: {// SRAI
                return alu::AluOp::kSra;
                break;
            }
            }
            break;
        }
        case 0b110: {// ORI
            return alu::AluOp::kOr;
            break;
        }
        case 0b111: {// ANDI
            return alu::AluOp::kAnd;
            break;
        }
        }
        break;
    }
    case 0b1100011: {// B-Type
        switch (funct3)
        {
        case 0b000: {// BEQ
            return alu::AluOp::kSub;
            break;
        }
        case 0b001: {// BNE
            return alu::AluOp::kSub;
            break;
        }
        case 0b100: {// BLT
            return alu::AluOp::kSlt;
            break;
        }
        case 0b101: {// BGE
            return alu::AluOp::kSlt;
            break;
        }
        case 0b110: {// BLTU
            return alu::AluOp::kSltu;
            break;
        }
        case 0b111: {// BGEU
            return alu::AluOp::kSltu;
            break;
        }
        }
        break;
    }
    case 0b0000011: {// Load
        return alu::AluOp::kAdd;
        break;
    }
    case 0b0100011: {// Store
        return alu::AluOp::kAdd;
        break;
    }
    case 0b1100111: {// JALR
        return alu::AluOp::kAdd;
        break;
    }
    case 0b1101111: {// JAL
        return alu::AluOp::kAdd;
        break;
    }
    case 0b0110111: {// LUI
        return alu::AluOp::kLui;
        break;
    }
    case 0b0010111: {// AUIPC
        return alu::AluOp::kAuipc;
        break;
    }
    case 0b0000000: {// FENCE
        return alu::AluOp::kNone;
        break;
    }
    case 0b1110011: {// SYSTEM
        switch (funct3) 
        {
        case 0b000: // ECALL
            return alu::AluOp::kNone;
            break;
        case 0b001: // CSRRW
            return alu::AluOp::kNone;
            break;
        default:
            break;
        }
        break;
    }
    case 0b0011011: {// R4-Type
        switch (funct3) 
        {
            case 0b000: {// ADDIW
                return alu::AluOp::kAddw;
                break;
            }
            case 0b001: {// SLLIW
                return alu::AluOp::kSllw;
                break;
            }
            case 0b101: {// SRLIW & SRAIW
                switch (funct7) 
                {
                case 0b0000000: {// SRLIW
                    return alu::AluOp::kSrlw;
                    break;
                }
                case 0b0100000: {// SRAIW
                        return alu::AluOp::kSraw;
                        break;
                    }
                }
                break;
            }
        }
        break;
    }
    case 0b0111011: {// R4-Type
        switch (funct3) {
        case 0b000: {// kAddw, kSubw, kMulw
            switch (funct7) 
            {
            case 0b0000000: {// kAddw
                return alu::AluOp::kAddw;
                break;
            }
            case 0b0100000: {// kSubw
                return alu::AluOp::kSubw;
                break;
            }
            case 0b0000001: {// kMulw
                return alu::AluOp::kMulw;
                break;
            }
            }
            break;
        }
        case 0b001: {// kSllw
            return alu::AluOp::kSllw;
            break;
        }
        case 0b100: {// kDivw
            switch (funct7) {// kDivw
                case 0b0000001: {// kDivw
                    return alu::AluOp::kDivw;
                    break;
                }
            }
            break;
        }
        case 0b101: {// kSrlw, kSraw, kDivuw
            switch (funct7) {
                case 0b0000000: {// kSrlw
                    return alu::AluOp::kSrlw;
                    break;
                }
                case 0b0100000: {// kSraw
                    return alu::AluOp::kSraw;
                    break;
                }
                case 0b0000001: {// kDivuw
                    return alu::AluOp::kDivuw;
                    break;
                }
            }
            break;
        }
        case 0b110: {// kRemw
            switch (funct7) 
            {
            case 0b0000001: {// kRemw
                return alu::AluOp::kRemw;
                break;
            }
            }
            break;
        }
        case 0b111: {// kRemuw
                switch (funct7) {
                    case 0b0000001: {// kRemuw
                        return alu::AluOp::kRemuw;
                        break;
                    }
                }
                break;
            }
        }
        break;
    }
    
    // F extension + D extension
    // TODO: correct this

    case 0b1000011: {
        return alu::AluOp::kFmadd_s;
    }

    case 0b1010011: {
        switch (funct7) {
            case 0b0000000: {// FADD_S
                return alu::AluOp::FADD_S;
            }
            case 0b0000001: {// FADD_D
                return alu::AluOp::FADD_D;
            }
            case 0b0000100: {// FSUB_S
                return alu::AluOp::FSUB_S;
            }
            case 0b0000101: {// FSUB_D
                return alu::AluOp::FSUB_D;
            }
            case 0b0001000: {// FMUL_S
                return alu::AluOp::FMUL_S;
            }
            case 0b0001001: {// FMUL_D
                return alu::AluOp::FMUL_D;
            }
            case 0b0001100: {// FDIV_S
                return alu::AluOp::FDIV_S;
            }
            case 0b0001101: {// FDIV_D
                return alu::AluOp::FDIV_D;
            }
            case 0b0101100: {// FSQRT_S
                return alu::AluOp::FSQRT_S;
            }
            case 0b0101101: {// FSQRT_D
                return alu::AluOp::FSQRT_D;
            }
            case 0b1100000: { // FCVT.(W|WU|L|LU).S
                switch (funct5) {
                    case 0b00000: {// FCVT_W_S
                        return alu::AluOp::FCVT_W_S;
                    }
                    case 0b00001: {// FCVT_WU_S
                        return alu::AluOp::FCVT_WU_S;
                    }
                    case 0b00010: {// FCVT_L_S
                        return alu::AluOp::FCVT_L_S;
                    }
                    case 0b00011: {// FCVT_LU_S
                        return alu::AluOp::FCVT_LU_S;
                    }
                }
                break;
            }
            case 0b1100001: { // FCVT.(W|WU|L|LU).D
                switch (funct5) {
                    case 0b00000: {// FCVT_W_D
                        return alu::AluOp::FCVT_W_D;
                    }
                    case 0b00001: {// FCVT_WU_D
                        return alu::AluOp::FCVT_WU_D;
                    }
                    case 0b00010: {// FCVT_L_D
                        return alu::AluOp::FCVT_L_D;
                    }
                    case 0b00011: {// FCVT_LU_D
                        return alu::AluOp::FCVT_LU_D;
                    }
                }
                break;
            }
            case 0b1101000: { // FCVT.S.(W|WU|L|LU)
                switch (funct5) {
                    case 0b00000: {// FCVT_S_W
                        return alu::AluOp::FCVT_S_W;
                    }
                    case 0b00001: {// FCVT_S_WU
                        return alu::AluOp::FCVT_S_WU;
                    }
                    case 0b00010: {// FCVT_S_L
                        return alu::AluOp::FCVT_S_L;
                    }
                    case 0b00011: {// FCVT_S_LU
                        return alu::AluOp::FCVT_S_LU;
                    }
                }
                break;
            }
            case 0b1101001: { // FCVT.D.(W|WU|L|LU)
                switch (funct5) {
                    case 0b00000: {// FCVT_D_W
                        return alu::AluOp::FCVT_D_W;
                    }
                    case 0b00001: {// FCVT_D_WU
                        return alu::AluOp::FCVT_D_WU;
                    }
                    case 0b00010: {// FCVT_D_L
                        return alu::AluOp::FCVT_D_L;
                    }
                    case 0b00011: {// FCVT_D_LU
                        return alu::AluOp::FCVT_D_LU;
                    }
                }
                break;
            }
            case 0b0010000: { // FSGNJ(N|X).S
                switch (funct3) {
                    case 0b000: {// FSGNJ
                        return alu::AluOp::FSGNJ_S;
                    }
                    case 0b001: {// FSGNJN
                        return alu::AluOp::FSGNJN_S;
                    }
                    case 0b010: {// FSGNJX
                        return alu::AluOp::FSGNJX_S;
                    }
                }
                break;
            }
            case 0b0010001: { // FSGNJ(N|X).D
                switch (funct3) {
                    case 0b000: {// FSGNJ
                        return alu::AluOp::FSGNJ_D;
                    }
                    case 0b001: {// FSGNJN
                        return alu::AluOp::FSGNJN_D;
                    }
                    case 0b010: {// FSGNJX
                        return alu::AluOp::FSGNJX_D;
                    }
                }
                break;
            }
            case 0b0010100: { // F(MIN|MAX).S
                switch (funct3) {
                    case 0b000: {// FMIN
                        return alu::AluOp::FMIN_S;
                    }
                    case 0b001: {// FMAX
                        return alu::AluOp::FMAX_S;
                    }
                }
                break;
            }
            case 0b0010101: { // F(MIN|MAX).D
                switch (funct3) {
                    case 0b000: {// FMIN
                        return alu::AluOp::FMIN_D;
                    }
                    case 0b001: {// FMAX
                        return alu::AluOp::FMAX_D;
                    }
                }
                break;
            }
            case 0b1010000: { // F(EQ|LT|LE).S
                switch (funct3) {
                    case 0b010: {// FEQ
                        return alu::AluOp::FEQ_S;
                    }
                    case 0b001: {// FLT
                        return alu::AluOp::FLT_S;
                    }
                    case 0b000: {// FLE
                        return alu::AluOp::FLE_S;
                    }
                }
                break;
            }
            case 0b1010001: { // F(EQ|LT|LE).D
                switch (funct3) {
                    case 0b010: {// FEQ
                        return alu::AluOp::FEQ_D;
                    }
                    case 0b001: {// FLT
                        return alu::AluOp::FLT_D;
                    }
                    case 0b000: {// FLE
                        return alu::AluOp::FLE_D;
                    }
                }
                break;
            }
            case 0b1111000: { // FMV.W.X
                return alu::AluOp::FMV_W_X;
            }
            case 0b1111001: { //FMV.D.X
                return alu::AluOp::FMV_D_X;
            }
            case 0b1110000: { // FMV.X.W, FCLASS.S
                switch (funct3) {
                    case 0b000: {
                        return alu::AluOp::FMV_X_W;
                    }
                    case 0b001: {
                        return alu::AluOp::FCLASS_S;
                    }
                }
                break;
            }
            case 0b1110001: { // FMV.X.D, FCLASS.D
                switch (funct3) {
                    case 0b000: {
                        return alu::AluOp::FMV_X_D;
                    }
                    case 0b001: {
                        return alu::AluOp::FCLASS_D;
                    }
                }
                break;
            }
            case 0b1000011: { // FMADD.S, FMADD.D
                switch (funct2) {
                    case 0b00: {// FMADD.S
                        return alu::AluOp::kFmadd_s;
                    }
                    case 0b01: {// FMADD.D
                        return alu::AluOp::FMADD_D;
                    }
                }
                break;
            }
            case 0b1000111: { // FMSUB.S, FMSUB.D
                switch (funct2) {
                    case 0b00: {// FMSUB.S
                        return alu::AluOp::kFmsub_s;
                    }
                    case 0b01: {// FMSUB.D
                        return alu::AluOp::FMSUB_D;
                    }
                }
                break;
            }
            case 0b1001011: { // FNMADD.S, FNMADD.D
                switch (funct2) {
                    case 0b00: {// FNMADD.S
                        return alu::AluOp::kFnmadd_s;
                    }
                    case 0b01: {// FNMADD.D
                        return alu::AluOp::FNMADD_D;
                    }
                }
                break;
            }
            case 0b1001111: { // FNMSUB.S, FNMSUB.D
                switch (funct2) {
                    case 0b00: {// FNMSUB.S
                        return alu::AluOp::kFnmsub_s;
                    }
                    case 0b01: {// FNMSUB.D
                        return alu::AluOp::FNMSUB_D;
                    }
                }
                break;
            }
        }
        break;
    }
    
    case 0b0000111: {// F-Type Load
        switch (funct3) {
        case 0b010: {// FLW
            return alu::AluOp::kAdd;
        }
        case 0b011: {// FLD
            return alu::AluOp::kAdd;
        }
        }
        break;
    }

    case 0b0100111: {// F-Type Store
        switch (funct3) {
        case 0b010: {// FSW
            return alu::AluOp::kAdd;
            break;
        }
        case 0b011: {// FSD
            return alu::AluOp::kAdd;
            break;
        }
        default:
            break;
        }
        break;
    }

    
    }
    
    return alu::AluOp::kNone;
}



int32_t RV5SDecodeUnit::ImmGenerator(InstrContext& instr_context){
    int32_t imm = 0;
    uint32_t& instruction = instr_context.instruction;

    auto sign_extend = [](uint32_t value, unsigned int bits) -> int32_t {
        int32_t mask = 1 << (bits - 1);
        return (value ^ mask) - mask;
    };

    switch (instr_context.opcode) {
        /*** I-TYPE (Load, alu Immediate, JALR, FPU Loads) ***/
        case 0b0010011: // alu Immediate (ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI)
        case 0b0000011: // Load (LB, LH, LW, LD, LBU, LHU, LWU)
        case 0b1100111: // JALR
        case 0b0001111: // FENCE
        case 0b0000111: // FLW, FLD (Floating-point load)
            imm = (instruction >> 20) & 0xFFF;
            imm = sign_extend(imm, 12);
            break;

        /*** S-TYPE (Store, Floating-Point Store) ***/
        case 0b0100011: // Store (SB, SH, SW, SD)
        case 0b0100111: // FSW, FSD (Floating-point store)
            imm = ((instruction >> 7) & 0x1F) | ((instruction >> 25) & 0x7F) << 5;
            imm = sign_extend(imm, 12);
            break;

        /*** SB-TYPE (branch_ Instructions) ***/
        case 0b1100011: // branch_ (BEQ, BNE, BLT, BGE, BLTU, BGEU)
            imm = ((instruction >> 8) & 0xF) // Bits 11:8
                  | ((instruction >> 25) & 0x3F) << 4 // Bits 10:5
                  | ((instruction >> 7) & 0x1) << 10 // Bit 4
                  | ((instruction >> 31) & 0x1) << 11; // Bit 12
            imm <<= 1;
            imm = sign_extend(imm, 13);
            break;

        /*** U-TYPE (LUI, AUIPC) ***/
        case 0b0110111: // LUI
        case 0b0010111: // AUIPC
            imm = (instruction & 0xFFFFF000) >> 12;  // Upper 20 bits
            
            break;

        /*** J-TYPE (JAL) ***/
        case 0b1101111: // JAL
            imm = ((instruction >> 21) & 0x3FF)  // Bits 10:1
                | ((instruction >> 20) & 0x1) << 10  // Bit 11
                | ((instruction >> 12) & 0xFF) << 11  // Bits 19:12
                | ((instruction >> 31) & 0x1) << 19;  // Bit 20
            imm <<= 1;  // Shift left by 1
            // if (imm & 0x1000) {
            //    imm |= 0xFFFFE000;
            // }
            imm = sign_extend(imm, 21); // Might be 20
            break;

        /*** M-EXTENSION (Multiplication, Division) - R-TYPE ***/
        case 0b0110011: // kMul, kMulh, kMulhu, kMulhsu, kDiv, kDivu, kRem, kRemu
            // R-Type (no immediate needed)
            imm = 0;
            break;

        /*** F-EXTENSION (Floating Point Operations) - R-TYPE ***/
        case 0b1010011: // Floating-point (FADD, FSUB, FMUL, FDIV, FSQRT, etc.)
            // R-Type (no immediate needed)
            imm = 0;
            break;

        default:
            imm = 0;
            break;
    }

    return imm;
}
