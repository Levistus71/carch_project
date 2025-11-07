#include "vm/dual_issue/hardware/reg_status_file.h"

namespace dual_issue
{

TagFile::TagFile() {
    gpr_tags.fill(0);
    fpr_tags.fill(0);

    gpr_valid.fill(false);
    fpr_valid.fill(false);

    gpr_ready.fill(false);
    fpr_ready.fill(false);

    gpr_values.fill(0);
    fpr_values.fill(0);
}

void TagFile::Reset() {
    gpr_tags.fill(0);
    fpr_tags.fill(0);

    gpr_valid.fill(false);
    fpr_valid.fill(false);

    gpr_ready.fill(false);
    fpr_ready.fill(false);

    gpr_values.fill(0);
    fpr_values.fill(0);
}

std::pair<bool, uint64_t> TagFile::GetTagGpr(uint64_t reg) const {
    if (reg >= NUM_GPR) throw std::out_of_range("Invalid GPR index");

    if(gpr_valid[reg]){
        return {true, gpr_tags[reg]};
    }
    return {false, gpr_tags[reg]};
}

std::pair<bool, uint64_t> TagFile::GetTagFpr(uint64_t reg) const {
    if (reg >= NUM_FPR) throw std::out_of_range("Invalid FPR index");

    if(fpr_valid[reg]){
        return {true, fpr_tags[reg]};
    }
    return {false, fpr_tags[reg]};
}

std::pair<bool, uint64_t> TagFile::GetValueGpr(uint64_t reg) const {
    if (reg >= NUM_GPR) throw std::out_of_range("Invalid GPR index");

    if(gpr_ready[reg]){
        return {true, gpr_values[reg]};
    }
    return {false, gpr_values[reg]};
}

std::pair<bool, uint64_t> TagFile::GetValueFpr(uint64_t reg) const {
    if (reg >= NUM_FPR) throw std::out_of_range("Invalid FPR index");

    if(fpr_ready[reg]){
        return {true, fpr_values[reg]};
    }
    return {false, fpr_values[reg]};
}

void TagFile::WriteGprTag(uint64_t reg, uint64_t tag){
    if (reg >= NUM_FPR) throw std::out_of_range("Invalid GPR index");

    gpr_valid[reg] = true;
    gpr_ready[reg] = false;
    gpr_tags[reg] = tag;
}

void TagFile::WriteFprTag(uint64_t reg, uint64_t tag){
    if (reg >= NUM_FPR) throw std::out_of_range("Invalid FPR index");

    fpr_valid[reg] = true;
    fpr_ready[reg] = false;
    fpr_tags[reg] = tag;
}

void TagFile::WriteGprValue(uint64_t reg, uint64_t val){
    if (reg >= NUM_FPR) throw std::out_of_range("Invalid GPR index");

    gpr_ready[reg] = true;
    gpr_values[reg] = val;
}

void TagFile::WriteFprValue(uint64_t reg, uint64_t val){
    if (reg >= NUM_FPR) throw std::out_of_range("Invalid FPR index");

    fpr_ready[reg] = true;
    fpr_values[reg] = val;
}


void RegisterStatusFile::UpdateTableTag(uint8_t reg_num, bool gpr_register, uint64_t tag){
    try{
        if(gpr_register){
            tag_file_.WriteGprTag(reg_num, tag);
        }
        else{
            tag_file_.WriteFprTag(reg_num, tag);
        }
    }
    catch(const std::out_of_range& e){
        throw e;
    }
}

void RegisterStatusFile::UpdateTableValue(uint8_t reg_num, bool gpr_register, uint64_t val){
    try{
        if(gpr_register){
            tag_file_.WriteGprValue(reg_num, val);
        }
        else{
            tag_file_.WriteFprValue(reg_num, val);
        }
    }
    catch(const std::out_of_range& e){
        throw e;
    }
}

std::pair<bool, uint64_t> RegisterStatusFile::QueryTableTag(uint8_t reg_num, bool gpr_register){
    try{
        if(gpr_register){
            tag_file_.GetTagGpr(reg_num);
        }
        else{
            tag_file_.GetTagFpr(reg_num);
        }
    }
    catch(const std::out_of_range& e){
        throw e;
    }
}


std::pair<bool, uint64_t> RegisterStatusFile::QueryTableValue(uint8_t reg_num, bool gpr_register){
    try{
        if(gpr_register){
            tag_file_.GetValueGpr(reg_num);
        }
        else{
            tag_file_.GetValueFpr(reg_num);
        }
    }
    catch(const std::out_of_range& e){
        throw e;
    }
}

} // namespace dual_issue
