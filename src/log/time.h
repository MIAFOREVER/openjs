#ifndef _CTIME_H_
#define _CTIME_H_
#include <iostream>
#include <iterator>
#include <sys/time.h>
#include <map>
#include <mutex>
#include <string>
class Time
{
private:
    /* data */
    std::map<std::string, timeval*> store;
public:
    static Time& GetInstance(){
        static Time instance;
        return instance;
    }
    void BeginTime(std::string s){
        timeval* tmp = new timeval;
        store[s] = tmp;
        gettimeofday (tmp, nullptr); 
    }
    void EndTime(std::string s){
        timeval tmp;
        gettimeofday (&tmp, nullptr);
        long second = tmp.tv_sec - store[s]->tv_sec;
        long micro_second = (tmp.tv_usec - store[s]->tv_usec) / 1000.0;
        std::cout << "[time]:" << s << "\t";
        std::cout << second * 1000.0 + micro_second << "ms" << std::endl;
    }
    
};
#endif