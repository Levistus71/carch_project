#pragma once
#include "../core/core.h"

namespace dual_issue{


/**
 * An instruction should follow this path:
 * if_id_2 -> if_id_1 -> id_issue_2 -> id_issue_1 -> Rest
 * 
 * id_issue_1 is the oldest instruction. if_id_2 is the youngest.
 * 
 */

class DualIssueStages{
public:
    /**
     * If num_fetch is 2, it stores both of them in if-id
     * If num_fetch is 1, it stores it in if_id_2 (youngest instruction)
     * If num_fetch is 0, it doesn't fetch (no update to if_id_1/if_id_2)
     */
    static void Fetch(DualIssueCore& vm_core, int num_fetch);


    /**
     * Decodes instructions in place from vm_core.pipeline_reg_instrs_.if_id_1 and if_id_2, doens't move anything
     */
    static void Decode(DualIssueCore& vm_core);

    /**
     * If both can be pushed into the reserve stations, it pushes both of them
     * If only 1 can be pushed, it stores the unpushed instruction in id_issue_1 (oldest unpushed instruction)
     * If nothing can be pushed, it does nothing
     * 
     * Returns the number of pushed instructions
     */
    static int Issue(DualIssueCore& vm_core);

    /**
     * The branch logic is done here. if the branch is predicted incorrectly, the vm_core.to_commit_tag_
     * gets updated here. The eviction is done in commit_buffer
     */
    static void Execute(DualIssueCore& vm_core);

    static void MemoryAccess(DualIssueCore& vm_core);

    static void WriteBack(DualIssueInstrContext& wb_instruction, DualIssueCore& vm_core);


private:
    // FIXME: this doesn't belong here
    static void HandleSyscall(DualIssueCore& vm_core);

    static void ResolveBranch(DualIssueInstrContext& instr, DualIssueCore& vm_core);
    static void ExecuteBasic(DualIssueCore& vm_core);
    static void ExecuteFloat(DualIssueCore& vm_core);
    static void ExecuteDouble(DualIssueCore& vm_core);

    static void WriteBackCsr(DualIssueInstrContext& wb_instruction, DualIssueCore& vm_core);
};


} // namespace rv5s