#include "main.h"

// Don't put a breakpoint in this function (on vscode atleast)
int main(){
    std::cout << "Virtual Machine started." << std::endl;

    std::cout << "Enter the path to the file:" << std::endl;
    std::string file_name;
    std::cin >> file_name;
    std::cout << std::endl;

    RV5SVM vm;
    try{
        vm.program_ = assemble(file_name);
        std::cout << "Assembly successful!" << std::endl << std::endl;;
    }
    catch(const std::runtime_error& e){
        std::cout << "Assembly failed" << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
    }

    vm.LoadVM();
    vm.Run();
    return 0;
}