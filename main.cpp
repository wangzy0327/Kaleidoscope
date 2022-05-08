#include <iostream>
#include "lexer.h"
#include "parser.h"
#include "driver.h"

using namespace std;

int main() {
//    string input = " ";
//    while((getline(cin,input))){
//    while(true) {
//        int type = getTok();
//
//        string chs = "";
//        if (type == tok_number)
//            chs = to_string(NumVal);
//        else if (type == tok_def)
//            chs = "def";
//        else if (type == tok_extern)
//            chs = "extern";
//        else if (type == tok_identifier)
//            chs = IdentifierStr;
//        else if(type == tok_eof)
//            break;
//        else
//            chs = (char)type;
//        cout << "character " << chs << " tok type : " << type << endl;
//    }
//    }

    // Install standard binary operators.
    // 1 is lowest precedence.
    BinopPrecedence['<'] = 10;
    BinopPrecedence['+'] = 20;
    BinopPrecedence['-'] = 20;
    BinopPrecedence['*'] = 40; // highest.

    // Prime the first token.
    fprintf(stderr, "ready> ");
    getNextToken();

    //Run the main "interpreter loop" now.
    MainLoop();

    /**
     * //函数定义以空格分割
ready>def foo(x y) x+foo(y,4.0);
 ready> Parsed a function definition.
ready>def foo(x y) x+y y;
 ready> Parsed a function definition.
ready> Parsed a top-level expr
ready>extern sin(a);
 ready> Parsed an extern
     *
     */

    return 0;
}
