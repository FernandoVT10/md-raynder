#ifndef LEXER_H
#define LEXER_H

typedef struct {
    const char *buf;
    int buf_cur;
} Lexer;

void lexer_init(const char *buf);

#endif // LEXER_H
