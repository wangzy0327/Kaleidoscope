#include <iostream>
#include "lexer.h"

using namespace std;

int main() {
//    string input = " ";
//    while((getline(cin,input))){
    while(true) {
        int type = getTok();

        string chs = "";
        if (type == tok_number)
            chs = to_string(NumVal);
        else if (type == tok_def)
            chs = "def";
        else if (type == tok_extern)
            chs = "extern";
        else if (type == tok_identifier)
            chs = IdentifierStr;
        else if(type == tok_eof)
            break;
        else
            chs = (char)type;
        cout << "character " << chs << " tok type : " << type << endl;
    }
//    }
    return 0;
}
