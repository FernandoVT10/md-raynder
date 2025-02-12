#ifndef NODES_H
#define NODES_H

#include <stdbool.h>
#include "../utils.h"

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
    AST_BLOCKQUOTE_NODE,
    AST_SB_NODE, // soft-break node
    AST_LIST_NODE,
    AST_LIST_ITEM_NODE,
} ASTNodeType;

typedef struct {
    LList *children;
} ParentNode;

typedef ParentNode DocumentNode;

typedef ParentNode ParagraphNode;
typedef ParentNode BlockquoteNode;

typedef struct {
    LList *children;
    bool ordered;
} ListNode;
typedef ParentNode ListItemNode;

typedef struct {
    String str;
} TextNode;

typedef struct {
    LList *children;
    int level;
} HeaderNode;

typedef struct {
    String content;
} CodeSpanNode;

typedef ParentNode StrongNode;
typedef ParentNode EmphasisNode;

typedef struct {
    LList *text;
    String dest;
} LinkNode;

typedef struct {
    String desc;
    String uri;
} ImageNode;

void ast_free_list(LList *list);
void ast_free_node(LNode *node);

#endif
