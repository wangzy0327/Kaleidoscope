//
// Created by wangzy on 2022/4/30.
//

#ifndef KALEIDOSCOPE_PARSER_H
#define KALEIDOSCOPE_PARSER_H

#include <cstring>
#include <string>
#include "lexer.h"

using namespace std;

//ExprAST - Base class for all expression nodes.
class ExprAST{
public:
    virtual ~ExprAST();
};


//NumberExprAST - Expression class for numeric literals like "1.0"
class NumberExprAST : public ExprAST{
    double Val;
public:
    NumberExprAST(double val) : Val(val){}
};

//VariableExprAST用于保存变量名
// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST{
    string Name;
public:
    VariableExprAST(const string &Name):Name(Name){}
};

//BinaryExprAST用于保存运算符（如+）
// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST{
    char Op;
    unique_ptr<ExprAST> LHS,RHS;
public:
    BinaryExprAST(char op,unique_str<ExprAST> LHS,
                  unique_str<ExprAST> RHS) : Op(op),LHS(move(LHS)),RHS(move(RHS)){}
};

//CallExprAST用于保存函数名和用作参数的表达式列表
// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

public:
    CallExprAST(const std::string &Callee,
                std::vector<std::unique_ptr<ExprAST>> Args)
            : Callee(Callee), Args(std::move(Args)) {}
};

//函数的接口
// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;

public:
    PrototypeAST(const std::string &name, std::vector<std::string> Args)
            : Name(name), Args(std::move(Args)) {}

    const std::string &getName() const { return Name; }
};

//函数本身
/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                std::unique_ptr<ExprAST> Body)
            : Proto(std::move(Proto)), Body(std::move(Body)) {}
};

// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
// token the parser is looking at.  getNextToken reads another token from the
// lexer and updates CurTok with its results.
//tok type
//这在词法分析器周围实现了一个简单的标记缓冲区
//这允许我们在词法分析器返回时提前查看一个标记
//我们的解析器中的每个函数都假定CurTok是需要解析的当前标记
static int CurTok;
static int getNextToken() {
    return CurTok = getTok();
}

//LogError* - These are little helper functions for error handling.
//LogError例程是我们的解析器将用于处理错误的简单帮助程序例程
unique_ptr<ExprAST> LogError(const char* Str){
    fprintf(stderr, "LogError: %s\n", Str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
}


#endif //KALEIDOSCOPE_PARSER_H
