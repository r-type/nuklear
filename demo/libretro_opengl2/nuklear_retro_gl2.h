/*
 * Nuklear - v1.00 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2016 by Micha Mettke
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NK_RETRO_GL2_H_
#define NK_RETRO_GL2_H_

NK_API struct nk_context*   nk_retro_init(char *nil);
NK_API void                 nk_retro_font_stash_begin(struct nk_font_atlas **atlas);
NK_API void                 nk_retro_font_stash_end(void);
NK_API void                 nk_retro_handle_event(int *evt);
NK_API void                 nk_retro_render(enum nk_anti_aliasing , int max_vertex_buffer, int max_element_buffer);
NK_API void                 nk_retro_shutdown(void);

#endif
/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_RETRO_GL2_IMPLEMENTATION

struct nk_retro_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture null;
    GLuint font_tex;
};

static struct nk_retro {
    char *win;
    struct nk_retro_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
} retro;

NK_INTERN void
nk_retro_device_upload_atlas(const void *image, int width, int height)
{
    struct nk_retro_device *dev = &retro.ogl;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
              GL_RGBA, GL_UNSIGNED_BYTE, image);
}

NK_API void
nk_retro_render(enum nk_anti_aliasing AA, int max_vertex_buffer, int max_element_buffer)
{
    /* setup global state */
    struct nk_retro_device *dev = &retro.ogl;

    glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_TRANSFORM_BIT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* setup viewport/project */
    glViewport(0,0,(GLsizei)rwidth,(GLsizei)rheight);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, rwidth, rheight, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    {
        GLsizei vs = sizeof(struct nk_draw_vertex);
        size_t vp = offsetof(struct nk_draw_vertex, position);
        size_t vt = offsetof(struct nk_draw_vertex, uv);
        size_t vc = offsetof(struct nk_draw_vertex, col);

        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        const nk_draw_index *offset = NULL;
        struct nk_buffer vbuf, ebuf;

        /* fill converting configuration */
        struct nk_convert_config config;
        memset(&config, 0, sizeof(config));
        config.global_alpha = 1.0f;
        config.shape_AA = AA;
        config.line_AA = AA;
        config.circle_segment_count = 22;
        config.curve_segment_count = 22;
        config.arc_segment_count = 22;
        config.null = dev->null;

        /* convert shapes into vertexes */
        nk_buffer_init_default(&vbuf);
        nk_buffer_init_default(&ebuf);
        nk_convert(&retro.ctx, &dev->cmds, &vbuf, &ebuf, &config);

        /* setup vertex buffer pointer */
        {const void *vertices = nk_buffer_memory_const(&vbuf);
        glVertexPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vp));
        glTexCoordPointer(2, GL_FLOAT, vs, (const void*)((const nk_byte*)vertices + vt));
        glColorPointer(4, GL_UNSIGNED_BYTE, vs, (const void*)((const nk_byte*)vertices + vc));}

        /* iterate over and execute each draw command */
        offset = (const nk_draw_index*)nk_buffer_memory_const(&ebuf);
        nk_draw_foreach(cmd, &retro.ctx, &dev->cmds)
        {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor(
                (GLint)(cmd->clip_rect.x),
                (GLint)((rheight - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h))),
                (GLint)(cmd->clip_rect.w),
                (GLint)(cmd->clip_rect.h));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(&retro.ctx);
        nk_buffer_free(&vbuf);
        nk_buffer_free(&ebuf);
    }

    /* default OpenGL state */
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

static void
nk_retro_clipbard_paste(nk_handle usr, struct nk_text_edit *edit)
{
/*
    const char *text = SDL_GetClipboardText();
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
    (void)usr;
*/
}

static void
nk_retro_clipbard_copy(nk_handle usr, const char *text, int len)
{
/*
    char *str = 0;
    (void)usr;
    if (!len) return;
    str = (char*)malloc((size_t)len+1);
    if (!str) return;
    memcpy(str, text, (size_t)len);
    str[len] = '\0';
    SDL_SetClipboardText(str);
    free(str);
*/
}

NK_API struct nk_context*
nk_retro_init(char *win)
{
    retro.win = win;
    nk_init_default(&retro.ctx, 0);
    retro.ctx.clip.copy = nk_retro_clipbard_copy;
    retro.ctx.clip.paste = nk_retro_clipbard_paste;
    retro.ctx.clip.userdata = nk_handle_ptr(0);
    nk_buffer_init_default(&retro.ogl.cmds);
    return &retro.ctx;
}

NK_API void
nk_retro_font_stash_begin(struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&retro.atlas);
    nk_font_atlas_begin(&retro.atlas);
    *atlas = &retro.atlas;
}

NK_API void
nk_retro_font_stash_end(void)
{
    const void *image; int w, h;
    image = nk_font_atlas_bake(&retro.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_retro_device_upload_atlas(image, w, h);
    nk_font_atlas_end(&retro.atlas, nk_handle_id((int)retro.ogl.font_tex), &retro.ogl.null);
    if (retro.atlas.default_font)
        nk_style_set_font(&retro.ctx, &retro.atlas.default_font->handle);
}

NK_API void
nk_retro_handle_event(int *evt)
{
    //struct nk_context *ctx = &retro.ctx;
#if 0
    /* optional grabbing behavior */
    if (ctx->input.mouse.grab) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
        ctx->input.mouse.grab = 0;
    } else if (ctx->input.mouse.ungrab) {
        int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
        SDL_SetRelativeMouseMode(SDL_FALSE);
        SDL_WarpMouseInWindow(retro.win, x, y);
        ctx->input.mouse.ungrab = 0;
    }

    if (evt->type == SDL_KEYUP || evt->type == SDL_KEYDOWN) {
        /* key events */
        int down = evt->type == SDL_KEYDOWN;
        const Uint8* state = SDL_GetKeyboardState(0);
        SDL_Keycode sym = evt->key.keysym.sym;
        if (sym == SDLK_RSHIFT || sym == SDLK_LSHIFT)
            nk_input_key(ctx, NK_KEY_SHIFT, down);
        else if (sym == SDLK_DELETE)
            nk_input_key(ctx, NK_KEY_DEL, down);
        else if (sym == SDLK_RETURN)
            nk_input_key(ctx, NK_KEY_ENTER, down);
        else if (sym == SDLK_TAB)
            nk_input_key(ctx, NK_KEY_TAB, down);
        else if (sym == SDLK_BACKSPACE)
            nk_input_key(ctx, NK_KEY_BACKSPACE, down);
        else if (sym == SDLK_HOME)
            nk_input_key(ctx, NK_KEY_TEXT_START, down);
        else if (sym == SDLK_END)
            nk_input_key(ctx, NK_KEY_TEXT_END, down);
        else if (sym == SDLK_z)
            nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_r)
            nk_input_key(ctx, NK_KEY_TEXT_REDO, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_c)
            nk_input_key(ctx, NK_KEY_COPY, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_v)
            nk_input_key(ctx, NK_KEY_PASTE, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_x)
            nk_input_key(ctx, NK_KEY_CUT, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_b)
            nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_e)
            nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && state[SDL_SCANCODE_LCTRL]);
        else if (sym == SDLK_LEFT) {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
            else nk_input_key(ctx, NK_KEY_LEFT, down);
        } else if (sym == SDLK_RIGHT) {
            if (state[SDL_SCANCODE_LCTRL])
                nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
            else nk_input_key(ctx, NK_KEY_RIGHT, down);
        }
    } else if (evt->type == SDL_MOUSEBUTTONDOWN || evt->type == SDL_MOUSEBUTTONUP) {
        /* mouse button */
        int down = evt->type == SDL_MOUSEBUTTONDOWN;
        const int x = evt->button.x, y = evt->button.y;
        if (evt->button.button == SDL_BUTTON_LEFT)
            nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
        if (evt->button.button == SDL_BUTTON_MIDDLE)
            nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
        if (evt->button.button == SDL_BUTTON_RIGHT)
            nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
    } else if (evt->type == SDL_MOUSEMOTION) {
        if (ctx->input.mouse.grabbed) {
            int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
            nk_input_motion(ctx, x + evt->motion.xrel, y + evt->motion.yrel);
        } else nk_input_motion(ctx, evt->motion.x, evt->motion.y);
    } else if (evt->type == SDL_TEXTINPUT) {
        nk_glyph glyph;
        memcpy(glyph, evt->text.text, NK_UTF_SIZE);
        nk_input_glyph(ctx, glyph);
    } else if (evt->type == SDL_MOUSEWHEEL) {
        nk_input_scroll(ctx,(float)evt->wheel.y);
    }
#endif
}

NK_API
void nk_retro_shutdown(void)
{
    struct nk_retro_device *dev = &retro.ogl;
    nk_font_atlas_clear(&retro.atlas);
    nk_free(&retro.ctx);
    glDeleteTextures(1, &dev->font_tex);
    nk_buffer_free(&dev->cmds);
    memset(&retro, 0, sizeof(retro));
}

#endif
