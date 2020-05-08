#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H 1

#include <curses.h>
#include <pthread.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>

#include "bigint.h"
#include "command_options.h"

void init_curses()
{
  (void) initscr();      /* initialize the curses lib */
  keypad(stdscr, TRUE);  /* enable keyboard mapping */
  (void) nonl();         /* don't do NL->CR/NL on output */
  (void) cbreak();       /* input chars one at a time, no wait for \n */
  (void) noecho();       /* don't echo input */
  (void) clear();
 
  if (has_colors())
  {
    start_color();

    init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
  }
}

const char ui_panel[] =

/*0*/ "BigInt Sort by Will Wray, mobeewave 2020\n"
/*1*/ "\n"
/*2*/ "Data filename:\n"
/*3*/ "Sort algorithm:\n"
/*4*/ "\n"
/*5*/ "(z):quit (r):read (l):list\n"
/*6*/ "(q):quicksort (m):mergesort (h):heapsort\n"
/*7*/ "\n"
/*8*/ "Command: "
/*9   01234567890123456789  */
;

static void finish(int sig)
{
    endwin();
    exit(0);
}

int bigint_wprint(WINDOW* w, bigint b)
{
  char buf[256];
  int sn = gmp_snprintf(buf,256,"%Zd",b);
  if (sn < 0)
    strcpy(buf," bigint printf error ");
  else if (sn == 255)
      /* number output too big for buf, do something */;
  waddstr(w,buf);
  return sn;
}

/** @brief UI paged list output of bigint data.
 *
 *  @param bigints Big integer 'array' (data ptr & size struct).
 */
void list_less(bigint_array bigints)
{
  int maxy = getmaxy(curscr);
  int maxx = getmaxx(curscr);
  WINDOW* cout = newwin(maxy-10,maxx,10,0);
  scrollok(cout,true);
  wmove(cout,0,0);
  int y = 0;
  [[maybe_unused]]int x = 0;

  int i = 0;
  while (i != bigints.size)
  {
    int bp = bigint_wprint(cout,bigints.data[i]);
    wrefresh(cout);

    if (bp < 0)
    {
      mvaddstr(maxy-1,0,"Press q to quit ");
      if (getch() == 'q')
        break;
    }
    getyx(cout,y,x);
    if (y == maxy-12)
    {
      mvaddstr(maxy-1,0,"Press q to quit, any key to continue ");
      if (getch() == 'q')
        break;
      else {
        wclear(cout);
      }
    }
    if (++i == bigints.size)
    {
      mvaddstr(maxy-1,0,"Press any key to exit and continue ");
      getch();
      wclear(cout);
    }
    else
      waddch(cout,',');
    wrefresh(cout);
  }
  delwin(cout);
}

/** @brief Initialize UI and enter main loop; get key & respond.
 *
 *  @param args Program arguments, parsed from commandline.
 *  @param bigints Big integer 'array' (data ptr & size struct).
 */
void ui_loop(arguments* args, bigint_array bigints)
{
  (void) signal(SIGINT, finish); /* arrange interrupts to terminate */
  init_curses();

  addstr(ui_panel);

  for (;;) {
    mvaddstr(2,15, get_filename(args));
    printw(" %d", bigints.size);
    mvaddstr(3,16, get_sort_algo(args));
    move(8,9);
    int c = getch(); /* accept single keystroke of input */
    switch (c) {     /* process the command keystroke */
      case 'q' :
      case 'm' :
      case 'h' : addch(c); set_sort_algo(args,c); continue;
      case 'l' : addch(c); list_less(bigints); continue;
      case 'z' : addch(c); break;
      default: continue;
    }
    break;
  }
  finish(0);
}

#endif