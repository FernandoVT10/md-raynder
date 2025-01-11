#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

typedef struct {
    const char *buf;
    int buf_cur;
} Lexer;

void lexer_init(const char *buf);

char lexer_peek(void); // returns current char without advancing the cursor
char lexer_consume(void); // returns char and advance cursor

bool lexer_match(char c); // returns true and advances cursor when the next char matches
bool lexer_is_next_terminal(void); // returns true if next char is EOF or '\n'
bool lexer_is_at_end(void); // returns true if "buf_cur" is at the end of buffer

#endif // LEXER_H
