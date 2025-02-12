#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "raylib.h"

void ast_free_list(LList *list)
{
    LNode *node = list->head;

    while(node != NULL) {
        LNode *_node = node;
        node = node->next;
        ast_free_node(_node);
    }

    llist_destroy(list);
}

void ast_free_node(LNode *node)
{
    switch((ASTNodeType) node->type) {
        case AST_DOCUMENT_NODE:
        case AST_PARAGRAPH_NODE:
        case AST_BLOCKQUOTE_NODE:
        case AST_LIST_ITEM_NODE:
        case AST_STRONG_NODE:
        case AST_EMPHASIS_NODE: {
            ParentNode *parent = (ParentNode*)node->data;
            ast_free_list(parent->children);
            free(parent);
        } break;
        case AST_TEXT_NODE: {
            TextNode *t = (TextNode*)node->data;
            da_free(&t->str);
            free(t);
        } break;
        case AST_HEADER_NODE: {
            HeaderNode *h = (HeaderNode*)node->data;
            ast_free_list(h->children);
            free(h);
        } break;
        case AST_CODE_SPAN_NODE: {
            CodeSpanNode *code = (CodeSpanNode*)node->data;
            da_free(&code->content);
            free(code);
        } break;
        case AST_LINK_NODE: {
            LinkNode *link = (LinkNode*)node->data;
            ast_free_list(link->text);
            da_free(&link->dest);
            free(link);
        } break;
        case AST_IMAGE_NODE: {
            ImageNode *img = (ImageNode*)node->data;
            string_free(&img->desc);
            string_free(&img->uri);
            free(img);
        } break;
        case AST_LIST_NODE: {
            ListNode *l = (ListNode*)node->data;
            ast_free_list(l->children);
            free(l);
        } break;
        case AST_SB_NODE:
        case AST_HR_NODE:
            break;
    }
}
