#include "tokenizer.h"
#include "log/log.h"
#include <fstream>
#include <algorithm>
#include <iostream>

namespace openjs{

Tokenizer::Tokenizer(std::string _filename){
    SetFilename(_filename);
    Run();
}

void Tokenizer::Run(){
    SetOperatorCode();
    ReadFile(filename);
    Split(file_stream);
    
    SetRegex();
    RunTokenizer();

    DebugPrintToken();
}

std::vector<std::string> Tokenizer::GetSourceCode(){
    return source_code;
}

void Tokenizer::Split(std::string s){
    int last_id = 0;
    int i = 0;
    // Here is use whlile rather than for is bacause of the continue sentence
    // 'for(int i = 0; i < size(); i++)'
    // the 'continue' is still run i++ at the end;
    // 'while(){}' don't
    while(i < s.size()){
        // Delete annotation
        if(s[i] == '/'){
            if(i + 1 < s.size()){
                if(s[i + 1] == '/'){
                    int found = s.find('\n', i + 2);
                    if(found != std::string::npos){
                        position_line ++;
                        position_char = 0;


                        i = found + 1;
                        last_id = i;
                        continue;
                    }else{
                        i = s.size();
                        last_id = i;
                        continue;
                    }
                }
            }
        }
        if(InOperatorCode(s[i])){
            if(s[i] == '\n'){
                position_line ++;
                position_char = 0;
            }
            std::string tmp;
  
            // push_back Tokenizer
            tmp.assign(s, last_id, i - last_id);
            //std::cout << "token:\t" << "[" << tmp << "]" << std::endl;
            if(!tmp.empty() && tmp != " " && tmp != "\n"){
                tk.push_back(std::make_shared<Token>(std::move(tmp)));
                tk.back()->SetPosition(position_line, position_char);
            }
            
            // push_back Operator
            if(s[i] != ' ' && s[i] != '\n'){
                tmp.assign(s, i, 1);
                tk.push_back(std::make_shared<Token>(std::move(tmp)));
                tk.back()->SetPosition(position_line, position_char);
            }

            last_id = i + 1;
        }
        position_char ++;
        i ++;
    }
}

void Tokenizer::DebugPrintToken(){
    for(auto i : tk){
        LOG::INFO(i->GetTokenName() + ":" + Defination::GetInstance().token_map[i->GetTokenType()] + 
        ":" + std::to_string(i->GetPosition().first) + ":" + std::to_string(i->GetPosition().second));
    }
}


void Tokenizer::SetOperatorCode(){
    operator_code.push_back("+");
    operator_code.push_back("-");
    operator_code.push_back("*");
    operator_code.push_back("/");
    operator_code.push_back("(");
    operator_code.push_back(")");
    operator_code.push_back(" ");
    operator_code.push_back(",");
    operator_code.push_back("\n");
    operator_code.push_back(";");
    operator_code.push_back("{");
    operator_code.push_back("}");
    operator_code.push_back(">");
    operator_code.push_back("<");
}

bool Tokenizer::InOperatorCode(char s){
    std::string tmp;
    tmp.push_back(s);
    if(find(operator_code.begin(), operator_code.end(), tmp) != operator_code.end()){
        return true;
    }
    return false;
}

void Tokenizer::ReadFile(std::string _filename){
    std::ifstream in(_filename);
    std::istreambuf_iterator<char> begin(in);
    std::istreambuf_iterator<char> end;
    std::string retval(begin, end);
    file_stream = retval;
    LOG::INFO(file_stream);
    std::string tmp;
    int begin_id = 0;
    int found = 0;
    while(found != std::string::npos){
        found = file_stream.find('\n', begin_id);
        tmp.assign(file_stream, begin_id, found - begin_id);
        source_code.push_back(tmp);
        begin_id = found + 1;
    }
    for(auto i : source_code){
        LOG::INFO(i);
    }
}

void Tokenizer::SetFilename(std::string _filename){
    filename = _filename;
}

void Tokenizer::SetRegex(){
    std::vector<std::pair<std::string, Defination::token_type>> tmp;
    tmp.push_back(std::make_pair("[a-zA-Z][a-zA-Z0-9]*", Defination::TOKEN_ID));
    tmp.push_back(std::make_pair("^(\\-|\\+)?\\d+(\\.\\d+)?", Defination::TOKEN_NUMBER));
    tmp.push_back(std::make_pair("[<>\\+\\-\\*\\\\]", Defination::TOKEN_ORERATOR));
    for(auto i : tmp){
        std::regex r(i.first);
        reg.push_back(std::make_pair(r, i.second));
    }
}

void Tokenizer::RunTokenizer(){
    for(auto token : tk){
        std::string token_name = token->GetTokenName();
        Defination::token_type token_type;
        if(token_name == "{")
            token_type = Defination::TOKEN_SCOPE_BEGIN;
        else if(token_name == "}")
            token_type = Defination::TOKEN_SCOPE_END;
        else if(token_name == "(")
            token_type = Defination::TOKEN_SMALL_SCOPE_BEGIN;
        else if(token_name == ")")
            token_type = Defination::TOKEN_SMALL_SCOPE_END;
        else if(token_name == ",")
            token_type = Defination::TOKEN_COMMA;
        else if(token_name == "if")
            token_type = Defination::TOKEN_IF;
        else if(token_name == "else")
            token_type = Defination::TOKEN_ELSE;
        else if(token_name == "function")
            token_type = Defination::TOKEN_FUNCTION;
        else if(token_name == "return")
            token_type = Defination::TOKEN_RETURN;
        else if(token_name == "true")
            token_type = Defination::TOKEN_BOOL_TRUE;
        else if(token_name == "false")
            token_type = Defination::TOKEN_BOOL_FALSE;
        else{
            token_type = Defination::TOKEN_UNKNOWN;
            for(auto reg_tmp : reg){
                if(std::regex_match(token_name, reg_tmp.first)){
                    token_type = reg_tmp.second;
                }
            }
        }
        
        token->SetTokenType(std::move(token_type));
    }
}

std::vector<std::shared_ptr<Token>> Tokenizer::GetToken(){
    return tk;
}

}