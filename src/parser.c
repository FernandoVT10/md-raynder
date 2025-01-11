#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "utils.h"
#include "lexer.h"
#include "parser.h"
#include "raylib.h"

void parse_inline(ASTLinkedList *children)
{
    TextNode *t = calloc(sizeof(TextNode), 1);

    while(!lexer_is_next_terminal()) {
        string_append_char(&t->str, lexer_consume());
    }

    create_and_add_item(children, AST_TEXT_NODE, t);
}

void parse_paragraph(ASTLinkedList *children)
{
    ParagraphNode *p = calloc(sizeof(ParagraphNode), 1);

    while(!lexer_is_next_terminal()) {
        parse_inline(&p->children);
    }

    create_and_add_item(children, AST_PARAGRAPH_NODE, p);
}

void parse_block(ASTLinkedList *children)
{
    parse_paragraph(children);
}

DocumentNode *parse_document()
{
    DocumentNode *doc = calloc(sizeof(DocumentNode), 1);

    while(!lexer_is_at_end()) {
        parse_block(&doc->children);
        while(lexer_match('\n'));
    }

    return doc;
}

ASTItem *parse_file(const char *file_path)
{
    char *file_content = load_file_contents(file_path);
    if(file_content == NULL) return NULL;

    lexer_init(file_content);

    return create_ast_item(AST_DOCUMENT_NODE, parse_document());
}

void parser_free_ast(ASTItem *doc_item)
{
    free_item(doc_item);
}
