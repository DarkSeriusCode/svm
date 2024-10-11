#include <stdio.h>
#include "lexer.h"

void print_token(Token tok);

int main() {
    Lexer lexer = new_lexer("test.asm");

    Token tok;
    do {
        tok = lexer_get_next_token(&lexer);
        print_token(tok);
        free_token(tok);
    } while (tok.type != TOKEN_EOF);

    free_lexer(lexer);
    return 0;
}

void print_token(Token tok) {
    printf("%s: `%s` at (%lu, %lu)\n", token_type_to_str(tok.type), tok.value,
                                     tok.span.column, tok.span.line);
}
