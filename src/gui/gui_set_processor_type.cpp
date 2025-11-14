#include "../include/gui/gui_set_processor_type.h"

void set_processor_type(){

    static const char* PROCESSOR_TYPES[] = {"Single Cycle", "Pipelined W/O Hazard", "Pipelined With Hazard W/O Forwarding", "Pipelined With Hazard With Forwarding", "Dual issue (6 stage)", "Triple Issue (6 stage)"};
    static int PROCESSOR_IDX = 0;
    static bool PROCESSOR_CHANGE = false;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(100.0f/255.0f, 100.0f/255.0f, 100.0f/255.0f, 1.0f)); // Lighter background
    if(ImGui::BeginCombo("Processor Display Type", PROCESSOR_TYPES[PROCESSOR_IDX]))
    {
        for(int i=0;i<IM_ARRAYSIZE(PROCESSOR_TYPES);i++){
            const bool is_selected = (PROCESSOR_IDX == i);

            if(ImGui::Selectable(PROCESSOR_TYPES[i], is_selected)){
                PROCESSOR_IDX = i;
                PROCESSOR_CHANGE = true;
            }

            if(is_selected){
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleColor();

    static const char* BRANCH_TYPES[] = {"No Prediction", "Static Prediction", "Dynamic Prediction (1-bit)"};
    static int BRANCH_IDX = 0;

    if(PROCESSOR_IDX != 0){
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(100.0f/255.0f, 100.0f/255.0f, 100.0f/255.0f, 1.0f)); // Lighter background
        if(ImGui::BeginCombo("Branch Display Type", BRANCH_TYPES[BRANCH_IDX]))
        {
            for(int i=0;i<IM_ARRAYSIZE(BRANCH_TYPES);i++){
                const bool is_selected = (BRANCH_IDX == i);

                if(ImGui::Selectable(BRANCH_TYPES[i], is_selected)){
                    BRANCH_IDX = i;
                    PROCESSOR_CHANGE = true;
                }

                if(is_selected){
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleColor();
    }

    if(PROCESSOR_CHANGE){
        PROCESSOR_CHANGE = false;
        vm_config::config.dual_issue = false;
        vm_config::config.triple_issue = false;
        vm_config::config.branch_prediction_enabled = false;
        vm_config::config.branch_prediction_static = false;
        vm_config::config.data_forwarding_enabled = false;
        vm_config::config.hazard_detection_enabled = false;
        vm_config::config.pipelining_enabled = false;

        switch(PROCESSOR_IDX){
            case 0 : {
                break;
            }
            case 1 : {
                vm_config::config.pipelining_enabled = true;
                break;
            }
            case 2 : {
                vm_config::config.pipelining_enabled = true;
                vm_config::config.hazard_detection_enabled = true;
                break;
            }
            case 3 : {
                vm_config::config.pipelining_enabled = true;
                vm_config::config.hazard_detection_enabled = true;
                vm_config::config.data_forwarding_enabled = true;
                break;
            }
            case 4 : {
                vm_config::config.dual_issue = true;
            }
            case 5 : {
                vm_config::config.triple_issue = true;
            }
        }

        switch(BRANCH_IDX){
            case 0 : {
                break;
            }
            case 1 : {
                vm_config::config.branch_prediction_enabled = true;
                vm_config::config.branch_prediction_static = true;
                break;
            }
            case 2 : {
                vm_config::config.branch_prediction_enabled = true;
                vm_config::config.branch_prediction_static = false;
                break;
            }
        }

        vm.LoadVM();
    }
}