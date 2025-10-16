#include "main.h"

int main(){
    std::cout << "Virtual Machine started." << std::endl;

    std::cout << "Enter the path to the file:" << std::endl;
    std::string file_name;
    std::cin >> file_name;
    std::cout << std::endl;

    AssembledProgram program;
    try{
        program = assemble(file_name);
        std::cout << "Assembly successful!" << std::endl << std::endl;;
    }
    catch(const std::runtime_error& e){
        std::cout << "Assembly failed" << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
    }

    RVSSVM vm;

    vm.LoadProgram(program);
    vm.Run();

    return 0;
}