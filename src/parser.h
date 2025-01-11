#ifndef PARSER_H
#define PARSER_H

#include "nodes.h"

ASTItem *parse_file(const char *file_path);

void parser_free_ast(ASTItem *doc_item);

#endif // PARSER_H
