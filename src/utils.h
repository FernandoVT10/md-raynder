#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define TODO(message) do { fprintf(stderr, "%s:%d: TODO: %s\n", __FILE__, __LINE__, message); abort(); } while(0)

#define DA_INIT_CAP 128

#define da_append(da, item)                                                          \
    do {                                                                             \
        if((da)->count >= (da)->capacity) {                                          \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "No enough ram");                          \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while(0)

#define da_free(da) do { free((da)->items); } while(0)

typedef struct {
    char *items; // this is not null-terminated
    size_t count;
    size_t capacity;
} String;

typedef struct LNode LNode;

struct LNode {
    int type;
    void *data;
    LNode *next;
};

typedef struct {
    LNode *head;
    LNode *tail;
    size_t count;
} LList;

char *load_file_contents(const char *path); // returns the contents of a file (should be freed)

void string_append_char(String *str, char c);
void string_append_str(String *str, const char *l_str); // appends string literal
char *string_dump(String str); // returns a string null-terminated (should be freed

// returns the index (starting from "start") if "c" is found. If "c" is not found, it returns -1
int string_find_index(String str, int start, char c);
void string_free(String *str); // frees string

// allocates you "size" bytes of 0 initialized memory
// NOTE: it aborts when malloc fails
void *allocate(size_t size);

LList *llist_create();
void llist_append_node(LList *list, int type, void *data);
void llist_destroy(LList *list);

#endif // UTILS_H
