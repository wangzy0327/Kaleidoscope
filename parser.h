//
// Created by wangzy on 2022/4/30.
//

#ifndef KALEIDOSCOPE_PARSER_H
#define KALEIDOSCOPE_PARSER_H

//Syntax Parser
//语法解析器

#include "lexer.h"
#include "ast.h"
#include <cstring>
#include <string>

using namespace std;

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

//解析基本表达式
//数值常量

//调用它时，带解析语元只能是tok_number
//该函数用刚解析出的数值构造出NumberExprAST节点，然后令词法分析器继续读取下一个语元
//numberexpr := number
static std::unique_str<ExprAST> ParseNumberExpr(){
    std::unique_ptr<ExprAST> Result(new NumberExprAST(NumVal));
    getNextToken();
    return std::move(Result);
}

//消化了所有与当前生成规则相关的所有语元
//还把下一个待解析的语元放进了词法分析器的语元缓冲
//1.调用该函数时，待解析的语元只能是(,然而解析完子表达式后，紧跟着的语元却不一定是)。 比如用户输入的是(4 x而不是(4)
//2.该函数的另一个特点在于递归调用了ParseExpression.这种手法简化了递归语法的处理。
//注意，我们没有必要为括号构造AST节点。括号的作用主要还是对表达式进行分组进而引导语法解析过程。当构造完AST后，括号就没用了
//parentexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParentExpr(){
    getNextToken(); // eat (
    auto V = ParseExpression();
    if(!V)
        return nullptr;
    if(CurTok != ')')
        return LogError("expected ')'");
    getNextToken();  // eat ).
    return V;
}

//负责处理变量引用和函数调用
//identifierexpr
// :: = identifier
// :: = identifier '(' expression* ')'
//判断它究竟是个独立的变量引用还是个函数调用
//只要检查紧跟标识符之后的语元是不是(，它就能知道到底应该构造VariableExprAST节点还是CallExprAST节点

static std::unique_ptr<ExprAST> ParseIdentifierExpr(){
    std::string IdName = IdentifierStr;
    getNextToken();       // eat identifier.
    //判断是独立变量
    if(CurTok != '('){
        std::unique_ptr<VariableExprAST> SimpleVariable(new VariableExprAST(IdName));
        return std::move(SimpleVariable);
    } // Simple variable ref.
    //Call.
    //函数调用
    getNextToken();   // eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if(CurTok != ')'){
        while(1){
            if(auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else
                return nullptr;

            if(CurTok == ')')
                break;
            if(CurTok != ',')
                return LogError("Expected ')' or ',' in argument list");
            getNextToken();   // eat ,
        }
    }
    // Eat the ')'
    getNextToken();
    //含函数名和参数列表的 表达式列表
    std::unique_ptr<CallExprAST> CallExprList(new CallExprAST(IdName,std::move(Args)));
    return std::move(CallExprList);
}


//辅助函数
//判定待解析表达式的类别
//primary
// ::= identifierexpr
// ::= numberexpr
// ::= parentexpr
static std::unique_ptr<ExprAST> ParsePrimary(){
    //根据 CurTok 语元类型来判定表达式类别
    switch (CurTok) {
        default:
            return LogError("unknown token when expecting an expression");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParentExpr();
    }
}

//解析二元表达式



#endif //KALEIDOSCOPE_PARSER_H
