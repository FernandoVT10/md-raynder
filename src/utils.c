#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

#include "raylib.h"
#include "utils.h"

char *load_file_contents(const char *path)
{
    struct stat buf_stat;
    if(stat(path, &buf_stat) == -1) {
        TraceLog(LOG_ERROR, "Couldn't open file %s: %s", path, strerror(errno));
        return NULL;
    }

    if((buf_stat.st_mode & S_IFMT) != S_IFREG) {
        TraceLog(LOG_ERROR, "%s is not a valid file path", path);
        return NULL;
    }

    FILE *file = fopen(path, "r");

    if(file == NULL) {
        TraceLog(LOG_ERROR, "Couldn't open file %s: %s\n", path, strerror(errno));
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *text = calloc(sizeof(char), file_size + 1);

    if(text == NULL) {
        TraceLog(LOG_ERROR, "Couldn't allocate memory to contain the file");
        return NULL;
    }

    fread(text, file_size, sizeof(char), file);
    text[file_size] = '\0';

    fclose(file);

    return text;
}

void string_append_char(String *str, char c)
{
    da_append(str, c);
}

void string_append_str(String *str, const char *l_str)
{
    for(size_t i = 0; i < strlen(l_str); i++) {
        da_append(str, l_str[i]);
    }
}

char *string_dump(String str)
{
    char *res = malloc(str.count + 1);
    for(size_t i = 0; i < str.count; i++) {
        res[i] = str.items[i];
    }
    res[str.count] = '\0';
    return res;
}

int string_find_index(String str, int start, char c)
{
    if(start > str.count - 1) return -1;

    for(size_t i = start; i < str.count; i++) {
        if(str.items[i] == c) return i;
    }

    return -1;
}

void string_free(String *str)
{
    da_free(str);
}

void *allocate(size_t size)
{
    void *ptr = malloc(size);
    if(ptr == NULL) {
        TraceLog(LOG_ERROR, "Couldn't allocate memory :(");
        exit(EXIT_FAILURE);
    }
    memset(ptr, 0, size);
    return ptr;
}

LList *llist_create()
{
    return allocate(sizeof(LList));
}

void llist_append_node(LList *list, int type, void *data)
{
    LNode *node = allocate(sizeof(LNode));
    node->type = type;
    node->data = data;
    node->next = NULL;

    if(list->count == 0) {
        list->head = node;
        list->tail = node;
    } else {
        LNode *last_node = list->tail;
        last_node->next = node;
        list->tail = node;
    }

    list->count++;
}

void llist_destroy(LList *list)
{
    LNode *node = list->head;
    while(node != NULL) {
        LNode *free_node = node;
        node = node->next;
        free(free_node);
    }
    free(list);
}
