#include "tokenizer.h"
int main(int argc, char* argv[]){
    openjs::Tokenizer t;
    std::string tmp = argv[1];
    t.SetFilename(tmp);
    t.Run();
}