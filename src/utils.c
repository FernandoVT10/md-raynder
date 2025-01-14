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

void string_trim_end(String *str)
{
    for(int i = str->count - 1; i > 0; i--) {
        if(isspace(str->items[i])) {
            str->count--;
        } else {
            break;
        }
    }
}
