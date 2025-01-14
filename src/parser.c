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
#define MAX_BACKTICK_COUNT 5

void parse_inline(ASTList *children);

bool parse_code_span(ASTList *children)
{
    int start_pos = lexer_get_cur_pos();
    int backtick_count = 0;
    while(lexer_match('`')) backtick_count++;

    if(backtick_count > MAX_BACKTICK_COUNT || backtick_count < 1) {
        lexer_set_cur_pos(start_pos);
        return false;
    }

    CodeSpanNode *code = allocate(sizeof(CodeSpanNode));
    bool closing_found = false;

    while(!lexer_is_next_terminal()) {
        if(lexer_match('`')) {
            int count = 1;
            while(lexer_match('`')) count++;

            if(count == backtick_count) {
                closing_found = true;
                break;
            }

            while(count > 0) {
                string_append_char(&code->content, '`');
                count--;
            }
        }

        string_append_char(&code->content, lexer_consume());
    }

    if(!closing_found) {
        da_free(&code->content);
        free(code);
        lexer_set_cur_pos(start_pos);
        return false;
    }

    ast_list_create_and_add(children, AST_CODE_SPAN_NODE, code);
    return true;
}

bool parse_emphasis(ASTList *children)
{
    int start_pos = lexer_get_cur_pos();

    if(!lexer_match_many("*_") || lexer_is_next_whitespace()) {
        lexer_set_cur_pos(start_pos);
        return false;
    }

    char indicator = lexer_prev();

    ASTList e_children = {0};

    bool closing_found = false;

    while(!lexer_is_next_terminal()) {
        if(!lexer_is_prev_whitespace() && lexer_match(indicator)) {
            closing_found = true;
            break;
        }

        parse_inline(&e_children);
    }

    if(e_children.count == 0) {
        lexer_set_cur_pos(start_pos);
        return false;
    }

    if(!closing_found) {
        // TODO: we have already the children list, we could reuse that
        ast_free_list(&e_children);
        lexer_set_cur_pos(start_pos);
        return false;
    }

    EmphasisNode *e = allocate(sizeof(EmphasisNode));
    e->children = e_children;
    ast_list_create_and_add(children, AST_EMPHASIS_NODE, e);
    return true;
}

void parse_text(ASTList *children)
{
    ASTItem *last_item = children->tail;
    TextNode *t;
    if(last_item != NULL && last_item->type == AST_TEXT_NODE) {
        t = (TextNode*)last_item->data;
    } else {
        t = allocate(sizeof(TextNode));
        ast_list_create_and_add(children, AST_TEXT_NODE, t);
    }

    da_append(&t->str, lexer_consume());
    while(isalpha(lexer_peek())) {
        da_append(&t->str, lexer_consume());
    }
}

void parse_inline(ASTList *children)
{
    if(parse_code_span(children)) return;
    if(parse_emphasis(children)) return;

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

bool parse_atx_heading(ASTList *children)
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

    HeaderNode *h = allocate(sizeof(HeaderNode));
    h->level = level;

    while(!atx_closing_found()) {
        parse_inline(&h->children);
    }

    lexer_match('\n');

    ast_list_create_and_add(children, AST_HEADER_NODE, h);
    return true;
}

bool parse_horizontal_rule(ASTList *children)
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

    ast_list_create_and_add(children, AST_HR_NODE, NULL);
    return true;
}

void parse_paragraph(ASTList *children)
{
    ParagraphNode *p = allocate(sizeof(ParagraphNode));

    while(!lexer_is_next_terminal()) {
        parse_inline(&p->children);
    }

    ast_list_create_and_add(children, AST_PARAGRAPH_NODE, p);
}

void parse_block(ASTList *children)
{
    if(parse_atx_heading(children)) return;
    if(parse_horizontal_rule(children)) return;

    parse_paragraph(children);
}

DocumentNode *parse_document()
{
    DocumentNode *doc = allocate(sizeof(DocumentNode));

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

    return ast_create_item(AST_DOCUMENT_NODE, parse_document());
}
