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
#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
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
static std::unique_ptr<ExprAST> ParseNumberExpr(){
    std::unique_ptr<ExprAST> Result(new NumberExprAST(NumVal));
    getNextToken();
    return std::move(Result);
}

static std::unique_ptr<ExprAST> ParseExpression();

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

//接下来是解析二元表达式相关内容


//通过二元运算符的优先级来引导递归调用走向解析

//制定优先级表
//BinopPrecedence - This holds the precedence for each binary operator that is defined
static std::map<char,int> BinopPrecedence;

//函数GetTokPrecedence用于查询当前语元的优先级，如果当前语元不是二元运算符则返回-1
// 这里的map简化了新运算符的添加，同时也可以证明我们的算法与具体的运算符无关
// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence() {
    if (!isascii(CurTok))
        return -1;

    // Make sure it's a declared binop.
    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0) return -1;
    return TokPrec;
}

static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,std::unique_ptr<ExprAST> LHS);

//解析二元表达式
//运算符优先级解析的基本思想就是通过拆解含有二元运算符的表达式来解决可能的二义性问题
//以表达式a+b+(c+d)*e*f+g为例
//解析出来的第一个主表达式应该是a，紧跟着是若干个有序对，即：[+, b]、[+, (c+d)]、[*, e]、[*, f]和[+,g]
//注意括号表达式也是主表达式，所以在解析二元表达式时无需特殊照顾(c+d)这样的嵌套表达式

//每个表达式都由一个主表达式打头阵，身后可能还跟着一串由有序对构成的列表
//有序对的格式为[binop,primaryexpr]

//expression
// := primary binoprhs
static std::unique_ptr<ExprAST> ParseExpression(){
    auto LHS = ParsePrimary();
    if(!LHS)
        return nullptr;
    return ParseBinOpRHS(0,std::move(LHS));
}

//函数ParseBinOpRHS用于解析有序对列表（其中RHS是Right Hand Side的缩写，表示右侧；与此相对应，LHS表示左侧）
//它的参数包括一个整数和一个指针，其中整数代表运算符优先级，指针则指向当前已解析出来的那部分表达式
//注意，单独一个x也是合法的表达式：也就是说binoprhs有可能为空;碰到这种情况时，函数直接返回作为参数传入的表达式
//在上面的例子中a+b+(c+d)*e*f+g，传入ParseBinOpRHS的表达式是a,当前语元是+

//传入ParseBinOpRHS的优先级表示的是该函数所能处理的最低运算符优先级
//假设语元流中的下一对是[+, x]，且传入ParseBinOpRHS的优先级是40，那么该函数将直接返回（因为+的优先级是20）

//binoprhs
// ::=('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,std::unique_ptr<ExprAST> LHS){
    // If this is a binop, find its precedence.
    //先检查当前语元的优先级，如果优先级过低则直接返回
    while (true) {
        int TokPrec = GetTokPrecedence();

        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (TokPrec < ExprPrec)
            return LHS;
        // Okay, we know this is a binop.
        int BinOp = CurTok;
        getNextToken();  // eat binop

        //就这样，二元运算符处理完毕（并保存妥当）之后，紧跟其后的主表达式也随之解析完毕
        //第一对有序对[+,b]就构造完了
        // Parse the primary expression after the binary operator.
        auto RHS = ParsePrimary();
        if (!RHS)
            return nullptr;
        //现在表达式的左侧和RHS序列中第一对都已经解析完毕，该考虑表达式的结合次序了。
        // 路有两条，要么选择(a+b) binop unparsed，要么选择a + (b binopunparsed)。
        // 为了搞清楚到底该怎么走，我们先预读出binop，查出它的优先级，再将之与BinOp（本例中是“+”）的优先级相比较

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        //binop位于RHS的右侧，如果binop的优先级低于或等于当前运算符的优先级，则可知括号应该加在前面，即按(a+b) binop ...处理
        //当前运算符是+，下一个运算符也是+，二者的优先级相同。既然如此，理应按照“a+b”来构造AST节点，然后我们继续解析
        int NextPrec = GetTokPrecedence();
        //如果自binop以右的若干个连续有序对都含有优先级高于“+”的运算符，那么就应该把它们全部解析出来，拼成“RHS”后返回。
        //为此，我们将最低优先级设为“TokPrec+1”，递归调用函数“ParseBinOpRHS”
        //该调用会完整解析出上述示例中的“(c+d)*e*f”，并返回构造出的AST节点，这个节点就是“+”表达式右侧的RHS
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec+1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }
        //Merge LHS/RHS
        unique_ptr<BinaryExprAST> newLHS(new BinaryExprAST(BinOp,std::move(LHS),std::move(RHS)));
        LHS = std::move(newLHS);

        //最后，while(1)循环的下一轮迭代将会解析出剩下的“+g”并将之纳入AST

    } // loop around to the top of the while loop

}

//表达式的解析就此告一段落
// 现在，我们可以将任意语元流喂入语法解析器并逐步从中构造出表达式，直到解析至不属于表达式的语元为止

//解析其他结构
//有两处会用到函数原型：一是extern函数声明，二是函数定义
/// prototype
///   ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {
    if (CurTok != tok_identifier)
        return LogErrorP("Expected function name in prototype");

    std::string FnName = IdentifierStr;
    getNextToken(); // eat CurTok which is FnName

    if (CurTok != '(')
        return LogErrorP("Expected '(' in prototype");

    //CurTok == '('
    // Read the list of argument names.
    std::vector<std::string> ArgNames;
    //getNextToken() -> eat '(' return next CurTok
//    while (getNextToken() == tok_identifier)
//        ArgNames.push_back(IdentifierStr);
//    if (CurTok != ')')
//        return LogErrorP("Expected ')' in prototype");

//修改为函数参数定义以,分割
    getNextToken();  // eat '('
    if(CurTok != ')'){
        while(1){
            if(CurTok == tok_identifier)
                ArgNames.push_back(IdentifierStr);
            else if(CurTok == ')')
                break;
            else if(CurTok != ',')
                return LogErrorP("Expected ')' or ',' in argument list");
            getNextToken();   // eat ,
        }
    }

    // success.
    getNextToken();  // eat ')'.

    std::unique_ptr<PrototypeAST> prototypeAst(new PrototypeAST(FnName,std::move(ArgNames)));

    return std::move(prototypeAst);
}

//在此基础上 函数定义就简单了，就是函数原型再加一个用作函数体的表达式
// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition() {
    getNextToken();  // eat def.
    //解析函数接口
    auto Proto = ParsePrototype();
    if (!Proto) return nullptr;
    //解析函数体表达式
    if (auto E = ParseExpression()){
        //构建函数AST
        std::unique_ptr<FunctionAST> ProtoFunc(new FunctionAST(std::move(Proto), std::move(E)));
        return std::move(ProtoFunc);
    }
    return nullptr;
}

//除了用于用户自定义函数的前置声明，extern语句还可以用来声明sin、cos等（C标准库）函数
//这些extern语句不过就是些不带函数体的函数原型罢了
// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern() {
    getNextToken();  // eat extern.
    return ParsePrototype();
}

//最后，我们还允许用户随时在顶层输入任意表达式并求值
// 这一特性是通过一个特殊的匿名零元函数（没有任何参数的函数）实现的，所有顶层表达式都定义在这个函数之内
// toplevelexpr ::= expression
static std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
    if (auto E = ParseExpression()) {
        // Make an anonymous proto.
        //解析函数接口
        std::unique_ptr<PrototypeAST> ProtoName(new PrototypeAST("", std::vector<std::string>()));
        std::unique_ptr<FunctionAST> FuncAST(new FunctionAST(std::move(ProtoName), std::move(E)));
        return std::move(FuncAST);
    }
    return nullptr;
}


#endif //KALEIDOSCOPE_PARSER_H
