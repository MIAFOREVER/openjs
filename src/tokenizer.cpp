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
    PreProcess(file_stream);
    
    SetRegex();
    RunTokenizer();

    DebugPrintToken();
}

void Tokenizer::PreProcess(std::string s){
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
            //std::cout << "char:\t" << "[" << s[i] << "]" << std::endl;
            std::string tmp;
  
            // push_back Tokenizer
            tmp.assign(s, last_id, i - last_id);
            //std::cout << "token:\t" << "[" << tmp << "]" << std::endl;
            if(!tmp.empty() && tmp != " " && tmp != "\n"){
                tk.push_back(tmp);
            }
            
            // push_back Operator
            if(s[i] != ' ' && s[i] != '\n'){
                tmp.assign(s, i, 1);
                tk.push_back(tmp);
            }

            last_id = i + 1;
        }
        i ++;
    }
}

void Tokenizer::DebugPrintToken(){
    for(int i  = 0; i < tk.size(); i++){
        LOG::INFO(tk[i] + ":" + Defination::GetInstance().token_map[token_type[i]]);
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
    for(auto i : tk){
        
        Defination::token_type token_type_tmp;
        if(i == "{")
            token_type_tmp = Defination::TOKEN_SCOPE_BEGIN;
        else if(i == "}")
            token_type_tmp = Defination::TOKEN_SCOPE_END;
        else if(i == "(")
            token_type_tmp = Defination::TOKEN_SMALL_SCOPE_BEGIN;
        else if(i == ")")
            token_type_tmp = Defination::TOKEN_SMALL_SCOPE_END;
        else if(i == ",")
            token_type_tmp = Defination::TOKEN_COMMA;
        else if(i == "if")
            token_type_tmp = Defination::TOKEN_IF;
        else if(i == "else")
            token_type_tmp = Defination::TOKEN_ELSE;
        else if(i == "function")
            token_type_tmp = Defination::TOKEN_FUNCTION;
        else if(i == "return")
            token_type_tmp = Defination::TOKEN_RETURN;
        else if(i == "true")
            token_type_tmp = Defination::TOKEN_BOOL_TRUE;
        else if(i == "false")
            token_type_tmp = Defination::TOKEN_BOOL_FALSE;
        else{
            token_type_tmp = Defination::TOKEN_UNKNOWN;
            for(auto reg_tmp : reg){
                if(std::regex_match(i, reg_tmp.first)){
                    token_type_tmp = reg_tmp.second;
                }
            }
        }
        // LOG::INFO(i + ":" + token_map[token_type_tmp]);
        token_type.push_back(token_type_tmp);
    }
}

std::vector<Token> Tokenizer::GetToken(){
    for(int i = 0; i < tk.size(); i++){
        Token t;
        t.SetTokenName(tk[i]);
        t.SetTokenType(token_type[i]);
        public_token.push_back(t);
    }
    return public_token;
}

}