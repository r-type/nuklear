/* nuklear - v1.00 - public domain */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <time.h>

#include <glsym/glsym.h>
#include <libretro.h>

extern retro_input_poll_t input_poll_cb;
extern retro_input_state_t input_state_cb;

extern unsigned rwidth ,rheight;

#define MOUSE_RELATIVE 0 //0 = absolute
int gmx,gmy; // mouse
int mouse_wu=0,mouse_wd=0;
//KEYBOARD
char Key_Sate[512];
char Key_Sate2[512];
static char old_Key_Sate[512];


//#define NK_PRIVATE
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_RETRO_GL2_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_retro_gl2.h"

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/* macros */

#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])


/* Platform */
struct nk_color background;
char *win;
/* GUI */
struct nk_context *ctx;
struct nk_font_atlas *atlas;

/* ===============================================================
 *
 *                          EXAMPLE
 *
 * ===============================================================*/

/* This are the main examples to provide a small overview of what can be
 * done with this library. only one at time */
//#define NO_EXAMPLE 1
//#define EXAMPLE_FILEBROWSER 1
//#define EXAMPLE_EXTENDED 1
//#define EXAMPLE_CANVAS 1

#if defined(EXAMPLE_FILEBROWSER)
#include "./example/file_browser.c"
#elif defined(EXAMPLE_EXTENDED)
#include "./example/extended.c"
#elif defined(EXAMPLE_CANVAS)
#include "./example/canvas.c"
#endif

/* This are some code examples to provide a small overview of what can be
 * done with this library. To try out an example uncomment the include
 * and the corresponding function. */

#include "style.c"

#if !defined(EXAMPLE_CANVAS) && !defined(NO_EXAMPLE)
#include "node_editor.c"
#include "calculator.c"
#include "overview.c"
#endif

int app_init()
{
    /* GUI */
    ctx = nk_retro_init(win);

    /* Load Fonts: if none of these are loaded a default font will be used  */
    {//struct nk_font_atlas *atlas;
    nk_retro_font_stash_begin(&atlas);

#if defined(EXAMPLE_FILEBROWSER)
    filebrowser_font();
#elif defined(EXAMPLE_EXTENDED)
    extended_font();
#elif defined(EXAMPLE_CANVAS)
    canvas_font();
#endif

    nk_retro_font_stash_end();
    }

    /* style.c */
    /*set_style(ctx, THEME_WHITE);*/
    /*set_style(ctx, THEME_RED);*/
    /*set_style(ctx, THEME_BLUE);*/
    /*set_style(ctx, THEME_DARK);*/
     set_style(ctx, THEME_DARK);

    /* icons */

#if defined(EXAMPLE_FILEBROWSER)
    filebrowser_init(ctx);
#elif defined(EXAMPLE_EXTENDED)
    extended_init(ctx);
#elif defined(EXAMPLE_CANVAS)
    canvas_init(ctx);
#endif

#if !defined(EXAMPLE_CANVAS) && !defined(NO_EXAMPLE)
    background = nk_rgb(28,48,62);
#endif

//LIBRETRO   

   memset(Key_Sate,0,512);
   memset(Key_Sate2,0,512);

   memset(old_Key_Sate ,0, sizeof(old_Key_Sate));

   gmx=(rwidth/2)-1;
   gmy=(rheight/2)-1;

 return 0;
}

int app_free()
{
//FIXME: memory leak here
#if defined(EXAMPLE_FILEBROWSER)
    filebrowser_free();
#elif defined(EXAMPLE_EXTENDED)
    extended_free();
#elif defined(EXAMPLE_CANVAS)
    canvas_free();
#endif
    nk_retro_shutdown();

 return 0;
}

void retro_key(int key,int down)
{
	struct nk_context *ctx = &retro.ctx;
	if(key<512);
	else return;

        if (key == RETROK_RSHIFT || key == RETROK_LSHIFT) nk_input_key(ctx, NK_KEY_SHIFT, down);
        else if (key == RETROK_DELETE)    nk_input_key(ctx, NK_KEY_DEL, down);
        else if (key == RETROK_RETURN)    nk_input_key(ctx, NK_KEY_ENTER, down);
        else if (key == RETROK_TAB)       nk_input_key(ctx, NK_KEY_TAB, down);
        else if (key == RETROK_LEFT)      nk_input_key(ctx, NK_KEY_LEFT, down);
        else if (key == RETROK_RIGHT)     nk_input_key(ctx, NK_KEY_RIGHT, down);
        else if (key == RETROK_UP)        nk_input_key(ctx, NK_KEY_UP, down);
        else if (key == RETROK_DOWN)      nk_input_key(ctx, NK_KEY_DOWN, down);
        else if (key == RETROK_BACKSPACE) nk_input_key(ctx, NK_KEY_BACKSPACE, down);
        else if (key == RETROK_HOME)      nk_input_key(ctx, NK_KEY_TEXT_START, down);
        else if (key == RETROK_END)       nk_input_key(ctx, NK_KEY_TEXT_END, down);
        else if (key == RETROK_SPACE && !down) nk_input_char(ctx, ' ');
}


void mousebut(int but,int down,int x,int y){

	struct nk_context *ctx = &retro.ctx;

 	if(but==1)nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down);
 	else if(but==2)nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down);
 	else if(but==3)nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
	else if(but==4)nk_input_scroll(ctx,(float)down);
	//printf("but:%d %s (%d,%d)\n",but,down==1?"pressed":"released",x,y);
}

void Process_key()
{
	int i;

	for(i=0;i<320;i++)
        	Key_Sate[i]=input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0,i) ? 0x80: 0;
   
	if(memcmp( Key_Sate,old_Key_Sate , sizeof(Key_Sate) ) )
	 	for(i=0;i<320;i++)
			if(Key_Sate[i] && Key_Sate[i]!=old_Key_Sate[i]  )
        	{	
/*
				if(i==RETROK_F12){
					//play_tape();
					continue;
				}

				if(i==RETROK_RCTRL){
					//CTRLON=-CTRLON;
					printf("Modifier crtl pressed %d \n",CTRLON); 
					continue;
				}
				if(i==RETROK_RSHIFT){
					//SHITFON=-SHITFON;
					printf("Modifier shift pressed %d \n",SHIFTON); 
					continue;
				}

				if(i==RETROK_LALT){
					//KBMOD=-KBMOD;
					printf("Modifier alt pressed %d \n",KBMOD); 
					continue;
				}
//printf("press: %d \n",i);
*/
				retro_key(i,1);
	
        	}	
        	else if ( !Key_Sate[i] && Key_Sate[i]!=old_Key_Sate[i]  )
        	{
/*
				if(i==RETROK_F12){
      				//kbd_buf_feed("|tape\nrun\"\n^");
					continue;
				}

				if(i==RETROK_RCTRL){
					CTRLON=-CTRLON;
					printf("Modifier crtl released %d \n",CTRLON); 
					continue;
				}
				if(i==RETROK_RSHIFT){
					SHIFTON=-SHIFTON;
					printf("Modifier shift released %d \n",SHIFTON); 
					continue;
				}

				if(i==RETROK_LALT){
					KBMOD=-KBMOD;
					printf("Modifier alt released %d \n",KBMOD); 
					continue;
				}
//printf("release: %d \n",i);
*/
				retro_key(i,0);
	
        	}	

	memcpy(old_Key_Sate,Key_Sate , sizeof(Key_Sate) );

}

int Retro_PollEvent()
{

   input_poll_cb();

   static int mmbL=0,mmbR=0,mmbM=0;

   int mouse_l,mouse_m,mouse_r;

   int16_t mouse_x,mouse_y;
   mouse_x=mouse_y=0;

   Process_key();

   mouse_wu = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_WHEELUP);
   mouse_wd = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_WHEELDOWN);
   if(mouse_wu || mouse_wd)mousebut(4,mouse_wd?-1:1,0,0);

   mouse_l    = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT);
   mouse_r    = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT);
   mouse_m    = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_MIDDLE); 

//relative
if(MOUSE_RELATIVE){
   mouse_x = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
   mouse_y = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);

   gmx+=mouse_x;
   gmy+=mouse_y;
   if(gmx<0)gmx=0;
   if(gmx>rwidth-1)gmx=rwidth-1;
   if(gmy<0)gmy=0;
   if(gmy>rheight-1)gmy=rheight-1;

}
else{

//absolute
//FIXME FULLSCREEN no pointer
 int p_x = input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
 int p_y = input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);

 if(p_x!=0 && p_y!=0){
	int px=(int)((p_x+0x7fff)*rwidth/0xffff);
	int py=(int)((p_y+0x7fff)*rheight/0xffff);
	//printf("px:%d py:%d (%d,%d)\n",p_x,p_y,px,py);
	gmx=px;
	gmy=py;
 }

}

   if(mmbL==0 && mouse_l){

	mmbL=1;		
	mousebut(1,1,gmx,gmy);

   }
   else if(mmbL==1 && !mouse_l) {

	mmbL=0;
	mousebut(1,0,gmx,gmy);
   }

   if(mmbR==0 && mouse_r){

      	mmbR=1;
	mousebut(2,1,gmx,gmy);		
   }
   else if(mmbR==1 && !mouse_r) {

      	mmbR=0;
	mousebut(2,0,gmx,gmy);
   }

   if(mmbM==0 && mouse_m){

      	mmbM=1;
	mousebut(3,1,gmx,gmy);		
   }
   else if(mmbM==1 && !mouse_m) {

      	mmbM=0;
	mousebut(3,0,gmx,gmy);
   }

 return 0;
}


int app_event(){

	nk_input_begin(ctx);
	Retro_PollEvent();
	static int lmx=0,lmy=0;
	if(gmx!=lmx || lmy!=gmy){
		nk_input_motion(ctx, gmx, gmy);
		//printf("mx:%d my:%d \n",gmx,gmy);
	}
	lmx=gmx;lmy=gmy;
	nk_input_end(ctx);

 return 0;
}

int
app_main()
{

#if defined(EXAMPLE_FILEBROWSER)
	filebrowser(ctx);
#elif defined(EXAMPLE_EXTENDED)
	extended(ctx);
#elif defined(EXAMPLE_CANVAS)
	canvas(ctx);
#endif

        /* -------------- EXAMPLES ---------------- */
	/* uncomment here and corresponding header  */
	/* to enable demo example		    */
#if !defined(EXAMPLE_CANVAS) && !defined(NO_EXAMPLE)
	calculator(ctx);
	overview(ctx);
	node_editor(ctx);
#endif
        /* ----------------------------------------- */

        /* Draw */
        {float bg[4];
        nk_color_fv(bg, background);

        glViewport(0, 0, rwidth, rheight);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(bg[0], bg[1], bg[2], bg[3]);

        /* IMPORTANT: `nk_retro_render` modifies some global OpenGL state
         * with blending, scissor, face culling, depth test and viewport and
         * defaults everything back into a default state.
         * Make sure to either a.) save and restore or b.) reset your own state after
         * rendering the UI. */
        nk_retro_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);

	}
    


    return 0;
}

