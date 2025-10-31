/**
 * @file rvss_control_unit.cpp
 * @brief RVSS Control Unit implementation
 * @author Vishank Singh, https://github.com/VishankSingh
 */

#include "vm/rv5s/decoder/rv5s_decode_unit.h"
#include "vm/rv5s/decoder/decoder_helper.h"

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
    instr_context.rs2_value = rf.ReadGpr(instr_context.rs2);

    instr_context.frs1_value = rf.ReadFpr(instr_context.rs1);
    instr_context.frs2_value = rf.ReadFpr(instr_context.rs2);
    instr_context.frs3_value = rf.ReadFpr(instr_context.frs3);
    
    instr_context.immediate = ImmGenerator(instr_context);
    
    // csr related
    instr_context.csr_rd = (instr_context.instruction >> 20) & 0xFFF;
    instr_context.csr_value = rf.ReadCsr(instr_context.csr_rd);
    instr_context.csr_uimm = instr_context.rs1;
    instr_context.csr_write_val = instr_context.rs1_value;
}


void RV5SDecodeUnit::SetMemValues(InstrContext& instr_context){
    instr_context.mem_access_bytes = 0;
    
    if(!instr_context.mem_read && !instr_context.mem_write)
    return;
    
    if(instruction_set::isFInstruction(instr_context.instruction)){
        instr_context.mem_access_bytes = 4;
    }
    else if(instruction_set::isDInstruction(instr_context.instruction)){
        instr_context.mem_access_bytes = 8;
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
    
    DecoderHelper::SetContextValues(instr_context);
    
    // csr related
    if(instr_context.opcode==0b1110011){
        instr_context.csr_op = true;
    }
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
