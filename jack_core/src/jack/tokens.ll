%{
    #include "jack/bqltokens.h"
%}

%%
[a-zA-Z][_a-zA-Z0-9]*       return BQL_SYMBOL;
"("                         return BQL_OPEN_PARENTHESES;
")"                         return BQL_CLOSE_PARENTHESES;
"<"                         return BQL_LESS_THAN_OP;
">"                         return BQL_GREATER_THAN_OP;
"=="                        return BQL_EQUAL;
","                         return BQL_COMMA;
"!"                         return BQL_EXCLAMATION;
[0-9]*                      return BQL_NUMBER;
[ \t\n]                     { /* ignore whitespace */ }
.                           printf("unexpected token: %s\n", yytext); return -1;
%%

void scan_string(const char* str)
{
    yy_switch_to_buffer(yy_scan_string(str));
}

void delete_current_buffer(void)
{
    yy_delete_buffer(YY_CURRENT_BUFFER);
}
