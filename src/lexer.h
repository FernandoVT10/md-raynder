#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>

typedef struct {
    const char *buf;
    int buf_cur;
} Lexer;

void lexer_init(const char *buf);

int lexer_get_cur_pos(void); // returns buf_cur pos
void lexer_set_cur_pos(int pos); // sets buf_cur pos

char lexer_peek(void); // returns current char without advancing the cursor

char lexer_consume(void); // returns char and advance cursor
void lexer_consume_whitespaces(); // consumes all whitespaces
void lexer_consume_all(char c); // consumes a sequence of the same char

bool lexer_match(char c); // returns true and advances cursor when the next char
// returns true and advances the cursor when the next char is inside chars
bool lexer_match_many(const char *chars);

bool lexer_is_next_terminal(void); // returns true if next char is EOF or '\n'
bool lexer_is_at_end(void); // returns true if "buf_cur" is at the end of buffer
bool lexer_is_next_whitespace(); // returns true if next char is ' ' or '\t'

#endif // LEXER_H
