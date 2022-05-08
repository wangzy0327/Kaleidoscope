//
// Created by wzy on 2022/5/7.
//

#ifndef KALEIDOSCOPE_AST_H
#define KALEIDOSCOPE_AST_H

//Abstract syntax tree
//抽象语法树

#include <cstring>
#include <string>
#include <memory>
#include <vector>
#include "lexer.h"

using namespace std;

//ExprAST - Base class for all expression nodes.
class ExprAST{
public:
    virtual ~ExprAST(){};
};


//NumberExprAST - Expression class for numeric literals like "1.0"
class NumberExprAST : public ExprAST{
    double Val;
public:
    NumberExprAST(double val) : Val(val){};
//    virtual ~NumberExprAST(){};
};

//VariableExprAST用于保存变量名
// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST{
    string Name;
public:
    VariableExprAST(const string &Name):Name(Name){};
//    virtual ~VariableExprAST(){};
};

//BinaryExprAST用于保存运算符（如+）
// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST{
    char Op;
    unique_ptr<ExprAST> LHS,RHS;
public:
    BinaryExprAST(char op,unique_ptr<ExprAST> LHS,
                  unique_ptr<ExprAST> RHS) : Op(op),LHS(move(LHS)),RHS(move(RHS)){};
//    virtual ~BinaryExprAST(){};
};

//CallExprAST用于保存函数名和用作参数的表达式列表
// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

public:
    CallExprAST(const std::string &Callee,
                std::vector<std::unique_ptr<ExprAST>> Args)
            : Callee(Callee), Args(std::move(Args)) {};
//    virtual ~CallExprAST(){};
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
            : Name(name), Args(std::move(Args)) {};
//    virtual ~PrototypeAST(){};

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
            : Proto(std::move(Proto)), Body(std::move(Body)) {};
//    virtual ~FunctionAST(){};
};

// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
// token the parser is looking at.  getNextToken reads another token from the
// lexer and updates CurTok with its results.
//tok type
//这在词法分析器周围实现了一个简单的标记缓冲区
//这允许我们在词法分析器返回时提前查看一个标记
//我们的解析器中的每个函数都假定CurTok是需要解析的当前标记

#endif //KALEIDOSCOPE_AST_H
