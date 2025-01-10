#include "lexer.h"

Lexer lexer = {0};

void lexer_init(const char *buf)
{
    lexer.buf = buf;
}
