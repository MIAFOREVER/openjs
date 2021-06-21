#include "token.h"
int main(int argc, char* argv[]){
    openjs::Token t;
    std::string tmp = argv[1];
    t.SetFilename(tmp);
    t.Run();
}