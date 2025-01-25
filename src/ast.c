#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "raylib.h"

ASTItem *ast_create_item(ASTNodeType type, void *data)
{
    ASTItem *item = allocate(sizeof(ASTItem));
    item->type = type;
    item->data = data;
    item->next = NULL;
    return item;
}

void ast_add_item(ASTList *list, ASTItem *item)
{
    if(list->count == 0) {
        list->tail = list->head = item;
    } else {
        ASTItem *last_item = list->tail;
        last_item->next = item;
        list->tail = item;
    }

    list->count++;
}

void ast_list_create_and_add(ASTList *list, ASTNodeType type, void *data)
{
    ast_add_item(list, ast_create_item(type, data));
}

void ast_free_list(ASTList *list)
{
    ASTItem *item = list->head;

    while(item != NULL) {
        ASTItem *item_to_free = item;
        item = item->next;
        ast_free_item(item_to_free);
    }
}

void ast_free_item(ASTItem *item)
{
    switch(item->type) {
        case AST_DOCUMENT_NODE:
        case AST_BLOCKQUOTE_NODE:
        case AST_LIST_ITEM_NODE:
        case AST_STRONG_NODE:
        case AST_EMPHASIS_NODE: {
            ParentNode *parent = (ParentNode*)item->data;
            ast_free_list(&parent->children);
            free(parent);
        } break;
        case AST_PARAGRAPH_NODE: {
            ParagraphNode *p = (ParagraphNode*)item->data;
            ast_free_list(&p->children);
            free(p);
        } break;
        case AST_TEXT_NODE: {
            TextNode *t = (TextNode*)item->data;
            da_free(&t->str);
            free(t);
        } break;
        case AST_HEADER_NODE: {
            HeaderNode *h = (HeaderNode*)item->data;
            ast_free_list(&h->children);
            free(h);
        } break;
        case AST_CODE_SPAN_NODE: {
            CodeSpanNode *code = (CodeSpanNode*)item->data;
            da_free(&code->content);
            free(code);
        } break;
        case AST_LINK_NODE: {
            LinkNode *link = (LinkNode*)item->data;
            ast_free_list(&link->text);
            da_free(&link->dest);
            free(link);
        } break;
        case AST_IMAGE_NODE: {
            ImageNode *img = (ImageNode*)item->data;
            string_free(&img->desc);
            string_free(&img->uri);
            free(img);
        } break;
        case AST_LIST_NODE: {
            ListNode *l = (ListNode*)item->data;
            ast_free_list(&l->children);
            free(l);
        } break;
        case AST_SB_NODE:
        case AST_HR_NODE:
            break;
    }

    free(item);
}

void ast_add_text(ASTList *list, const char *text)
{
    ASTItem *last_item = list->tail;
    TextNode *t;
    if(last_item != NULL && last_item->type == AST_TEXT_NODE) {
        t = (TextNode*)last_item->data;
    } else {
        t = allocate(sizeof(TextNode));
        ast_list_create_and_add(list, AST_TEXT_NODE, t);
    }

    string_append_str(&t->str, text);
}

void ast_catenate_lists(ASTList *dest, const ASTList *src)
{
    if(src->count == 0) return;

    if(dest->count > 0) {
        ASTItem *last_item = dest->tail;
        last_item->next = src->head;
        dest->tail = src->head;
        dest->count += src->count;
    } else {
        *dest = *src;
    }
}
