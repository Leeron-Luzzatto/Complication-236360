%{
#include "tokens.hpp"
#include<iostream>
using std::cout;
using std::endl;

char string_buf[1030];
char *string_buf_ptr;
int esc_err = 0;
std::string esc;

unsigned char to_ascii(std::string hex);

%}

%option yylineno
%option noyywrap

digit           ([1-9])
letter          ([a-zA-Z])
whitespace      ([\t\n\r ])
no_lf_cr        ([^\x0A\x0D])
good_str_char   ([\x09\x20-\x21\x23-\x5b\x5d-\x7e])

%x str
%x comment


%%

void                                  return VOID;
int                                   return INT;
byte                                  return BYTE;
b                                     return B;
bool                                  return BOOL;
and                                   return AND;
or                                    return OR;
not                                   return NOT;
true                                  return TRUE;
false                                 return FALSE;
return                                return RETURN;
if                                    return IF;
else                                  return ELSE;
while                                 return WHILE;
break                                 return BREAK;
continue                              return CONTINUE;
(\;)                                  return SC;
(\,)                                  return COMMA;
(\()                                  return LPAREN;
(\))                                  return RPAREN;
(\{)                                  return LBRACE;
(\})                                  return RBRACE;
(\=)                                  return ASSIGN;
(((\=|\!|\<|\>)\=)|\<|\>)             return RELOP;
(\+|\-|\*|\/)                         return BINOP;
\/\/([^\r\n])*                        return COMMENT;
{letter}({letter}|{digit}|0)*         return ID;
({digit}({digit}|0)*)|0               return NUM;
{whitespace}				                    ;

\"                                    string_buf_ptr = string_buf; esc_err=0; BEGIN(str);
<str>\"                               {
                                        BEGIN(INITIAL);
                                        *string_buf_ptr = '\0';
                                        if(esc_err > 0){
                                            cout << "Error undefined escape sequence " << esc << endl;
                                            exit(0);
                                        }
                                        printf("%d %s %s\n", yylineno, "STRING", string_buf);
                                        return STRING;
                                      }
<str>[\n\r]                           {printf("Error unclosed string\n"); exit(0);}
<str><<EOF>>                          {printf("Error unclosed string\n"); exit(0);}
<str>\\\"                             { *string_buf_ptr++ = '\"'; }
<str>\\\\                             { *string_buf_ptr++ = '\\'; }
<str>\\0                              { *string_buf_ptr++ = '\0'; }
<str>\\r                              { *string_buf_ptr++ = '\r'; }
<str>\\n                              { *string_buf_ptr++ = '\n'; }
<str>\\t                              { *string_buf_ptr++ = '\t'; }
<str>\\x{good_str_char}{good_str_char}  { *string_buf_ptr++ =  to_ascii(std::string(yytext).substr(2,3));}
<str>\\x.\"                           { printf("Error undefined escape sequence x%c\n", yytext[2]); exit(0);}
,str>\\x\"                            { printf("Error undefined escape sequence x\n"); exit(0);}
<str>\\.                              {if(esc_err==0){
                                            esc_err=1;
                                            esc.push_back(yytext[1]);
                                            }
                                      }



<str>({good_str_char}|\\)                 { *string_buf_ptr++ = *yytext;}
.		                              return UNKNOWNCommand;


%%

unsigned char to_ascii(std::string hex){
    if (hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos){
        std::cout << "Error undefined escape sequence x" << hex << std::endl;
        exit(0);
    }
    unsigned char chr = (char) (int)strtol(hex.c_str(), nullptr, 16);
    if(chr == 0){
        std::cout << "Error undefined escape sequence x" << hex << std::endl;
        exit(0);
    }
    if(chr < 0x00 || chr > 0x7F){
        std::cout << "Error undefined escape sequence x" << hex << std::endl;
        exit(0);
    }
    return chr;
}