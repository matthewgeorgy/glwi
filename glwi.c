#include "glwi.h"
#include <glad.h>

// Global state
glwi_ctx_t      *g_ctx;

// CTX API
b32
glwi_ctx_create(glwi_ctx_t **pp_ctx,
                glwi_ctx_desc_t *p_desc)
{
    *pp_ctx = (glwi_ctx_t *)HeapAlloc(GetProcessHeap(), 0, sizeof(glwi_ctx_t));

    g_ctx = *pp_ctx;

    if (!(*pp_ctx))
        return FALSE;

    (*pp_ctx)->hinstance = GetModuleHandle(NULL);
    (*pp_ctx)->cbs.fbuffer_resize = p_desc->fbuffer_resize;
	(*pp_ctx)->cbs.mouse = p_desc->mouse;
    (*pp_ctx)->window = glwi_window_create(p_desc->width, p_desc->height, p_desc->xpos, p_desc->ypos, p_desc->title);
    (*pp_ctx)->mouse.x = 0;
    (*pp_ctx)->mouse.y = 0;
    (*pp_ctx)->mouse.last_x = 0;
    (*pp_ctx)->mouse.last_y = 0;

    return TRUE;
}

// Create just the window (WNDCLASSEX + HWND)
glwi_window_t *
glwi_window_create(u32 width,
                   u32 height,
                   u32 xpos,
                   u32 ypos,
                   const char *window_title)
{
    WNDCLASSEX      wc = {0};
    RECT            windowdim;
    glwi_window_t   *wnd;


    wnd = (glwi_window_t *)HeapAlloc(GetProcessHeap(), 0, sizeof(glwi_window_t));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &glwi_wndproc;
    wc.hInstance = g_ctx->hinstance;
    wc.lpszClassName = "glwi_internal_wc";

    if (!RegisterClassEx(&wc))
        return NULL;

    windowdim.left = 0;
    windowdim.top = 0;
    windowdim.right = width;
    windowdim.bottom = height;
    AdjustWindowRect(&windowdim, WS_OVERLAPPEDWINDOW, FALSE);

    wnd->width = windowdim.right - windowdim.left;
    wnd->height = windowdim.bottom - windowdim.top;
    wnd->title = window_title;
    wnd->b_close = FALSE;

    // Update global ctx window handle before creating window
    // Need it to be up-to-date before WM_CREATE is called
    g_ctx->window = wnd;

    wnd->hwnd = CreateWindowEx(0, wc.lpszClassName, window_title, WS_OVERLAPPEDWINDOW,
            xpos, ypos, wnd->width, wnd->height,
            NULL, NULL, g_ctx->hinstance, NULL);

    if (!wnd->hwnd)
        return NULL;

    ShowWindow(wnd->hwnd, SW_SHOW);
    UpdateWindow(wnd->hwnd);

    return wnd;
}

void
glwi_ctx_init(glwi_ctx_t *p_ctx)
{
    int                         pf;
    PIXELFORMATDESCRIPTOR       pfd = {0};


    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    p_ctx->window->dc = GetDC(p_ctx->window->hwnd);
    pf = ChoosePixelFormat(p_ctx->window->dc, &pfd);
    SetPixelFormat(p_ctx->window->dc, pf, &pfd);
    p_ctx->rc = wglCreateContext(p_ctx->window->dc);
    wglMakeCurrent(p_ctx->window->dc, p_ctx->rc);
    gladLoadGL();
    glViewport(0, 0, p_ctx->window->width, p_ctx->window->height);
    MessageBox(NULL, (LPCSTR)glGetString(GL_VERSION), "OPENGL VERSION", 0);
}

// Internally call SwapBuffers(dc) -- HDC dc from above;
void
glwi_swap_buffers(glwi_ctx_t *p_ctx)
{
    SwapBuffers(p_ctx->window->dc);
}

// Relates to WndProc?
void
glwi_poll_events(glwi_ctx_t *p_ctx)
{
    MSG     msg;
    POINT   mouse_pos;


    PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
    if (msg.message == WM_QUIT)
        p_ctx->window->b_close = TRUE;
    else
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GetCursorPos(&mouse_pos);
	ScreenToClient(p_ctx->window->hwnd, &mouse_pos);
    p_ctx->mouse.last_x = p_ctx->mouse.x; 
    p_ctx->mouse.last_y = p_ctx->mouse.y; 
	p_ctx->mouse.x = mouse_pos.x;
	p_ctx->mouse.y = mouse_pos.y;
	p_ctx->cbs.mouse(p_ctx);
}

b32
glwi_window_should_close(glwi_ctx_t *p_ctx)
{
    return p_ctx->window->b_close;
}

LRESULT CALLBACK
glwi_wndproc(HWND hwnd,
             UINT msg,
             WPARAM wparam,
             LPARAM lparam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            // Set the window handle for the global context since
            // we need it for creating the GL rc
            g_ctx->window->hwnd = hwnd;
            glwi_ctx_init(g_ctx);
        } break;

        case WM_CLOSE:
        {
            g_ctx->window->b_close = TRUE;
            PostQuitMessage(0);
            DestroyWindow(hwnd);
        } break;

        case WM_DESTROY:
        {
            g_ctx->window->b_close = TRUE;
            PostQuitMessage(0);
            DestroyWindow(hwnd);
        } break;

        case WM_SIZE:
        {
            RECT rect;
            int width, height;

            GetClientRect(hwnd, &rect);
            width = rect.right - rect.left;
            height = rect.bottom - rect.top;
            g_ctx->cbs.fbuffer_resize(width, height);
            g_ctx->window->width = width;
            g_ctx->window->height = height;
        } break;

        default:
            return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}
