#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "utils.h"
#include "lexer.h"
#include "parser.h"
#include "raylib.h"

#define MAX_HEADER_LEVEL 6

void parse_paragraph(ASTLinkedList *children, const char *initial_text);

void parse_text(ASTLinkedList *children)
{
    ASTItem *last_item = children->tail;
    TextNode *t;
    if(last_item != NULL && last_item->type == AST_TEXT_NODE) {
        t = (TextNode*)last_item->data;
    } else {
        t = allocate_node(sizeof(TextNode));
        create_and_add_item(children, AST_TEXT_NODE, t);
    }

    da_append(&t->str, lexer_consume());
    while(isalpha(lexer_peek())) {
        da_append(&t->str, lexer_consume());
    }
}

void parse_inline(ASTLinkedList *children)
{
    parse_text(children);
}

// returns true when an atx closing sequence is found
// NOTE: this function consumes all the characters that are part
// of the closing sequence when found
bool atx_closing_found()
{
    if(lexer_is_next_terminal()) return true;

    int start_pos = lexer_get_cur_pos();

    if(!lexer_is_next_whitespace()) return false;

    lexer_consume_whitespaces();
    lexer_consume_all('#');
    lexer_consume_whitespaces();

    if(!lexer_is_next_terminal()) {
        lexer_set_cur_pos(start_pos);
        return false;
    }

    return true;
}

bool parse_atx_heading(ASTLinkedList *children)
{
    int start_pos = lexer_get_cur_pos();
    int level = 0;

    while(lexer_match('#') && level <= MAX_HEADER_LEVEL) level++;

    if(
        (level < 1 || level > MAX_HEADER_LEVEL)
        || (!lexer_is_next_whitespace() && !lexer_is_next_terminal())
    ) {
        lexer_set_cur_pos(start_pos);
        return false;
    }

    lexer_consume_whitespaces();

    HeaderNode *h = allocate_node(sizeof(HeaderNode));
    h->level = level;

    while(!atx_closing_found()) {
        parse_inline(&h->children);
    }

    lexer_match('\n');

    create_and_add_item(children, AST_HEADER_NODE, h);
    return true;
}

bool parse_horizontal_rule(ASTLinkedList *children)
{
    int start_pos = lexer_get_cur_pos();

    lexer_consume_whitespaces();

    if(!lexer_match_many("*-_")) {
        lexer_set_cur_pos(start_pos);
        return false;
    }

    char indicator = lexer_prev();
    int ind_count = 1;

    lexer_consume_whitespaces();
    while(!lexer_is_next_terminal() && lexer_match(indicator)) {
        lexer_consume_whitespaces();
        ind_count++;
    }

    if(ind_count < 3 || !lexer_is_next_terminal()) {
        lexer_set_cur_pos(start_pos);
        return false;
    }

    lexer_match('\n');

    create_and_add_item(children, AST_HR_NODE, NULL);
    return true;
}

void parse_paragraph(ASTLinkedList *children, const char *initial_text)
{
    ParagraphNode *p = allocate_node(sizeof(ParagraphNode));

    if(initial_text != NULL) {
        add_text_node(&p->children, initial_text);
    }

    while(!lexer_is_next_terminal()) {
        parse_inline(&p->children);
    }

    create_and_add_item(children, AST_PARAGRAPH_NODE, p);
}

void parse_block(ASTLinkedList *children)
{
    if(parse_atx_heading(children)) return;
    if(parse_horizontal_rule(children)) return;

    parse_paragraph(children, NULL);
}

DocumentNode *parse_document()
{
    DocumentNode *doc = allocate_node(sizeof(DocumentNode));

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
