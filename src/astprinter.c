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

void print_list(ASTList list, int spaces);

void print_item(ASTItem *item, int spaces)
{
    switch(item->type) {
        case AST_DOCUMENT_NODE: {
            DocumentNode *doc = (DocumentNode*)item->data;
            print_open_tag("DOCUMENT", spaces);
                print_list(doc->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_PARAGRAPH_NODE: {
            ParagraphNode *p = (ParagraphNode*)item->data;
            print_open_tag("PARAGRAPH", spaces);
                print_list(p->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_TEXT_NODE: {
            TextNode *t = (TextNode*)item->data;
            print_spaces(spaces);
            char *text = string_dump(t->str);
            printf("TEXT(%s)\n", text);
            free(text);
        } break;
        case AST_HEADER_NODE: {
            HeaderNode *h = (HeaderNode*)item->data;
            print_open_tag(TextFormat("HEADER(%d)", h->level), spaces);
                print_list(h->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_HR_NODE: {
            print_spaces(spaces);
            printf("HORIZONTAL RULE {}\n");
        } break;
        case AST_CODE_SPAN_NODE: {
            CodeSpanNode *code = (CodeSpanNode*)item->data;
            print_open_tag("CODE_SPAN", spaces);
                print_spaces(spaces + 4);
                printf("TEXT(%s)\n", string_dump(code->content));
            print_close_tag(spaces);
        } break;
        case AST_STRONG_NODE: {
            StrongNode *s = (StrongNode*)item->data;
            print_open_tag("STRONG", spaces);
                print_list(s->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_EMPHASIS_NODE: {
            EmphasisNode *e = (EmphasisNode*)item->data;
            print_open_tag("EMPHASIS", spaces);
                print_list(e->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_LINK_NODE: {
            LinkNode *link = (LinkNode*)item->data;
            print_open_tag("LINK", spaces);
                print_open_tag("LINK_TEXT", spaces + 4);
                    print_list(link->text, spaces + 4);
                print_close_tag(spaces + 4);

            if(link->dest.count > 0) {
                print_spaces(spaces + 4);
                printf("DEST(%s)\n", string_dump(link->dest));
            }
            print_close_tag(spaces);
        } break;
    }
}

void print_list(ASTList list, int spaces)
{
    ASTItem *item = list.head;
    while(item != NULL) {
        print_item(item, spaces + 4);

        item = item->next;
    }
}

void print_ast(ASTItem *doc_item)
{
    print_item(doc_item, 0);
}
