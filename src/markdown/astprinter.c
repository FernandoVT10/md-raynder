#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "ast.h"

void print_spaces(int n)
{
    for(size_t i = 0; i < n; i++) putchar(' ');
}

void print_open_tag(const char *text, int spaces)
{
    print_spaces(spaces);
    printf("%s { \n", text);
}

void print_close_tag(int spaces)
{
    print_spaces(spaces);
    printf("}\n");
}

void print_ast_list(LList *list, int spaces);

void print_ast_node(LNode *node, int spaces)
{
    switch(node->type) {
        case AST_DOCUMENT_NODE: {
            DocumentNode *doc = (DocumentNode*)node->data;
            print_open_tag("DOCUMENT", spaces);
                print_ast_list(doc->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_PARAGRAPH_NODE: {
            ParagraphNode *p = (ParagraphNode*)node->data;
            print_open_tag("PARAGRAPH", spaces);
                print_ast_list(p->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_TEXT_NODE: {
            TextNode *t = (TextNode*)node->data;
            print_spaces(spaces);
            char *text = string_dump(t->str);
            printf("TEXT(%s)\n", text);
            free(text);
        } break;
        case AST_HEADER_NODE: {
            HeaderNode *h = (HeaderNode*)node->data;
            print_open_tag(TextFormat("HEADER(%d)", h->level), spaces);
                print_ast_list(h->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_HR_NODE: {
            print_spaces(spaces);
            printf("HORIZONTAL RULE {}\n");
        } break;
        case AST_CODE_SPAN_NODE: {
            CodeSpanNode *code = (CodeSpanNode*)node->data;
            print_open_tag("CODE_SPAN", spaces);
                print_spaces(spaces + 4);
                printf("TEXT(%s)\n", string_dump(code->content));
            print_close_tag(spaces);
        } break;
        case AST_STRONG_NODE: {
            StrongNode *s = (StrongNode*)node->data;
            print_open_tag("STRONG", spaces);
                print_ast_list(s->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_EMPHASIS_NODE: {
            EmphasisNode *e = (EmphasisNode*)node->data;
            print_open_tag("EMPHASIS", spaces);
                print_ast_list(e->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_LINK_NODE: {
            LinkNode *link = (LinkNode*)node->data;
            print_open_tag("LINK", spaces);
                print_open_tag("LINK_TEXT", spaces + 4);
                    print_ast_list(link->text, spaces + 4);
                print_close_tag(spaces + 4);

            if(link->dest.count > 0) {
                print_spaces(spaces + 4);
                printf("DEST(%s)\n", string_dump(link->dest));
            }
            print_close_tag(spaces);
        } break;
        case AST_IMAGE_NODE: {
            ImageNode *img = (ImageNode*)node->data;
            print_open_tag("IMAGE", spaces);
                print_spaces(spaces + 4);
                printf("DESCRIPTION(%s)\n", string_dump(img->desc));


            if(img->uri.count > 0) {
                print_spaces(spaces + 4);
                printf("URI(%s)\n", string_dump(img->uri));
            }
            print_close_tag(spaces);
        } break;
        case AST_BLOCKQUOTE_NODE: {
            BlockquoteNode *q = (BlockquoteNode*)node->data;
            print_open_tag("BLOCKQUOTE", spaces);
                print_ast_list(q->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_SB_NODE: {
            print_spaces(spaces);
            printf("(SOFT_BREAK)\n");
        } break;
        case AST_LIST_NODE: {
            ListNode *list = (ListNode*)node->data;
            const char *text = list->ordered ? "ORDERED" : "UNORDERED";

            print_spaces(spaces);
            printf("LIST(%s) {\n", text);
                print_ast_list(list->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_LIST_ITEM_NODE: {
            ListItemNode *l_item = (ListItemNode*)node->data;
            print_open_tag("LIST_ITEM", spaces);
                print_ast_list(l_item->children, spaces);
            print_close_tag(spaces);
        } break;
    }
}

void print_ast_list(LList *list, int spaces)
{
    LNode *node = list->head;
    while(node != NULL) {
        print_ast_node(node, spaces + 4);

        node = node->next;
    }
}

void print_ast(LNode *doc_node)
{
    print_ast_node(doc_node, 0);
}
