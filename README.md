# RISC-V Simulator

## Overview

This project is an advanced extension of an in-house single-cycle RISC-V simulator. It has been refactored and expanded into a configurable architecture capable of simulating various pipelined designs, including advanced **Dynamic Dual Issue** and **Triple Issue** superscalar modes.

A custom **Graphical User Interface (GUI)** visualizes the instruction flow, pipeline stages, and core performance metrics like Cycles Per Instruction (CPI).

## Key Features

* **Pipelining:** Supports a 5-stage pipeline with configurable hazard detection and **data forwarding**.
* **Superscalar Modes:**
    * Dynamic **Dual Issue** (1 ALU + 1 LSU).
    * Dynamic **Triple Issue** (1 ALU + 1 FPU + 1 LSU).
* **Branch Prediction:** Configurable static and 1-bit dynamic prediction.
* **GUI Frontend:** Developed with ImGui, providing real-time pipeline visualization.

## Compilation and Execution

The simulator uses CMake for its build system.

| Step | Command | Description |
| :--- | :--- | :--- |
| 1 | `brew install glfw (macos) / sudo apt install libglfw3-dev libglew-dev libgl1-mesa-dev (linux)` | Install glfw. |
| 2 | `git clone https://github.com/Levistus71/carch_project` | Fetch the source code. |
| 3 | `cd carch_project` | Navigate to the project directory. |
| 4 | `cmake -S . -B build` | Configure the build. |
| 5 | `cd build` | Navigate to the build directory. |
| 6 | `cmake --build .` | Compile the project binaries. |
| 7 | `./vm` | Run the simulator. |

> **Note on Compatibility:** The simulator was developed and tested on **macOS Tahoe**. While it uses the cross-platform ImGui library, full compatibility on other operating systems cannot be guaranteed.


## Academic Context
This project was developed as a course project for the **Computer Architecture** course (CS2323) taught at **IIT Hyderabad**.

* **Institution:** Indian Institute of Technology Hyderabad (IIT-H)
* **Course:** Computer Architecture (CS2323)`
* **Instructor:** Professor Rajesh Kedia (https://people.iith.ac.in/rkedia/)

## Third Party Software And Acknowledgements
* The project was an extension to the previous in-house simulator and therefore uses code from it. 
    * License at third_party_licenses/INEXTENDED_SIM_LICENSE. 
    * Original Code : https://github.com/VishankSingh/riscv-simulator-2
* The frontend editor uses code from BalazsJako's ImGuiColorTextEdit with minor modifications.
    * License at third_party_licenses/EDITOR_LICENSE. 
    * Original Code : https://github.com/BalazsJako/ImGuiColorTextEdit/.
* ImGUI Library was used for the frontend. 
    * License at imgui/LICENSE. 
    * Original Code : https://github.com/ocornut/imgui

Huge thanks to BalazsJako for creating and open-sourcing the ImGuiColorTextEdit editor and to Omar Cornut and others of Dear ImGui for their invaluable contribution to the developer community.
Thanks also the contributors of RIPES and RARS for inspiration.