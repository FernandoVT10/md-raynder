#ifndef NODES_H
#define NODES_H

#include "utils.h"

typedef enum {
    AST_DOCUMENT_NODE,
    AST_PARAGRAPH_NODE,
    AST_TEXT_NODE,
    AST_HEADER_NODE,
} ASTNodeType;

typedef struct ASTItem ASTItem;

struct ASTItem {
    ASTNodeType type;
    void *data;
    ASTItem *next;
};

typedef struct {
    ASTItem *head;
    ASTItem *tail;
    size_t count;
} ASTLinkedList;

typedef struct {
    ASTLinkedList children;
} ParentNode;

typedef ParentNode DocumentNode;
typedef ParentNode ParagraphNode;

typedef struct {
    String str;
} TextNode;

typedef struct {
    ASTLinkedList children;
    int level;
} HeaderNode;

ASTItem *create_ast_item(ASTNodeType type, void *data);
void add_ast_item(ASTLinkedList *children, ASTItem *item);
void create_and_add_item(ASTLinkedList *children, ASTNodeType type, void *data);
void free_item(ASTItem *item);

void add_text_node(ASTLinkedList *children, const char *text);
void *allocate_node(size_t size);

#endif
