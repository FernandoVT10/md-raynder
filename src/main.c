#include <stdio.h>
#include <string.h>

#include "raylib.h"
#include "parser.h"
#include "astprinter.h"

// Colors
#define COLOR_WHITE CLITERAL(Color){221, 221, 244, 255}
#define COLOR_BLUE CLITERAL(Color){133, 170, 249, 255}
#define COLOR_BLUE_BG CLITERAL(Color){133, 170, 249, 50}
#define COLOR_TRANSPARENT CLITERAL(Color){0}

// Fonts Config
#define REGULAR_FONT_PATH "./fonts/JetBrainsMono-Regular.ttf"
#define BOLD_FONT_PATH "./fonts/JetBrainsMono-Bold.ttf"
#define ITALIC_FONT_PATH "./fonts/JetBrainsMono-Italic.ttf"
#define BOLD_ITALIC_FONT_PATH "./fonts/JetBrainsMono-BoldItalic.ttf"

#define DEFAULT_FONT_SIZE 20
#define DEFAULT_FONT_SPACING 2
#define FONT_LINE_HEIGHT 1.5

#define DEFAULT_HORIZONTAL_RULE_THICKNESS 3

// Header Sizes
const int HEADER_FONT_SIZES[6] = {
    45, // LEVEL 1
    40, // LEVEL 2
    35, // LEVEL 3
    30, // LEVEL 4
    25, // LEVEL 5
    20, // LEVEL 6
};

typedef LList RETree;

typedef enum {
    FONT_WEIGHT_REGULAR,
    FONT_WEIGHT_BOLD,
    FONT_WEIGHT_ITALIC,
    FONT_WEIGHT_BOLD_ITALIC,
} FontWeight;

typedef struct {
    char* text;
    Rectangle rect;
} RETextChunk;

typedef struct {
    LList *chunks;
    int size;
    Color color;
    FontWeight weight;
    Color bg;
} RETextNode;

typedef struct {
    Vector2 start_pos;
    Vector2 end_pos;
    float thickness;
    Color color;
} RELineNode;

typedef enum {
    RENDER_TEXT_NODE,
    RENDER_LINE_NODE,
} RenderNodeType;

typedef struct {
    int font_size;
    Color font_color;
    FontWeight font_weight;
    Color font_bg;
} REParserCtx;

typedef struct {
    REParserCtx *items;
    size_t count;
    size_t capacity;
} REParserCtxs;

typedef struct {
    Font regular;
    Font bold;
    Font italic;
    Font bold_italic;
} RenderFonts;

typedef struct {
    struct {
        Vector2 draw_pos;
        REParserCtx ctx;
        REParserCtxs ctxs;
    } parser;

    RenderFonts fonts;
    int screen_width;
} RenderState;

RenderState render_state = {
    .parser = {
        .ctx = {
            .font_color = COLOR_WHITE,
            .font_weight = FONT_WEIGHT_REGULAR,
            .font_size = DEFAULT_FONT_SIZE,
            .font_bg = COLOR_TRANSPARENT,
        },
    },
};

void print_usage_error()
{
    TraceLog(LOG_ERROR, "Usage: ./build/main <file_path>");
}

// saves the current ctx to the ctx array "ctxs"
void re_parser_save_ctx()
{
    da_append(&render_state.parser.ctxs, render_state.parser.ctx);
}

// changes ctx value back to the previously saved ctx
void re_parser_restore_ctx()
{
    int last_index = render_state.parser.ctxs.count - 1;
    render_state.parser.ctx = render_state.parser.ctxs.items[last_index];
    render_state.parser.ctxs.count--;
}

Font get_font_by_weight(FontWeight weight)
{
    switch(weight) {
        case FONT_WEIGHT_BOLD: return render_state.fonts.bold;
        case FONT_WEIGHT_ITALIC: return render_state.fonts.italic;
        case FONT_WEIGHT_BOLD_ITALIC: return render_state.fonts.bold_italic;
        case FONT_WEIGHT_REGULAR:
        default: return render_state.fonts.regular;
    }
}

char* get_next_word_from_string(String str, int *start_pos)
{
    int found_index = string_find_index(str, *start_pos, ' ');

    if(found_index == -1) {
        if(str.count - *start_pos > 0) {
            char *res = strndup(str.items + *start_pos, str.count);
            *start_pos = str.count;
            return res;
        } else {
            return NULL;
        }
    } else {
        // copy the word
        char *res = strndup(str.items + *start_pos, found_index - *start_pos);
        // we sum one to skip the space
        *start_pos = found_index + 1;

        return res;
    }
}

// TODO: refactor this beast
void create_render_text_node(LList *list, String str)
{
    RETextNode *text = allocate(sizeof(RETextNode));
    text->color = render_state.parser.ctx.font_color;
    text->size = render_state.parser.ctx.font_size;
    text->weight = render_state.parser.ctx.font_weight;
    text->bg = render_state.parser.ctx.font_bg;
    text->chunks = llist_create();

    int str_pos = 0;
    char *word;

    bool is_first_word = true;
    String chunk_str = {0};
    Rectangle chunk_rect = {
        .x = render_state.parser.draw_pos.x,
        .y = render_state.parser.draw_pos.y,
        .height = text->size,
    };
    Font font = get_font_by_weight(text->weight);

    Vector2 space_size = MeasureTextEx(font, " ", text->size, DEFAULT_FONT_SPACING);
    int space_width = space_size.x + DEFAULT_FONT_SPACING * 2;

    while((word = get_next_word_from_string(str, &str_pos)) != NULL) {
        Vector2 word_size = MeasureTextEx(font, word, text->size, DEFAULT_FONT_SPACING);

        int word_width = word_size.x;

        if(!is_first_word) {
            word_width += space_width;
        }

        int pos_x = render_state.parser.draw_pos.x;

        if(pos_x + word_width > render_state.screen_width) {
            RETextChunk *chunk = allocate(sizeof(RETextChunk));
            chunk->rect = chunk_rect;
            chunk->text = string_dump(chunk_str);
            llist_append_node(text->chunks, RENDER_TEXT_NODE, chunk);

            // clean the variables for the new chunk
            render_state.parser.draw_pos.y += text->size * FONT_LINE_HEIGHT;
            render_state.parser.draw_pos.x = 0;

            chunk_str.count = 0;
            chunk_rect = (Rectangle) {
                .x = render_state.parser.draw_pos.x,
                .y = render_state.parser.draw_pos.y,
                .height = text->size,
            };

            // remove the unnecessary space for the new line
            if(!is_first_word) {
                word_width -= space_width;
            }

            is_first_word = true;
        }

        if(!is_first_word) {
            string_append_char(&chunk_str, ' ');
        }

        string_append_str(&chunk_str, word);
        render_state.parser.draw_pos.x += word_width;
        chunk_rect.width += word_width;
        is_first_word = false;
        free(word);
    }

    RETextChunk *chunk = allocate(sizeof(RETextChunk));
    chunk->rect = chunk_rect;
    chunk->text = string_dump(chunk_str);
    llist_append_node(text->chunks, RENDER_TEXT_NODE, chunk);
    string_free(&chunk_str);

    llist_append_node(list, RENDER_TEXT_NODE, text);
}

void parse_ast_item(LList *list, ASTItem *item);

void parse_ast_list(LList *list, ASTList ast_list)
{
    ASTItem *item = ast_list.head;

    while(item != NULL) {
        parse_ast_item(list, item);
        item = item->next;
    }
}

void parse_ast_item(LList *list, ASTItem *item)
{
    switch(item->type) {
        case AST_DOCUMENT_NODE:
        case AST_PARAGRAPH_NODE: {
            ParentNode *parent = (ParentNode*)item->data;
            parse_ast_list(list, parent->children);

            render_state.parser.draw_pos.x = 0;
            render_state.parser.draw_pos.y += render_state.parser.ctx.font_size;
        } break;
        case AST_HEADER_NODE: {
            HeaderNode *h = (HeaderNode*)item->data;

            int font_size = HEADER_FONT_SIZES[h->level - 1];

            re_parser_save_ctx();
            render_state.parser.ctx.font_color = COLOR_BLUE;
            render_state.parser.ctx.font_size = font_size;
            parse_ast_list(list, h->children);
            re_parser_restore_ctx();

            render_state.parser.draw_pos.x = 0;
            render_state.parser.draw_pos.y += font_size;
        } break;
        case AST_TEXT_NODE: {
            TextNode *t = (TextNode*)item->data;
            create_render_text_node(list, t->str);
        } break;
        case AST_STRONG_NODE: {
            StrongNode *strong = (StrongNode*)item->data;

            re_parser_save_ctx();
            if(render_state.parser.ctx.font_weight == FONT_WEIGHT_ITALIC) {
                render_state.parser.ctx.font_weight = FONT_WEIGHT_BOLD_ITALIC;
            } else {
                render_state.parser.ctx.font_weight = FONT_WEIGHT_BOLD;
            }
            parse_ast_list(list, strong->children);
            re_parser_restore_ctx();
        } break;
        case AST_EMPHASIS_NODE: {
            EmphasisNode *em = (EmphasisNode*)item->data;

            re_parser_save_ctx();
            if(render_state.parser.ctx.font_weight == FONT_WEIGHT_BOLD) {
                render_state.parser.ctx.font_weight = FONT_WEIGHT_BOLD_ITALIC;
            } else {
                render_state.parser.ctx.font_weight = FONT_WEIGHT_ITALIC;
            }
            parse_ast_list(list, em->children);
            re_parser_restore_ctx();
        } break;
        case AST_HR_NODE: {
            RELineNode *line = allocate(sizeof(RELineNode));
            line->start_pos = (Vector2) {
                .x = render_state.parser.draw_pos.x,
                .y = render_state.parser.draw_pos.y,
            };
            line->end_pos = (Vector2) {
                .x = render_state.screen_width,
                .y = render_state.parser.draw_pos.y,
            };
            line->thickness = DEFAULT_HORIZONTAL_RULE_THICKNESS;
            line->color = COLOR_BLUE;
            llist_append_node(list, RENDER_LINE_NODE, line);

            render_state.parser.draw_pos.x = 0;
            render_state.parser.draw_pos.y += line->thickness;
        } break;
        case AST_CODE_SPAN_NODE: {
            CodeSpanNode *code = (CodeSpanNode*)item->data;

            re_parser_save_ctx();
            render_state.parser.ctx.font_color = COLOR_BLUE;
            render_state.parser.ctx.font_bg = COLOR_BLUE_BG;
            create_render_text_node(list, code->content);
            re_parser_restore_ctx();
        } break;
        default: break;
    }
}

LList *create_render_tree(ASTItem *doc_item)
{
    LList *list = llist_create();

    parse_ast_item(list, doc_item);

    return list;
}

void draw_text_node(RETextNode *text)
{
    LNode *node = text->chunks->head;
    while(node != NULL) {
        RETextChunk *chunk = (RETextChunk*)node->data;

        Vector2 pos = {chunk->rect.x, chunk->rect.y};

        if(!ColorIsEqual(text->bg, COLOR_TRANSPARENT)) {
            DrawRectangleRec(chunk->rect, text->bg);
        }

        DrawTextEx(
            get_font_by_weight(text->weight),
            chunk->text,
            pos,
            text->size,
            DEFAULT_FONT_SPACING,
            text->color
        );

        node = node->next;
    }
}

void draw_render_node(LNode *node)
{
    switch((RenderNodeType) node->type) {
        case RENDER_TEXT_NODE: {
            RETextNode *text = (RETextNode*)node->data;
            draw_text_node(text);
        } break;
        case RENDER_LINE_NODE: {
            RELineNode *line = (RELineNode*)node->data;
            DrawLineEx(line->start_pos, line->end_pos, line->thickness, line->color);
        } break;
    }
}

void draw_render_list(LList *tree)
{
    LNode *node = tree->head;

    while(node != NULL) {
        draw_render_node(node);

        node = node->next;
    }
}

void load_fonts()
{
    render_state.fonts.regular = LoadFontEx(REGULAR_FONT_PATH, 45, NULL, 0);
    render_state.fonts.bold = LoadFontEx(BOLD_FONT_PATH, 45, NULL, 0);
    render_state.fonts.italic = LoadFontEx(ITALIC_FONT_PATH, 45, NULL, 0);
    render_state.fonts.bold_italic = LoadFontEx(BOLD_ITALIC_FONT_PATH, 45, NULL, 0);
}

int main(int argc, char **argv)
{
    if(argc != 2) {
        print_usage_error();
        return -1;
    }

    const char *file_path = argv[1];

    ASTItem *doc_item = parse_md_file(file_path);

    if(doc_item == NULL) {
        return -1;
    }

    InitWindow(1280, 720, "Md Rayrender");

    load_fonts();
    render_state.screen_width = GetScreenWidth();

    LList *render_tree = create_render_tree(doc_item);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        draw_render_list(render_tree);

        EndDrawing();
    }

    CloseWindow();

    // print_ast(doc_item);
    ast_free_item(doc_item);
    return 0;
}
