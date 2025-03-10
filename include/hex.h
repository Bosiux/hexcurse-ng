/******************************************************************************\
 *  Copyright (C) 2001, hexcurse is written by Jewfish and Armoth             *
 *  Copyright (C) 2020-2022 prso at github, fixes and improvements            *
 *									      *
 *  This program is free software; you can redistribute it and/or modify      *
 *  it under the terms of the GNU General Public License as published by      *
 *  the Free Software Foundation; either version 2 of the License, or	      *
 *  (at your option) any later version.					      *
 *									      *
 *  This program is distributed in the hope that it will be useful,	      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of	      *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	      *
 *  GNU General Public License for more details.			      *
 *									      *
 *  You should have received a copy of the GNU General Public License	      *
 *  along with this program; if not, write to the Free Software		      *
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *									      *
\******************************************************************************/
#include <config.h>
#include <ctype.h>				/* char types                 */
#include <errno.h>				/* errors                     */
#include <limits.h>
#include <stdio.h>				/* general includes           */
#include <stdlib.h>				/* standard library           */
#include <signal.h>				/* unix signals               */
#include <string.h>				/* string processing          */
#include <strings.h>				/* string processing          */
#include <unistd.h>				/* unix std routines          */
#if defined(HAVE_STDINT_H)
    #include <stdint.h>
#endif
#if defined(HAVE_INTTYPES_H)
    #include <inttypes.h>
#endif
#include <sys/types.h>				/* types                      */
#include "hgetopt.h"

#ifdef HAS_NCURSES
    #include <ncurses.h>
#else
    #include <curses.h> 
#endif

#if !defined(TRUE)
    #define TRUE        1
#endif
#if !defined(FALSE)
    #define FALSE       0
#endif

#if !defined(HAVE_FSEEKO)
    #define fseeko fseek
    #define fteelo ftell
#endif

/* datatypes */
typedef struct {                                /* struct that holds windows  */
    WINDOW  *hex,
            *ascii,
            *address,
            *scrollbar,
            *hex_outline,
            *ascii_outline,
            *cur_address;
} WINS;

struct LinkedList {				/* linked list structure      */
    off_t loc;
    int val;
    struct LinkedList *next;
};

struct Stack {					/* struct to be used for stack*/
    int savedVal;
    off_t currentLoc;
    struct Stack *prev;
};

/* typedefs */
typedef struct LinkedList hexList;		/* alias name to hexList      */
typedef struct Stack      hexStack;		/* alias name to hexStack     */

extern char *fpINfilename, *fpOUTfilename;      /* global file ptrs           */
extern int  MAXY;				/* external globals           */
extern WINS *windows;                           /* struct that stores windows */
extern hexList *head;				/* head for linked list       */
extern int  BASE;                               /* the base for the number    */
extern int  MIN_ADDR_LENGTH;
extern int  hex_outline_width;
extern int  hex_win_width;
extern int  ascii_outline_width;
extern int  ascii_win_width;
extern off_t maxlines;
extern off_t currentLine;
extern bool editHex;
extern bool printHex;
extern off_t LastLoc;
extern int  SIZE_CH;
extern bool USE_EBCDIC;
extern char EBCDIC[256];
extern int ASCII_to_EBCDIC[256];
extern bool color_enabled;
extern bool TERM_COLORS;
extern bool FNUMBERS;
extern int  color_level;
extern bool saved;

/* macros */
/*#define currentLoc(line, col) ((line) * BASE +((col)/3)) */
#define MAXY() ((CUR_LINES) - 3)		/* macro for maxy lines       */
#define CTRL_AND(c) ((c) & 037)			/* macro to use the ctrl key  */
						/* cursor location in the file*/
#define cursorLoc(line,col,editHex,b) (((line)*(b)) + ((col)/((editHex)?3:1)))
#define llalloc() (struct LinkedList *) calloc(1, sizeof(struct LinkedList))

#define UNUSED(x) (void)(x)

/* constants */
#define KEY_PGDN        338
#define KEY_PGUP        339
#define HVERSION	VERSION			/* the version of the source  */
#define MIN_COLS        70                      /* screen has to be 70< cols  */
#define MIN_LINES       7     /* 8 - 1 */       /* the slk crap minuses 1 line*/
#define KEY_TAB 		9			/* value for the tab key      */
#define NODEF           0  /* value for not defined characters in ASCII_to_EBCDIC */
#define SAVEPOINT      -1
#define FILEBUFF    16384			/* buffer size to copy file */
#define ALPHABET_LEN    256         /* for hexSearchBM */
#define BUF_L           8192        /* for hexSearchBM */

#define AlphabetSize (UCHAR_MAX +1)		/* for portability            */

#ifndef max
#define max(a,b) ((a) >(b) ? (a) : (b))
#endif

extern FILE *fpIN;	        		/* global file ptr           */

/* function prototypes */

/* acceptch.c */
int wacceptch(WINS *windows, off_t len);
void restoreBorder(WINS *win);
char *inputLine(WINDOW *win, int line, int col, bool allow_space);

/* file.c */
void outline(FILE *fp, off_t linenum);
off_t maxLoc(FILE *fp);
void set_saved(bool sav, WINDOW *win);
void print_usage(void);
off_t maxLines(off_t len);
int openfile(WINS *win);
int savefile(WINS *win);
off_t hexSearchBM(WINDOW *w, FILE *fp, int pat[], off_t startfp, int patlen);
off_t gotoLine(FILE *fp, off_t currLoc, off_t gotoLoc, off_t maxlines,  WINDOW *windows);
int getLocVal(off_t loc);

/* getopt.c */
int hgetopt(int argc, char *const *argv, const char *optstring);

/* hexcurse.c */
off_t parseArgs(int argc, char *argv[]);
/*void printDebug(hexList *head, int loc);*/
int getMinimumAddressLength(off_t len);
RETSIGTYPE catchSegfault(int sig);

/* llist.c */
hexList *deleteNode(hexList *head, off_t loc);
hexList *insertItem(hexList *head, off_t loc, int val);
int searchList(hexList *head, off_t loc);
off_t countList(hexList *head, off_t loc);
void updateBuf(hexList *head, char *buf, off_t pos1, off_t pos2);
int writeChanges(void);
hexList *freeList(hexList *head);

/* screen.c */
void init_menu(WINS *windows);
void free_windows(WINS *windows);
void exit_err(char *err_str);
void init_screen(void);
void screen_exit(int exit_val);
void init_fkeys(void);
RETSIGTYPE checkScreenSize(int sig);
void refreshall(WINS *win);
WINDOW *drawbox(int y, int x, int height, int width);
void scrollbar(WINS *windows, int currentLine, long maxLines);
void printHelp(WINS *win);
void winscroll(WINS *win, WINDOW *, int n, int currentLine);
void clearScreen(WINS *win);
void popupWin(char *msg, int time);
short int questionWin(char *msg);

/* stack.c */
void pushStack(hexStack **stack, off_t cl, int val);
void popStack(hexStack **stack);
void smashDaStack(hexStack **stack);

/* color.c */
void init_colors(void);
void byte_color_on(intmax_t address, char c);
void byte_color_off(intmax_t address, char c);
void address_color_on(intmax_t address);
void address_color_off(intmax_t address);
