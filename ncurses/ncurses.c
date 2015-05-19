#include <ncurses.h>			/* ncurses.h includes stdio.h */  
#include <string.h> 


WINDOW *create_newwin(int height, int width, int starty, int startx);

int maincurses()
{
	WINDOW *main_win, *play_win, *info_win;

	initscr();				/* start the curses mode */
	cbreak();			/* Line buffering disabled */

	int row,col, starty, startx, h, w;
	getmaxyx(stdscr,row,col);	

	h = LINES - 5 - 5;
	w = COLS;
	starty = LINES - h - 5;	
	startx = 0;	

	refresh();

	mvprintw(0, 0, "Welcome to spotify_terminal");
	mvprintw(1, 0, "> ");
	main_win = create_newwin(h, w, starty, startx);
	play_win = create_newwin(5, COLS, LINES-5, 0);
	info_win = create_newwin(5, COLS/3, 0, COLS - COLS/3);

	refresh();

	getch(); /* wait for keypress, then exit */

	endwin();
	return 0;
}

int create_window()
{
	char mesg[]="Welcome";		/* message to be appeared on the screen */
	int row,col;				/* to store the number of rows and *
								 * the number of colums of the screen */
	getmaxyx(stdscr,row,col);		/* get the number of rows and columns */
	mvprintw(row/2,(col-strlen(mesg))/2,"%s",mesg);

	/* print the message at the center of the screen */
	mvprintw(row-2,0,"This screen has %d rows and %d columns\n",row,col);
	printw("Try resizing your window(if possible) and then run this program again");

	refresh();
	getch();

	return 0;
}


WINDOW *create_newwin(int height, int width, int starty, int startx)
{
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		
	wrefresh(local_win);

	return local_win;
}


void destroy_win(WINDOW *local_win)
{	
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners 
	 * and so an ugly remnant of window. 
	 */
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	/* The parameters taken are 
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window 
	 * 3. rs: character to be used for the right side of the window 
	 * 4. ts: character to be used for the top side of the window 
	 * 5. bs: character to be used for the bottom side of the window 
	 * 6. tl: character to be used for the top left corner of the window 
	 * 7. tr: character to be used for the top right corner of the window 
	 * 8. bl: character to be used for the bottom left corner of the window 
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(local_win);
	delwin(local_win);
}
