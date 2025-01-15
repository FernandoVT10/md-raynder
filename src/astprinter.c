#include <stdio.h>
#include <stdlib.h>

#include "raylib.h"
#include "nodes.h"

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

void print_children(ASTLinkedList children, int spaces);

void print_item(ASTItem *item, int spaces)
{
    switch(item->type) {
        case AST_DOCUMENT_NODE: {
            DocumentNode *doc = (DocumentNode*)item->data;
            print_open_tag("DOCUMENT", spaces);
                print_children(doc->children, spaces);
            print_close_tag(spaces);
        } break;
        case AST_PARAGRAPH_NODE: {
            ParagraphNode *p = (ParagraphNode*)item->data;
            print_open_tag("PARAGRAPH", spaces);
                print_children(p->children, spaces);
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
                print_children(h->children, spaces);
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
        case AST_EMPHASIS_NODE: {
            EmphasisNode *e = (EmphasisNode*)item->data;
            print_open_tag("EMPHASIS", spaces);
                print_children(e->children, spaces);
            print_close_tag(spaces);
        } break;
    }
}

void print_children(ASTLinkedList children, int spaces)
{
    ASTItem *item = children.head;
    while(item != NULL) {
        print_item(item, spaces + 4);

        item = item->next;
    }
}

void print_ast(ASTItem *doc_item)
{
    print_item(doc_item, 0);
}
