#ifndef HEADER_LEX_GUI
#define HEADER_LEX_GUI

#include <allegro.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {    
    BITMAP *grid[9];
} LexSkinnedRect;

typedef struct {
    LexSkinnedRect  background[4];
    int             textcolor[4];
} LexButton;

typedef struct {
    
    BITMAP          *hGrip;
    BITMAP          *vGrip;
    BITMAP          *hSlider[3];
    BITMAP          *vSlider[3];
} LexSlider;

typedef struct {
    BITMAP *normal;
    BITMAP *checked;
    BITMAP *disabled;
    BITMAP *disabled_checked;
    int     textcolor[2];
} LexCheckbox;

typedef struct {
    BITMAP *normal;
    BITMAP *checked;
    BITMAP *disabled;
    BITMAP *disabled_checked;
    int     textcolor[2];
} LexRadiobutton;

typedef struct {
    LexSkinnedRect bg;
    int            textcolor[2];
} LexTextbox;

typedef struct {
    LexSkinnedRect bg;
    LexSkinnedRect vscroll;
    LexSkinnedRect scrollbar;
    int            textcolor[4];
} LexListbox;

typedef struct {
    LexSkinnedRect bg;
    int            textcolor[1];
    int            bgcol;
    int            titleAlign;
} LexDialog;



typedef struct {
    LexButton      button;
    LexSlider      slider;
    LexCheckbox    checkbox;
    LexRadiobutton radiobutton;
    LexTextbox     textbox;
    LexListbox     listbox;
    LexDialog      dialog;
} LexSkin;

extern LexSkin lex_skin;

/* Definition of the callback function prototypes */
typedef int (*lex_buttonCallback)(int id);
typedef char *(*getfuncptr)(int, int *);
typedef void (*lex_draw_buffer_callback)(BITMAP* buffer);


/* Loads a specific skin */
int lex_load_skin(const char* skinname);
/* Shuts down the gui */
void lex_gui_shutdown(void);

/* ----------------------------------------------------------------- */

/* The dialog procs */

/**
 The button proc. Behaves like the normal button proc, but calls a function
 on click allowing you to to react on the button click.
 Set the dp2 member to a callback function with the following prototype:
 <tt>
 int (*lex_buttonCallback)(int id);
 </tt>
 id will be set to the dialog->d1 member of the widget. The callback should return D_CLOSE 
 if the dialog should be closed as a result of the button press, or D_O_K if the dialog
 should remain open.
 If you don't need a callback, set the dp2 paramter to NULL
 */
int lex_button_proc(int msg, DIALOG *d, int c);

/** Behaves like the normal slider proc */
int lex_slider_proc(int msg, DIALOG *d, int c);
/** Behaves like the normal check proc */
int lex_check_proc(int msg, DIALOG *d, int c);
/** Behaves like the normal radio proc */
int lex_radio_proc(int msg, DIALOG *d, int c);
/** Behaves like the normal edit proc */
int lex_edit_proc(int msg, DIALOG *d, int c);
/** Behaves like the normal list proc */
int lex_list_proc(int msg, DIALOG *d, int c);

/** Allows for a movable dialog. Must be the first widget in the 
    dialog array. 
    (x,y) specify the upper left corner
    (w,h) specify the size of the dialog
    dp is a pointer to the dialog title
 */
int lex_dialog_proc(int msg, DIALOG *d, int c);


/**
 A method which runs / draws the dialog on a backbuffer bitmap.
 You need to specify a callback to blit that image to the screen.
 This function has the following prototype:
    void (*lex_draw_buffer_callback)(BITMAP* buffer);
 The buffer will be the supplied buffer bitmap.
 
 Everything else should work like the do_dialog method
 */
int lex_do_dialog(DIALOG *dialog, int focus_obj, BITMAP *buffer, void (*proc)(BITMAP*));

/**
 You need to call this function only if you switch themes while the
 dialog is already showing;
 If redraw is set to true, the dialog repaints itself
 */
void lex_init_dialog(DIALOG *dialog, int redraw);

int lex_set_screenupdate(void (*proc)(BITMAP*));

#ifdef __cplusplus
}
#endif

#endif 
