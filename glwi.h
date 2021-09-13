#ifndef GLWI_H
#define GLWI_H

/*
    Version History

        0.6.2   Style cleanup; windowdim fields update in WM_SIZE
        0.6.1   Added framebuffer resize callback
        0.6     Refactored to a context driven API
        0.5     Width and height params set the client area, not window area
        0.4     Removed hidden data; added primitive mouse/keyboard/callback/ctx structures
        0.3     Added window_should_close functionaltiy
        0.2     Bug fixes; currently working
        0.1     First write
*/

// TODO: Add keyboard support
// TODO: Add mouse support
// TODO: Add callback fn support
// TODO: Seperate public and internal facing APIs
// TODO: Add more NULL/error checking

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mho.h>

#ifdef __cplusplus
    #define GLWI_API    extern "C"
#else
    #define GLWI_API    extern
#endif

// Structures
typedef struct _tag_glwi_window
{
    u32         width,
                height;
    const char  *title;
    HWND        hwnd;
    HDC         dc;
    b32         b_close;
} glwi_window_t;

typedef struct _tag_glwi_callbacks
{
    void (*fbuffer_resize)(int, int);
} glwi_callbacks_t;

typedef struct _tag_glwi_ctx
{
    glwi_window_t   *window;
    HGLRC           rc;
    HINSTANCE       hinstance;
    glwi_callbacks_t cbs;
} glwi_ctx_t;

typedef struct _tag_glwi_ctx_desc
{
    u32         width, height;
    u32         xpos, ypos;
    const char  *title;
    void        (*fbuffer_resize)(int, int);
} glwi_ctx_desc_t;

// Functions
GLWI_API b32            glwi_ctx_create(glwi_ctx_t **pp_ctx, glwi_ctx_desc_t *p_desc);
GLWI_API b32            glwi_ctx_destroy(glwi_ctx_t **pp_ctx);
GLWI_API void           glwi_ctx_init(glwi_ctx_t *p_ctx);
GLWI_API glwi_window_t  *glwi_window_create(u32 width, u32 height, u32 xpos, u32 ypos, const char *window_title);
GLWI_API void           glwi_swap_buffers(glwi_ctx_t *p_ctx);
GLWI_API void           glwi_poll_events(glwi_ctx_t *p_ctx);
GLWI_API b32            glwi_window_should_close(glwi_ctx_t *p_ctx);
GLWI_API                LRESULT CALLBACK glwi_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#endif // GLWI_H
