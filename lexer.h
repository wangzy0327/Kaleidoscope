//
// Created by wangzy on 2022/4/30.
//

#ifndef KALEIDOSCOPE_LEXER_H
#define KALEIDOSCOPE_LEXER_H

#include <cstring>
#include <string>

using namespace std;

//The lexer returns tokens [0-255] if it is an unknown character,
//otherwise one of these for known things.

enum Token{
    tok_eof = -1,
    //command
    tok_def = -2,
    tok_extern = -3,
    //primary
    //标识符
    tok_identifier = -4,
    //数据类型 这里就只设置double一种数据类型
    tok_number = -5
};


//Filled in if tok_identifier
//如果当前语元是标识符，其名称存入全局变量Identifier
std::string IdentifierStr;
//Filled in if tok_number
//如果当前语元是数值常量(比如 1.0),其值将被存入NumVal
double NumVal;


// - Return the next token type from standard input.
int getTok(){
    //要在退出函数 保留lastChar的状态
    static char lastChar = ' ';

    //skip any whitespace.
    while(isspace(lastChar) )
        lastChar = getchar();

    if(lastChar == EOF)
        return tok_eof;

    if(isalpha(lastChar) || lastChar == '_'){
        //[a-zA-Z_][_a-zA-Z0-9]*
        IdentifierStr = lastChar;
        while(isalnum(lastChar = getchar()) || lastChar == '_')
            IdentifierStr += lastChar;
        if(IdentifierStr == "def")
            return tok_def;
        if(IdentifierStr == "extern")
            return tok_extern;
        //else identifier
        return tok_identifier;
    }
    //Number:[0-9.]+
    if(isdigit(lastChar) || lastChar == '.'){
        string NumStr = "";
        NumStr += lastChar;
        while(isdigit(lastChar = getchar()) || lastChar == '.')
            NumStr += lastChar;
        //这里不做数字合法性检查
        NumVal = stof(NumStr);
        return tok_number;
    }
    // #
    if(lastChar == '#'){
        //Comment until end of line
        do {
            lastChar = getchar();
        }while(lastChar != EOF && lastChar != '\n' && lastChar != '\r');

        //递归 吃掉 注释 重新找下一个token
        if(lastChar != EOF)
            return getTok();
    }
    //Otherwise,just return the character as its ascii value
    //for example +,-,*,/,<
    int thisChar = lastChar;
    lastChar = getchar();
    return thisChar;
}

#endif //KALEIDOSCOPE_LEXER_H
