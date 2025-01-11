#include <stdio.h>
#include <string.h>

#include "lexer.h"

Lexer lexer = {0};

void lexer_init(const char *buf)
{
    lexer.buf = buf;
}

char lexer_get(int pos)
{
    // I don't know if this is a good idea
    if(pos < 0) return '\0';

    if(pos >= strlen(lexer.buf)) {
        return EOF;
    }

    return lexer.buf[lexer.buf_cur];
}

char lexer_peek()
{
    return lexer_get(lexer.buf_cur);
}

char lexer_consume()
{
    char c = lexer_get(lexer.buf_cur);
    lexer.buf_cur++;
    return c;
}

bool lexer_match(char c)
{
    if(lexer_peek() == c) {
        lexer_consume();
        return true;
    }

    return false;
}

bool lexer_is_next_terminal()
{
    char c = lexer_peek();
    return c == EOF || c == '\n';
}

bool lexer_is_at_end()
{
    return lexer.buf_cur >= strlen(lexer.buf);
}
