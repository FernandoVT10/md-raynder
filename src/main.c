#include <stdio.h>
#include <string.h>

#include "raylib.h"
#include "parser.h"
#include "astprinter.h"

#define MD_BLACK CLITERAL(Color){9, 9, 17, 255}
#define MD_BLACK_LIGHT CLITERAL(Color){20, 21, 31, 255}
#define MD_WHITE CLITERAL(Color){221, 221, 244, 255}
#define MD_BLUE_BG CLITERAL(Color){133, 170, 249, 50}
#define MD_TRANSPARENT CLITERAL(Color){0}
#define MD_BLUE CLITERAL(Color){133, 170, 249, 255}

#define DEFAULT_FONT_SIZE 20
#define DEFAULT_FONT_SPACING 2
#define FONT_LINE_HEIGHT 1.5

#define REGULAR_FONT_PATH "./fonts/JetBrainsMono-Regular.ttf"
#define BOLD_FONT_PATH "./fonts/JetBrainsMono-Bold.ttf"
#define ITALIC_FONT_PATH "./fonts/JetBrainsMono-Italic.ttf"
#define BOLD_ITALIC_FONT_PATH "./fonts/JetBrainsMono-BoldItalic.ttf"

// HEADERS
#define HEADER_1_FONT_SIZE 45
#define HEADER_2_FONT_SIZE 40
#define HEADER_3_FONT_SIZE 35
#define HEADER_4_FONT_SIZE 30
#define HEADER_5_FONT_SIZE 25
#define HEADER_6_FONT_SIZE 20

#define PARAGRAPH_TOP_MARGIN 10

// HR
#define HR_THICKNESS 2
#define HR_TOP_MARGIN 10

typedef enum {
    RENDER_TEXT_NODE,
    RENDER_HR_NODE,
    RENDER_LINK_NODE,
} RenderNodeType;

typedef struct {
    RenderNodeType type;
    void *data;
} RenderNode;

typedef struct {
    RenderNode *items;
    size_t count;
    size_t capacity;
} RenderNodes;

typedef enum {
    FONT_WEIGHT_REGULAR,
    FONT_WEIGHT_BOLD,
    FONT_WEIGHT_ITALIC,
    FONT_WEIGHT_BOLD_ITALIC,
} FontWeight;

typedef struct {
    String text;
    Rectangle rect;
} RenderTextChunk;

typedef struct {
    RenderTextChunk *items;
    size_t count;
    size_t capacity;
    Color bg;
    Color color;
    int size;
    FontWeight weight;
} RenderTextNode;

typedef struct {
    Vector2 pos;
} RenderHRNode;

typedef struct {
    RenderNodes nodes;
    bool hovered;
} RenderLinkNode;

typedef struct {
    int font_size;
    Color font_color;
    FontWeight font_weight;
    Color font_bg;
} RenderCtx;

typedef struct {
    RenderCtx *items;
    size_t count;
    size_t capacity;
} RenderCtxs;

typedef struct {
    Font regular;
    Font bold;
    Font italic;
    Font bold_italic;
} RenderFonts;

typedef struct {
    RenderCtx ctx;
    RenderCtxs ctxs;
    Vector2 draw_pos;
    int screen_width;
    RenderFonts fonts;
} RenderState;

void create_render_nodes(RenderNodes *nodes, ASTList list);

void print_usage_error()
{
    TraceLog(LOG_ERROR, "Usage: ./build/main <file_path>");
}

RenderState render_state = {
    .ctx = {
        .font_size = DEFAULT_FONT_SIZE,
        .font_color = MD_WHITE,
        .font_weight = FONT_WEIGHT_REGULAR,
        .font_bg = MD_TRANSPARENT,
    },
};

void render_save_ctx()
{
    da_append(&render_state.ctxs, render_state.ctx);
}

void render_restore_ctx()
{
    render_state.ctx = render_state.ctxs.items[render_state.ctxs.count - 1];
    render_state.ctxs.count--;
}

int get_header_font_size(int level)
{
    switch(level) {
        case 1: return HEADER_1_FONT_SIZE;
        case 2: return HEADER_2_FONT_SIZE;
        case 3: return HEADER_3_FONT_SIZE;
        case 4: return HEADER_4_FONT_SIZE;
        case 5: return HEADER_5_FONT_SIZE;
        case 6: return HEADER_6_FONT_SIZE;
        default: return DEFAULT_FONT_SIZE;
    }
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

char* get_next_word(char **str_ptr)
{
    char* str = *str_ptr;
    char* found_ptr = strchr(*str_ptr, ' ');

    if(found_ptr == NULL) {
        if(strlen(str) == 0) {
            return NULL;
        }

        *str_ptr += strlen(str);
        return strdup(str);
    }

    *str_ptr = found_ptr + 1;
    return strndup(str, found_ptr - str + 1);
}

void create_render_text_node(RenderNodes *nodes, char *text)
{
    RenderTextNode *r_text = allocate(sizeof(RenderTextNode));
    r_text->bg = render_state.ctx.font_bg;
    r_text->color = render_state.ctx.font_color;
    r_text->size = render_state.ctx.font_size;
    r_text->weight = render_state.ctx.font_weight;

    RenderTextChunk chunk = {0};
    chunk.rect = (Rectangle) {
        .x = render_state.draw_pos.x,
        .y = render_state.draw_pos.y,
    };

    char *text_ptr = text;
    char *word;

    Font font = get_font_by_weight(render_state.ctx.font_weight);
    int font_size = render_state.ctx.font_size;

    while((word = get_next_word(&text_ptr)) != NULL) {
        Vector2 word_size = MeasureTextEx(font, word, font_size, DEFAULT_FONT_SPACING);

        if(render_state.draw_pos.x + word_size.x > render_state.screen_width) {
            da_append(r_text, chunk);

            render_state.draw_pos.y += word_size.y * FONT_LINE_HEIGHT;
            render_state.draw_pos.x = 0;

            chunk.rect = (Rectangle) {
                .x = render_state.draw_pos.x,
                .y = render_state.draw_pos.y,
            };
            chunk.text.count = 0;
            chunk.text.capacity = 0;
            chunk.text.items = NULL;
        }

        string_append_str(&chunk.text, word);
        free(word);
        chunk.rect.width += word_size.x + DEFAULT_FONT_SPACING;
        chunk.rect.height = word_size.y;

        render_state.draw_pos.x += word_size.x + DEFAULT_FONT_SPACING;
    }

    if(chunk.text.count > 0) {
        da_append(r_text, chunk);
    }

    da_append(nodes, ((RenderNode) {
        .type = RENDER_TEXT_NODE,
        .data = r_text,
    }));
}

void create_render_node(RenderNodes *nodes, ASTItem *item)
{
    switch(item->type) {
        case AST_DOCUMENT_NODE:
        case AST_PARAGRAPH_NODE: {
            if(render_state.draw_pos.y > 0) {
                render_state.draw_pos.y += PARAGRAPH_TOP_MARGIN;
            }

            ParentNode *parent = (ParentNode*)item->data;
            create_render_nodes(nodes, parent->children);
            render_state.draw_pos.x = 0;
            render_state.draw_pos.y += render_state.ctx.font_size;
        } break;
        case AST_HEADER_NODE: {
            HeaderNode *h = (HeaderNode*)item->data;

            int font_size = get_header_font_size(h->level);

            render_save_ctx();

            render_state.ctx.font_size = font_size;
            render_state.ctx.font_color = MD_BLUE;
            create_render_nodes(nodes, h->children);

            render_restore_ctx();

            render_state.draw_pos.x = 0;
            render_state.draw_pos.y += font_size;
        } break;
        case AST_TEXT_NODE:
            TextNode *t = (TextNode*)item->data;
            char *text = string_dump(t->str);

            create_render_text_node(nodes, text);

            free(text);
            break;
        case AST_HR_NODE: {
            render_state.draw_pos.y += HR_TOP_MARGIN;

            RenderHRNode *hr = allocate(sizeof(RenderHRNode));
            hr->pos.x = 0;
            hr->pos.y = render_state.draw_pos.y;

            da_append(nodes, ((RenderNode) {
                .type = RENDER_HR_NODE,
                .data = hr,
            }));

            render_state.draw_pos.x = 0;
            render_state.draw_pos.y += HR_THICKNESS;
        } break;
        case AST_CODE_SPAN_NODE: {
            CodeSpanNode *code = (CodeSpanNode*)item->data;

            render_save_ctx();

            render_state.ctx.font_color = MD_BLUE;
            render_state.ctx.font_bg = MD_BLUE_BG;
            char *text = string_dump(code->content);
            create_render_text_node(nodes, text);
            free(text);

            render_restore_ctx();
        } break;
        case AST_STRONG_NODE: {
            StrongNode *strong = (StrongNode*)item->data;

            render_save_ctx();
            if(render_state.ctx.font_weight == FONT_WEIGHT_ITALIC) {
                render_state.ctx.font_weight = FONT_WEIGHT_BOLD_ITALIC;
            } else {
                render_state.ctx.font_weight = FONT_WEIGHT_BOLD;
            }
            create_render_nodes(nodes, strong->children);
            render_restore_ctx();
        } break;
        case AST_EMPHASIS_NODE: {
            EmphasisNode *em = (EmphasisNode*)item->data;

            render_save_ctx();
            if(render_state.ctx.font_weight == FONT_WEIGHT_BOLD) {
                render_state.ctx.font_weight = FONT_WEIGHT_BOLD_ITALIC;
            } else {
                render_state.ctx.font_weight = FONT_WEIGHT_ITALIC;
            }
            create_render_nodes(nodes, em->children);
            render_restore_ctx();
        } break;
        default: break;
    }
}

void create_render_nodes(RenderNodes *nodes, ASTList list)
{
    ASTItem *item = list.head;

    while(item != NULL) {
        create_render_node(nodes, item);
        item = item->next;
    }
}

void load_fonts()
{
    render_state.fonts.regular = LoadFontEx(REGULAR_FONT_PATH, 20, NULL, 0);
    render_state.fonts.bold = LoadFontEx(BOLD_FONT_PATH, 20, NULL, 0);
    render_state.fonts.italic = LoadFontEx(ITALIC_FONT_PATH, 20, NULL, 0);
    render_state.fonts.bold_italic = LoadFontEx(BOLD_ITALIC_FONT_PATH, 20, NULL, 0);
}

bool is_any_node_hovered(RenderNodes nodes)
{
    Vector2 mouse_pos = GetMousePosition();

    for(size_t i = 0; i < nodes.count; i++) {
        RenderNode node = nodes.items[i];

        switch(node.type) {
            case RENDER_TEXT_NODE: {
                RenderTextNode *text = (RenderTextNode*)node.data;

                for(size_t i = 0; i < text->count; i++) {
                    RenderTextChunk chunk = text->items[i];

                    if(CheckCollisionPointRec(mouse_pos, chunk.rect)) {
                        return true;
                    }
                }
            } break;
            default: false;
        }
    }

    return false;
}

void render_nodes(RenderNodes nodes)
{
    for(size_t i = 0; i < nodes.count; i++) {
        RenderNode node = nodes.items[i];

        switch(node.type) {
            case RENDER_TEXT_NODE: {
                RenderTextNode *text = (RenderTextNode*)node.data;

                for(size_t i = 0; i < text->count; i++) {
                    RenderTextChunk chunk = text->items[i];

                    if(!ColorIsEqual(text->bg, MD_TRANSPARENT)) {
                        DrawRectangleRec(chunk.rect, text->bg);
                    }

                    char *l_text = string_dump(chunk.text);
                    Vector2 pos = {chunk.rect.x, chunk.rect.y};
                    DrawTextEx(
                        get_font_by_weight(text->weight),
                        l_text,
                        pos,
                        text->size,
                        DEFAULT_FONT_SPACING,
                        text->color
                    );

                    free(l_text);
                }
            } break;
            case RENDER_HR_NODE: {
                RenderHRNode *hr = (RenderHRNode*)node.data;
                Vector2 endPos = {render_state.screen_width, hr->pos.y};
                DrawLineEx(hr->pos, endPos, HR_THICKNESS, MD_BLUE);
            } break;
            case RENDER_LINK_NODE: {
                RenderLinkNode *link = (RenderLinkNode*)node.data;
                render_nodes(link->nodes);

                if(is_any_node_hovered(link->nodes)) {
                    if(!link->hovered) {
                        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                        link->hovered = true;
                    }
                } else if(link->hovered) {
                    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    link->hovered = false;
                }
            } break;
        }
    }
}

int main(int argc, char **argv)
{
    if(argc != 2) {
        print_usage_error();
        return -1;
    }

    const char *file_path = argv[1];

    ASTItem *md_doc = parse_md_file(file_path);

    if(md_doc == NULL) {
        return -1;
    }

    InitWindow(1280, 720, "MD Raynder");

    load_fonts();

    RenderNodes nodes = {0};
    render_state.screen_width = GetScreenWidth();

    create_render_node(&nodes, md_doc);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(MD_BLACK);

        render_nodes(nodes);

        EndDrawing();
    }

    // print_ast(md_doc);
    ast_free_item(md_doc);
    return 0;
}
