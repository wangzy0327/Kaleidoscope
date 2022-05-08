//
// Created by wangzy on 2022/5/8.
//

#ifndef KALEIDOSCOPE_DRIVER_H
#define KALEIDOSCOPE_DRIVER_H

#include "parser.h"


//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

static void HandleDefinition() {
    if (ParseDefinition()) {
        fprintf(stderr, "Parsed a function definition.\n");
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleExtern() {
    if (ParseExtern()) {
        fprintf(stderr, "Parsed an extern\n");
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (ParseTopLevelExpr()) {
        fprintf(stderr, "Parsed a top-level expr\n");
    } else {
        // Skip token for error recovery.
        getNextToken();
    }
}


//我们忽略了顶层的分号。为什么呢？举个例子，当你在命令行中键入“4 + 5”后，语法解析器无法判断你键入的内容是否已经完结
//top :: = definition | external | expression | ';'
static void MainLoop(){
    while(1){
        fprintf(stderr, "ready> ");
        switch (CurTok) {
            case tok_eof:
                return;
            case ';': // ignore top-level semicolons.
                getNextToken();
                break;
            case tok_def:
                HandleDefinition();
                break;
            case tok_extern:
                HandleExtern();
                break;
            default:
                HandleTopLevelExpression();
                break;
        }
    }
}

#endif //KALEIDOSCOPE_DRIVER_H
