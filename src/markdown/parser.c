#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "../utils.h"
#include "lexer.h"
#include "parser.h"
#include "raylib.h"

#define MAX_HEADER_LEVEL 6
#define MAX_BACKTICK_COUNT 5

void parse_inline(LList *children);
void parse_block(LList *children);
bool parse_horizontal_rule(LList *children);

bool parse_code_span(LList *children)
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

    llist_append_node(children, AST_CODE_SPAN_NODE, code);
    return true;
}

bool parse_strong(LList *children)
{
    int start_pos = lexer_get_cur_pos();
    if(!(lexer_match_many("*_") && lexer_match_many("*_")) || lexer_is_next_whitespace()) {
        lexer_set_cur_pos(start_pos);
        return false;
    }

    char indicator = lexer_prev();
    LList *s_children = llist_create();

    bool closing_found = false;

    int cur_pos = 0;

    while(!lexer_is_next_terminal()) {
        cur_pos = lexer_get_cur_pos();
        if(!lexer_is_prev_whitespace()
            && lexer_match(indicator)
            && lexer_match(indicator)
        ) {
            closing_found = true;
            break;
        } else {
            lexer_set_cur_pos(cur_pos);
        }

        parse_inline(s_children);
    }

    if(s_children->count == 0 || !closing_found) {
        // TODO: we have already the children list, we could reuse that
        ast_free_list(s_children);
        lexer_set_cur_pos(start_pos);
        return false;
    }

    StrongNode *s = allocate(sizeof(StrongNode));
    s->children = s_children;
    llist_append_node(children, AST_STRONG_NODE, s);

    return true;
}

bool parse_emphasis(LList *children)
{
    int start_pos = lexer_get_cur_pos();

    if(!lexer_match_many("*_") || lexer_is_next_whitespace()) {
        lexer_set_cur_pos(start_pos);
        return false;
    }

    char indicator = lexer_prev();

    LList *e_children = llist_create();

    bool closing_found = false;

    while(!lexer_is_next_terminal()) {
        if(!lexer_is_prev_whitespace() && lexer_match(indicator)) {
            closing_found = true;
            break;
        }

        parse_inline(e_children);
    }

    if(e_children->count == 0 || !closing_found) {
        ast_free_list(e_children);
        lexer_set_cur_pos(start_pos);
        return false;
    }

    EmphasisNode *e = allocate(sizeof(EmphasisNode));
    e->children = e_children;
    llist_append_node(children, AST_EMPHASIS_NODE, e);
    return true;
}

bool parse_link_dest(String *dest)
{
    if(!lexer_match('(')) return false;

    while(!lexer_is_next_terminal() && lexer_peek() != ')') {
        string_append_char(dest, lexer_consume());
    }

    if(!lexer_match(')')) {
        return false;
    }

    return true;
}

bool parse_link(LList *children)
{
    int start_pos = lexer_get_cur_pos();

    if(!lexer_match('[')) {
        return false;
    }

    LList* link_text = llist_create();

    while(!lexer_is_next_terminal() && lexer_peek() != ']') {
        parse_inline(link_text);
    }

    if(link_text->count == 0 || !lexer_match(']')) {
        lexer_set_cur_pos(start_pos);
        ast_free_list(link_text);
        return false;
    }

    String dest = {0};

    start_pos = lexer_get_cur_pos();
    if(!parse_link_dest(&dest)) {
        dest.count = 0;
        string_free(&dest);
        lexer_set_cur_pos(start_pos);
    }

    LinkNode *link = allocate(sizeof(LinkNode));
    link->text = link_text;
    if(dest.count > 0) {
        link->dest = dest;
    }
    llist_append_node(children, AST_LINK_NODE, link);
    return true;
}

bool parse_image(LList *children)
{
    int start_pos = lexer_get_cur_pos();
    if(!(lexer_match('!') && lexer_match('['))) {
        lexer_set_cur_pos(start_pos);
        return false;
    }

    String desc = {0};

    while(!lexer_is_next_terminal() && lexer_peek() != ']') {
        string_append_char(&desc, lexer_consume());
    }

    if(!lexer_match(']')) {
        string_free(&desc);
        lexer_set_cur_pos(start_pos);
        return false;
    }

    String uri = {0};
    start_pos = lexer_get_cur_pos();
    if(!parse_link_dest(&uri)) {
        uri.count = 0;
        string_free(&uri);
        lexer_set_cur_pos(start_pos);
    }

    ImageNode *img = allocate(sizeof(ImageNode));
    img->desc = desc;
    if(uri.count > 0) {
        img->uri = uri;
    }
    llist_append_node(children, AST_IMAGE_NODE, img);
    return true;
}

void parse_text(LList *children)
{
    LNode *last_node = children->tail;
    TextNode *t;
    if(last_node != NULL && last_node->type == AST_TEXT_NODE) {
        t = (TextNode*)last_node->data;
    } else {
        t = allocate(sizeof(TextNode));
        llist_append_node(children, AST_TEXT_NODE, t);
    }

    da_append(&t->str, lexer_consume());
    while(isalpha(lexer_peek())) {
        da_append(&t->str, lexer_consume());
    }
}

void parse_inline(LList *children)
{
    if(parse_code_span(children)) return;
    if(parse_strong(children)) return;
    if(parse_emphasis(children)) return;
    if(parse_link(children)) return;
    if(parse_image(children)) return;

    parse_text(children);
}

bool parse_blockquote(LList *children)
{
    int start_pos = lexer_get_cur_pos();
    LList *q_children = llist_create();

    while(lexer_match('>')) {
        lexer_consume_whitespaces();
        parse_block(q_children);
    }

    if(q_children->count == 0) {
        lexer_set_cur_pos(start_pos);
        ast_free_list(q_children);
        return false;
    }

    BlockquoteNode *q = allocate(sizeof(BlockquoteNode));
    q->children = q_children;
    llist_append_node(children, AST_BLOCKQUOTE_NODE, q);
    return true;
}

bool parse_ulist_item(LList *items, char *marker)
{
    int start_pos = lexer_get_cur_pos();
    lexer_consume_whitespaces();

    if(*marker == '\0') {
        if(!lexer_match_many("-*+")) {
            lexer_set_cur_pos(start_pos);
            return false;
        }

        *marker = lexer_prev();
    } else {
        if(!lexer_match(*marker)) {
            lexer_set_cur_pos(start_pos);
            return false;
        }
    }

    if(!lexer_is_next_whitespace()) {
        lexer_set_cur_pos(start_pos);
        return false;
    }

    lexer_consume_whitespaces();

    ListItemNode *item = allocate(sizeof(ListItemNode));
    parse_block(item->children);
    llist_append_node(items, AST_LIST_ITEM_NODE, item);
    return true;
}

bool parse_ulist(LList *children)
{
    int start_pos = lexer_get_cur_pos();
    LList *items = llist_create();

    char marker = '\0';
    while(parse_ulist_item(items, &marker));

    if(items->count == 0) {
        lexer_set_cur_pos(start_pos);
        ast_free_list(items);
        return false;
    }

    ListNode *l = allocate(sizeof(ListNode));
    l->children = items;
    l->ordered = false;
    llist_append_node(children, AST_LIST_NODE, l);
    return true;
}

bool parse_olist_item(LList *items)
{
    int start_pos = lexer_get_cur_pos();
    lexer_consume_whitespaces();

    if(!isdigit(lexer_peek())) {
        lexer_set_cur_pos(start_pos);
        return false;
    }
    // consume all subsequent digits
    while(isdigit(lexer_peek())) lexer_consume();

    if(!(lexer_match_many(".)") && lexer_is_next_whitespace())) {
        lexer_set_cur_pos(start_pos);
        return false;
    }

    lexer_consume_whitespaces();

    ListItemNode *item = allocate(sizeof(ListItemNode));
    parse_block(item->children);
    llist_append_node(items, AST_LIST_ITEM_NODE, item);
    return true;
}

bool parse_olist(LList *children)
{
    int start_pos = lexer_get_cur_pos();
    LList *items = llist_create();

    while(parse_olist_item(items));

    if(items->count == 0) {
        lexer_set_cur_pos(start_pos);
        ast_free_list(items);
        return false;
    }

    ListNode *l = allocate(sizeof(ListNode));
    l->children = items;
    l->ordered = true;
    llist_append_node(children, AST_LIST_NODE, l);
    return true;
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

bool parse_atx_heading(LList *children)
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
    h->children = llist_create();

    while(!atx_closing_found()) {
        parse_inline(h->children);
    }

    lexer_match('\n');

    llist_append_node(children, AST_HEADER_NODE, h);
    return true;
}

bool parse_horizontal_rule(LList *children)
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

    llist_append_node(children, AST_HR_NODE, NULL);
    return true;
}

void parse_paragraph(LList *children)
{
    ParagraphNode *p = allocate(sizeof(ParagraphNode));
    p->children = llist_create();

    while(!lexer_is_next_terminal()) {
        parse_inline(p->children);
    }

    llist_append_node(children, AST_PARAGRAPH_NODE, p);

    lexer_match('\n');
}

bool consume_blankline()
{
    int start_pos = lexer_get_cur_pos();
    if(lexer_prev() == '\n') {
        lexer_consume_whitespaces();

        if(lexer_match('\n')) {
            return true;
        } else {
            lexer_set_cur_pos(start_pos);
        }
    }

    return false;
}

void parse_block(LList *children)
{
    if(consume_blankline()) return;
    if(parse_blockquote(children)) return;
    if(parse_horizontal_rule(children)) return;
    if(parse_ulist(children)) return;
    if(parse_olist(children)) return;
    if(parse_atx_heading(children)) return;

    parse_paragraph(children);
}

DocumentNode *parse_document()
{
    DocumentNode *doc = allocate(sizeof(DocumentNode));
    doc->children = llist_create();

    while(!lexer_is_at_end()) {
        parse_block(doc->children);
    }

    return doc;
}

LNode *parse_md_file(const char *file_path)
{
    char *file_content = load_file_contents(file_path);
    if(file_content == NULL) return NULL;

    lexer_init(file_content);

    LNode *doc_node = allocate(sizeof(LNode));
    doc_node->type = AST_DOCUMENT_NODE;
    doc_node->data = parse_document();
    return doc_node;
}
