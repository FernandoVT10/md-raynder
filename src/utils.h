#ifndef UTILS_H
#define UTILS_H

#include <assert.h>

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

char *load_file_contents(const char *path); // returns the contents of a file (should be freed)

void string_append_char(String *str, char c); // appends char to string
char *string_dump(String str); // returns a string null-terminated (should be freed)

#endif // UTILS_H
