#include "tokenizer.h"
#include "ast.h"
int main(int argc, char* argv[]){
    std::string tmp = argv[1];
    auto tokenizer = std::make_unique<openjs::Tokenizer>(tmp);

    auto bt = std::make_unique<openjs::BuildTree>();
    bt->SetTokenizer(std::move(tokenizer));
    bt->Run();
}