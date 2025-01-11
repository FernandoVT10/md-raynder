#include <stdlib.h>

#include "nodes.h"
#include "raylib.h"

ASTItem *create_ast_item(ASTNodeType type, void *data)
{
    ASTItem *item = malloc(sizeof(ASTItem));

    if(item == NULL) {
        TraceLog(LOG_ERROR, "Couldn't allocate memory for AST Node");
        exit(EXIT_FAILURE);
    }

    item->type = type;
    item->data = data;
    item->next = NULL;
    return item;
}

void add_ast_item(ASTLinkedList *children, ASTItem *item)
{
    if(children->count == 0) {
        children->tail = children->head = item;
    } else {
        ASTItem *last_item = children->tail;
        last_item->next = item;
        children->tail = item;
    }

    children->count++;
}

void create_and_add_item(ASTLinkedList *children, ASTNodeType type, void *data)
{
    add_ast_item(children, create_ast_item(type, data));
}

void free_children(ASTLinkedList *children)
{
    ASTItem *item = children->head;

    while(item != NULL) {
        ASTItem *item_to_free = item;
        item = item->next;
        free_item(item_to_free);
    }
}

void free_item(ASTItem *item)
{
    switch(item->type) {
        case AST_DOCUMENT_NODE: {
            DocumentNode *doc = (DocumentNode*)item->data;
            free_children(&doc->children);
            free(doc);
        } break;
        case AST_PARAGRAPH_NODE: {
            ParagraphNode *p = (ParagraphNode*)item->data;
            free_children(&p->children);
            free(p);
        } break;
        case AST_TEXT_NODE: {
            TextNode *t = (TextNode*)item->data;
            da_free(&t->str);
            free(t);
        } break;
    }

    free(item);
}