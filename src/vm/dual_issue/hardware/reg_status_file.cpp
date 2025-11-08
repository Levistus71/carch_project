#include "vm/dual_issue/hardware/reg_status_file.h"

namespace dual_issue
{

TagFile::TagFile() {
    gpr_idxs.fill(0);
    fpr_idxs.fill(0);

    gpr_valid.fill(false);
    fpr_valid.fill(false);
}

void TagFile::Reset() {
    gpr_idxs.fill(0);
    fpr_idxs.fill(0);

    gpr_valid.fill(false);
    fpr_valid.fill(false);
}

std::pair<bool, uint64_t> TagFile::GetRobIdxGpr(uint64_t reg) const {
    if (reg >= NUM_GPR) throw std::out_of_range("Invalid GPR index");

    // no dependency with 0
    if(reg==0)
        return {false, gpr_idxs[0]};
    
    return {gpr_valid[reg], gpr_idxs[reg]};
}

std::pair<bool, uint64_t> TagFile::GetRobIdxFpr(uint64_t reg) const {
    if (reg >= NUM_FPR) throw std::out_of_range("Invalid FPR index");

    return {fpr_valid[reg], fpr_idxs[reg]};
}

void TagFile::WriteGprRobIdx(uint64_t reg, uint64_t rob_idx){
    if (reg >= NUM_FPR) throw std::out_of_range("Invalid GPR index");

    gpr_valid[reg] = true;
    gpr_idxs[reg] = rob_idx;
}

void TagFile::WriteFprRobIdx(uint64_t reg, uint64_t rob_idx){
    if (reg >= NUM_FPR) throw std::out_of_range("Invalid FPR index");

    fpr_valid[reg] = true;
    fpr_idxs[reg] = rob_idx;
}

void TagFile::EndDependencyGpr(uint64_t rd_reg, uint64_t rob_idx){
    if (rd_reg >= NUM_GPR) throw std::out_of_range("Invalid GPR index");

    if(gpr_idxs[rd_reg]==rob_idx)
        gpr_valid[rd_reg] = false;
}
void TagFile::EndDependencyFpr(uint64_t rd_reg, uint64_t rob_idx){
    if (rd_reg >= NUM_FPR) throw std::out_of_range("Invalid FPR index");

    if(fpr_idxs[rd_reg]==rob_idx)
        fpr_valid[rd_reg] = false;
}


void RegisterStatusFile::UpdateTableRobIdx(uint8_t reg_num, bool gpr_register, uint64_t rob_idx){
    try{
        if(gpr_register){
            tag_file_.WriteGprRobIdx(reg_num, rob_idx);
        }
        else{
            tag_file_.WriteFprRobIdx(reg_num, rob_idx);
        }
    }
    catch(const std::out_of_range& e){
        throw e;
    }
}

void RegisterStatusFile::EndDependency(size_t rd_reg_num, size_t rob_idx, bool gpr_register){
    try{
        if(gpr_register){
            tag_file_.EndDependencyGpr(rd_reg_num, rob_idx);
        }
        else{
            tag_file_.EndDependencyFpr(rd_reg_num, rob_idx);
        }
    }
    catch(const std::out_of_range& e){
        throw e;
    }
}

std::pair<bool, uint64_t> RegisterStatusFile::QueryTableRobIdx(uint8_t reg_num, bool gpr_register){
    try{
        if(gpr_register){
            return tag_file_.GetRobIdxGpr(reg_num);
        }
        else{
            return tag_file_.GetRobIdxFpr(reg_num);
        }
    }
    catch(const std::out_of_range& e){
        throw e;
    }
}

} // namespace dual_issue
