#include <stdio.h>

#include "raylib.h"
#include "parser.h"
#include "astprinter.h"

void print_usage_error()
{
    TraceLog(LOG_ERROR, "Usage: ./build/main <file_path>");
}

int main(int argc, char **argv)
{
    if(argc != 2) {
        print_usage_error();
        return -1;
    }

    const char *file_path = argv[1];

    if(!parse_file(file_path)) {
        return -1;
    }

    print_ast();
    return 0;
}
