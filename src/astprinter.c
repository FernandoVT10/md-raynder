#include <stdio.h>
#include <stdlib.h>

#include "nodes.h"

void print_spaces(int n)
{
    for(size_t i = 0; i < n; i++) putchar(' ');
}

void print_open_tag(const char *text, int spaces)
{
    print_spaces(spaces);
    printf(text);
    printf(" {\n");
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
