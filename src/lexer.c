#include <stdio.h>
#include <string.h>

#include "lexer.h"

Lexer lexer = {0};

void lexer_init(const char *buf)
{
    lexer.buf = buf;
}

int lexer_get_cur_pos(void)
{
    return lexer.buf_cur;
}
void lexer_set_cur_pos(int pos)
{
    lexer.buf_cur = pos;
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

void lexer_consume_whitespaces()
{
    while(lexer_is_next_whitespace()) lexer_consume();
}

void lexer_consume_all(char c)
{
    while(lexer_match(c));
}

bool lexer_match(char c)
{
    if(lexer_peek() == c) {
        lexer_consume();
        return true;
    }

    return false;
}

bool lexer_match_many(const char *chars)
{
    for(size_t i = 0; i < strlen(chars); i++) {
        if(lexer_match(chars[i])) return true;
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

bool lexer_is_next_whitespace()
{
    char c = lexer_peek();
    return c == ' ' || c == '\t';
}
