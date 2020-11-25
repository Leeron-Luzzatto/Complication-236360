#include "tokens.hpp"
#include <cstdio>
#include <stdlib.h>
#include <iostream>

char handle_escape(char esc){
    if(esc == 't'){
        return '\t';
    }
    else if(esc == 'r'){
        return '\r';
    }
    else if(esc == 'n'){
        return '\n';
    }
    else if(esc == '0'){
        return '\0';
    }
    else if(esc == '\\'){
        return '\\';
    }
    else if(esc == '\"'){
        return '\"';
    }
}

//unsigned char to_ascii(std::string hex){
//    if (hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos){
//        std::cout << "Error undefined escape sequence x" << hex << std::endl;
//        exit(0);
//    }
//    unsigned char chr = (char) (int)strtol(hex.c_str(), nullptr, 16);
//    if(chr == 0){
//        std::cout << "Error undefined escape sequence x" << hex << std::endl;
//        exit(0);
//    }
//    if(chr < 0x00 || chr > 0x7F){
//        std::cout << "Error undefined escape sequence x" << hex << std::endl;
//        exit(0);
//    }
//    return chr;
//}

int main()
{
	int token;
	while(token = yylex()) {
	    // Handle unknown char first
        if(token == UNKNOWNCommand){
            printf("Error %s\n", yytext);
            exit(0);
        }

        else if(token == COMMENT){
            printf("%d %s %s\n", yylineno, "COMMENT", "//");
        }

        else if(token == STRING){
            continue;
        }

        else{
            printf("%d %s %s\n", yylineno, enumToString[token], yytext);
        }
	}
	return 0;
}