#ifndef _ERROR_PROCESS_H_
#define _ERROR_PROCESS_H_
#include <string>
#include <vector>
#include <iostream>
#include "token.h"
namespace openjs{

class ErrorProcess{
public:
    void SetSouceCode(std::vector<std::string> _source_code){
        source_code = _source_code;
    }

    void PrintError(Token tk, std::string reason){
        auto tmp = tk.GetPosition();
        for(int i = 0; i < source_code[tmp.first].size(); i++)
            std::cout << "=";
        std::cout << "\n";
        
        std::cout << source_code[tmp.first] << std::endl;

        for(int i = 0; i < source_code[tmp.first].size(); i++){
            if(i != tmp.second)
                std::cout << " ";
            else
                std::cout << "^";
        }
        std::cout << std::endl;
        // for(int i = 0; i < source_code[tmp.first].size(); i++){
        //     if(i != tmp.second)
        //         std::cout << " ";
        //     else
        //         std::cout << "|";
        // }
        // std::cout << "\n";

        std::cerr << reason << std::endl;
        exit(0);
    }

    std::vector<std::string> source_code;

};

}
#endif