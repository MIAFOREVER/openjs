#ifndef _LOG_H_
#define _LOG_H_
#include <iostream>
#include <fstream>
#include <string>
class LOG{
public:
    static void INFO(std::string s);
    static void ERROR(std::string s);
};
#endif