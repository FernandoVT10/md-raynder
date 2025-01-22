#ifndef NODES_H
#define NODES_H

#include "utils.h"

typedef enum {
    AST_DOCUMENT_NODE,
    AST_PARAGRAPH_NODE,
    AST_TEXT_NODE,
    AST_HEADER_NODE,
    AST_HR_NODE,
    AST_CODE_SPAN_NODE,
    AST_STRONG_NODE,
    AST_EMPHASIS_NODE,
    AST_LINK_NODE,
    AST_IMAGE_NODE,
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
} ASTList;

typedef struct {
    ASTList children;
} ParentNode;

typedef ParentNode DocumentNode;
typedef ParentNode ParagraphNode;

typedef struct {
    String str;
} TextNode;

typedef struct {
    ASTList children;
    int level;
} HeaderNode;

typedef struct {
    String content;
} CodeSpanNode;

typedef ParentNode StrongNode;
typedef ParentNode EmphasisNode;

typedef struct {
    ASTList text;
    String dest;
} LinkNode;

typedef struct {
    String desc;
    String uri;
} ImageNode;

ASTItem *ast_create_item(ASTNodeType type, void *data);
void ast_add_item(ASTList *list, ASTItem *item); // adds and ASTItem to an ASTList
void ast_list_create_and_add(ASTList *list, ASTNodeType type, void *data); // creates ASTItem and adds it to the list
void ast_free_list(ASTList *list);
void ast_free_item(ASTItem *item);
void ast_add_text(ASTList *list, const char *text); // Adds or catenates (if the last item of the list is a text node) a text node
void ast_catenate_lists(ASTList *dest, const ASTList *src); // Adds the elements of the src list to the dest list

#endif
