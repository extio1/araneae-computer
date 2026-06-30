%{
#include "araneae/ast/Ast.h"
#include "araneae/ast/Lexer.h"
#include "araneae/parser/parser.out.h"
#include <stdio.h>
#include <string.h>

int CurrentLine = 1;
int CurrentColumn = 1;
int TokenStartLine = 1;
int TokenStartColumn = 1;

void UpdatePosition(int line, int column) {
  CurrentLine = line;
  CurrentColumn = column;
}

void SetTokenStart() {
  TokenStartLine = CurrentLine;
  TokenStartColumn = CurrentColumn;
}

int getCurrentLine() { return CurrentLine; }
int getCurrentColumn() { return CurrentColumn; }
int getTokenStartLine() { return TokenStartLine; }
int getTokenStartColumn() { return TokenStartColumn; }

#define YY_USER_ACTION { \
  SetTokenStart(); \
  CurrentColumn += yyleng; \
}

%}

%%
[ \t]+          { CurrentColumn += yyleng; }
\n              { CurrentLine++; CurrentColumn = 1; }
\r              {}
"//".*          { CurrentColumn += yyleng; }

"of" { return KW_OF; }
"from" { return KW_FROM; }
"in" { return KW_IN; }
"Array" { return KW_ARRAY; }
"method" { return KW_METHOD; }
"class" { return KW_CLASS; }
"public" { return KW_PUBLIC; }
"private" { return KW_PRIVATE; }
"if" { return KW_IF; }
"then" { return KW_THEN; }
"else" { return KW_ELSE; }
"begin" { return KW_BEGIN; }
"end" { return KW_END; }
"while" { return KW_WHILE; }
"do" { return KW_DO; }
"repeat" { return KW_REPEAT; }
"until" { return KW_UNTIL; }
"break" { return KW_BREAK; }
"var" { return KW_VAR; }
"ret" { return KW_RET; }

"bool" { return TYPE_BOOL; }
"byte" { return TYPE_BYTE; }
"int" { return TYPE_INT; }
"uint" { return TYPE_UINT; }
"long" { return TYPE_LONG; }
"ulong" { return TYPE_ULONG; }
"char" { return TYPE_CHAR; }
"string" { return TYPE_STRING; }

\"[^\"\\]*(?:\\.[^\"\\]*)*\" { yylval.strval = strdup(yytext); return LIT_STR; }
'\\[nrt0'\"\\]' {
  switch(yytext[2]) {
    case 'n': yylval.strval = strdup("\n"); break;
    case 'r': yylval.strval = strdup("\r"); break;
    case 't': yylval.strval = strdup("\t"); break;
    case '0': yylval.strval = strdup("\0"); break;
    default: yylval.strval = strdup(&yytext[2]); break;
  }
  return LIT_CHAR;
}
'[^'\\]' {
  yylval.strval = strdup(&yytext[1]);
  return LIT_CHAR;
}
0[xX][0-9A-Fa-f]+ {yylval.strval = strdup(yytext); return LIT_HEX; }
0[bB][01]+ { yylval.strval = strdup(yytext); return LIT_BITS; }
[0-9]+ { yylval.strval = strdup(yytext); return LIT_DEC; }
"true"|"false" { yylval.strval = strdup(yytext); return LIT_BOOL; }
[a-zA-Z_][a-zA-Z_0-9]* { yylval.strval = strdup(yytext); return IDENTIFIER; }

"(" { return SYM_LPAREN_CIRCLE; } 
")" { return SYM_RPAREN_CIRCLE; }
"[" { return SYM_LPAREN_SQUARE; } 
"]" { return SYM_RPAREN_SQUARE; }

":" { return SYM_COLON; }
"," { return SYM_COMMA; }
";" { return SYM_SEMICOLON; }
"." { return SYM_DOT; }

":=" { return SYM_ASSIGN; }

"+" { return SYM_PLUS; } 
"-" { return SYM_MINUS; }
"/" { return SYM_DIV; } 
"%" { return SYM_PERCENT; } 
"*" { return SYM_MUL; }

"==" { return SYM_EQ; }
"!=" { return SYM_NEQ; }
">" { return SYM_GT; }
">=" { return SYM_GTE; }
"<" { return SYM_LT; }
"<=" { return SYM_LTE; }

"&" { return SYM_AMPERSAND; }
"|" { return SYM_VSPLASH; }
">>" { return SYM_SHIFT_R; }
"<<" { return SYM_SHIFT_L; }
"||" { return SYM_OR; } 
"&&" { return SYM_AND; }

"++" { return SYM_INC; } 
"--" { return SYM_DEC; }
"~" { return SYM_TILDA; }
"!" { return SYM_NOT; }

. { printf("LEXER: Unknown syntax: %s\n", yytext); }
%%
