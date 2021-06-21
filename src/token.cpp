#include "token.h"
#include "log/log.h"
#include <fstream>
#include <algorithm>
#include <iostream>

namespace openjs{

void Token::Run(){
    SetOperatorCode();
    ReadFile(filename);
    PreProcess(file_stream);
    
    SetRegex();
    Tokenizer();

    DebugPrintToken();
}

void Token::PreProcess(std::string s){
    int last_id = 0;
    for(int i = 0; i < s.size(); i++){
        // Delete Annotation
        if(s[i] == '/'){
            if(i + 1 < s.size()){
                if(s[i + 1] == '/'){
                    int found = s.find('\n', i + 1);
                    if(found != std::string::npos){
                        i = found + 1;
                        last_id = i;
                        continue;
                    }
                }
            }
        }
        if(InOperatorCode(s[i])){
            //std::cout << "char:\t" << "[" << s[i] << "]" << std::endl;
            std::string tmp;
  
            // push_back Token
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
    }
}

void Token::DebugPrintToken(){
    for(int i  = 0; i < tk.size(); i++){
        LOG::INFO(tk[i] + ":" + token_map[token_type[i]]);
    }
}


void Token::SetOperatorCode(){
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

bool Token::InOperatorCode(char s){
    std::string tmp;
    tmp.push_back(s);
    if(find(operator_code.begin(), operator_code.end(), tmp) != operator_code.end()){
        return true;
    }
    return false;
}

void Token::ReadFile(std::string _filename){
    std::ifstream in(_filename);
    std::istreambuf_iterator<char> begin(in);
    std::istreambuf_iterator<char> end;
    std::string retval(begin, end);
    file_stream = retval;
    LOG::INFO(file_stream);
}

void Token::SetFilename(std::string _filename){
    filename = _filename;
}

void Token::SetRegex(){
    std::vector<std::pair<std::string, token>> tmp;
    tmp.push_back(std::make_pair("[a-zA-Z][a-zA-Z0-9]*", TOKEN_ID));
    tmp.push_back(std::make_pair("^(\\-|\\+)?\\d+(\\.\\d+)?", TOKEN_NUMBER));
    tmp.push_back(std::make_pair("[<>\\+\\-\\*\\\\]", TOKEN_ORERATOR));
    for(auto i : tmp){
        std::regex r(i.first);
        reg.push_back(std::make_pair(r, i.second));
    }
}

void Token::Tokenizer(){
    for(auto i : tk){
        
        token token_type_tmp;
        if(i == "{")
            token_type_tmp = TOKEN_SCOPE_BEGIN;
        else if(i == "}")
            token_type_tmp = TOKEN_SCOPE_END;
        else if(i == "(")
            token_type_tmp = TOKEN_SMALL_SCOPE_BEGIN;
        else if(i == ")")
            token_type_tmp = TOKEN_SMALL_SCOPE_END;
        else if(i == "if")
            token_type_tmp = TOKEN_IF;
        else if(i == "else")
            token_type_tmp = TOKEN_ELSE;
        else if(i == "function")
            token_type_tmp = TOKEN_FUNCTION;
        else if(i == "return")
            token_type_tmp = TOKEN_RETURN;
        else if(i == "true")
            token_type_tmp = TOKEN_BOOL_TRUE;
        else if(i == "false")
            token_type_tmp = TOKEN_BOOL_FALSE;
        else{
            token_type_tmp = TOKEN_UNKNOWN;
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

std::vector<std::string> Token::GetToken(){
    return tk;
}
std::vector<Token::token> Token::GetTokenType(){
    return token_type;
}

}