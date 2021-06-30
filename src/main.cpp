#include "tokenizer.h"
#include "ast.h"
#include "log/time.h"

int main(int argc, char* argv[]){
    std::string tmp = argv[1];
    Time::GetInstance().BeginTime("tokenizer");
    auto tokenizer = std::make_unique<openjs::Tokenizer>(tmp);
    Time::GetInstance().EndTime("tokenizer");
    Time::GetInstance().BeginTime("BuildTree");
    auto bt = std::make_unique<openjs::BuildTree>();
    bt->SetTokenizer(std::move(tokenizer));
    bt->Run();
    Time::GetInstance().EndTime("BuildTree");
}