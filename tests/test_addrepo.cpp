#include <openspm_cli.hpp>
#include <iostream>
int main() {
    int status = openspm::cli::createDefaultConfig();
    if(status !=0){
        std::cout << "test failed" << std::endl;
    }
    status = openspm::cli::addRepository("https://testing.openspm.org",false,false);
    if(status !=0){
        std::cout << "test failed" << std::endl;
    }
    status = openspm::cli::updateAll();
    if(status !=0){
        std::cout << "test failed" << std::endl;
    }
    status = openspm::cli::listPackages();
    if(status !=0){
        std::cout << "test failed" << std::endl;
    }
}