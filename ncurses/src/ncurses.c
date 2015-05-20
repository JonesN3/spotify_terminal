#include "includes.h"

WINDOW *create_newwin(int height, int width, int starty, int startx);
WINDOW *main_win, *play_win, *info_win, *menu_win;

extern sp_playlistcontainer *pc;
extern int shuffle_mode;
extern q_entry *queue_entry;

int curses_init()
{

	initscr();				/* start the curses mode */

	refresh();
	create_windows();
	refresh();

	return 0;
}

void curses_print_playlists(sp_session *session, sp_playlistcontainer *pc)
{
    sp_playlist *playlist;
    int i;
	int max_x, max_y;
	int x, y;
	int width;
	int textwidth = 23;
	int entries_per_page;


	main_win_write_init(main_win, &max_x, &max_y, &entries_per_page, &width, &x, &y, textwidth);

	char format[16];
		
    for(i = 0; i < sp_playlistcontainer_num_playlists(pc); ++i) {
        switch (sp_playlistcontainer_playlist_type(pc, i)) {
            case SP_PLAYLIST_TYPE_PLAYLIST: 
                playlist = sp_playlistcontainer_playlist(pc, i);

				if(x == max_x) {
					x = 2;
					y += width;
				}

				if(y + width >= max_y) {
					textwidth = max_y - y - 5;
				}

				sprintf(format, "%%d. %%.%ds", textwidth);
				mvwprintw(main_win, x, y, format, i, sp_playlist_name(playlist));
				++x;

                break;
            default:
                break;
        }
    }

	wrefresh(main_win);
    return 0;
}

void curses_print_queue(struct play_queue *node) 
{
	int max_x, max_y;
	int x, y, i;
	int width, entries_per_page;
	int textwidth = 25;
	char format[16];

	/*
	getmaxyx(main_win, max_x, max_y);
	wclear(main_win);
	box(main_win, 0, 0);

	width = MARGIN + BASEW + textwidth;
	entries_per_page = (max_x-3) * ( (max_y / width) + 1) -1;

	y = 1;
	x = 2;
	max_x -= 1;
	max_y -= 1;
	*/
	main_win_write_init(main_win, &max_x, &max_y, &entries_per_page, &width, &x, &y, textwidth);
	i = 0;

	mvwprintw(main_win, 1, 1, "Play queue");
    struct play_queue *current = queue_entry->head;
	if(current == NULL) {
		mvwprintw(main_win, 1, 1, "Play queue is empty.");	
	}

    while(current != NULL) {
		if(x == max_x) {
			x = 2;
			y += width;
		}

		if(y + width >= max_y) {
			textwidth = max_y - y - 5;
		}

		sprintf(format, "%%d. %%.%ds", textwidth);
		mvwprintw(main_win, x, y, format, i, sp_track_name(current->track));

        current = current->next;
		x++;
		i++;
		if( i > entries_per_page) break;
    }
	wrefresh(main_win);
}

void main_win_write_init(WINDOW* w, int *max_x, int *max_y, int *epp, int *width, int *x, int *y, int textwidth)
{
	getmaxyx(w, *max_x, *max_y);
	wclear(w);
	box(main_win, 0, 0);
	*width = MARGIN + BASEW + textwidth;
	*epp = (*max_x-3) * ( (*max_y / *width) + 1) -1;
	*max_y -= 1;
	*max_x -= 1;

	*x = 1;
	*y = 1;
}

curses_print_playinfo(sp_track* current_track) {
	sp_artist *artist;
	sp_album *album;
	int max_y, max_x;
	int print_y, print_x;

	album = sp_track_album(current_track);
	artist = sp_track_artist(current_track, 0);

	getmaxyx(play_win, max_x, max_y);
	print_y = max_y/2;

	char track_name[100];
	char artist_name[100];
	char album_name[100];

	wclear(play_win);
	box(play_win, 0 , 0);		

	strcpy(track_name, sp_track_name(current_track));
	strcpy(artist_name, sp_artist_name(artist));
	strcpy(album_name, sp_album_name(album));

	mvwprintw(play_win, 1, print_y - (strlen(track_name) /2), track_name);
	mvwprintw(play_win, 2, print_y - (strlen(artist_name) /2), artist_name);
	mvwprintw(play_win, 3, print_y - (strlen(album_name) /2), album_name);

	wrefresh(play_win);
}

int curses_search(sp_session *session) 
{
	char song[50];
	char artist[50];
	char query[120];

	mvprintw(1, 0, "Song: ");	
	refresh();
	mvgetnstr(1, 6, song, 50);

	mvprintw(2, 0, "Artist: ");
	refresh();
	mvgetnstr(2, 8, artist, 50);

	mvwprintw(main_win, 1, 1, "Searching for %s, %s", song, artist);
	wrefresh(main_win);

	sprintf(query, "artist:\"%s\" track:\"%s\"", artist, song);

	sp_search_create(session, query, 0, 30, 0, 0, 0, 0, 0, 0, SP_SEARCH_STANDARD,
			&search_complete, session);
}

static void SP_CALLCONV search_complete(sp_search *search, void *userdata)
{
	char tracknum[5];
	int trackdec;
	sp_error error = sp_search_error(search);
	if (error != SP_ERROR_OK) {
		fprintf(stderr, "Error: %s\n", sp_error_message(error));
		return;
	}

	int num_tracks = sp_search_num_tracks(search);
	int i;

	wclear(main_win);
	box(main_win, 0, 0);
	for(i = 0; i < num_tracks; i++) {
		mvwprintw(main_win, i+2, 1, "%d. %s", i, sp_track_name(sp_search_track(search, i)));
	}

	mvprintw(3, 0, "Select song nr.:");
	refresh();
	wrefresh(main_win);
	mvgetnstr(3, 17, tracknum, 5);
	
	trackdec = atoi(tracknum);

	sp_track *track = sp_search_track(search, trackdec);
	
	clear_menu_win();
	mvwprintw(menu_win, 4, 0, "Added %s to head of the play queue", sp_track_name(track));
	clear_window(main_win);
	wrefresh(menu_win);
    queue_add_first(track);
}

int curses_keyboard_handler(sp_session *session, struct play_queue *node)
{
	char buffer[512];
	clrtoeol();
	mvgetnstr(0, 3, buffer, 50);

	if (strcmp(buffer, "search") == 0) {
        //player_reset();
		curses_search(session);

    } else if ((strcmp(buffer, "list") == 0) || (strcmp(buffer, "ls") == 0 )) {
		curses_print_playlists(session, pc);

	} else if(strcmp(buffer, "qshuffle") == 0) {
		queue_shuffle();

	} else if(strcmp(buffer, "clear") == 0) {
		queue_free();

    } else if (strcmp(buffer, "queueadd") == 0) {
        sp_playlist* pl = parse_play_command(session, buffer, node);

        if(pl == NULL)
			return; 

        int index;
        char input[10];
        fputs("Song number: ", stdout);
        fgets(input, sizeof(input) - 1, stdin);
        sscanf(input, "%d", &index);
        if(sp_playlist_num_tracks(pl) < index) {
            printf("index too high!\n");
            return;
        }
    
        sp_track* track = pl_find_song_by_id(pl, index);
        if(track != NULL) queue_add_first(track);

    } else if (strcmp(buffer, "list songs") == 0 ) { 
	//release all threads
        sp_playlist* pl = playlist_find_by_num(session, pc);
        print_tracks_in_playlist(session, pl);
        
    } else if (strcmp(buffer, "help") == 0) {
        print_commands();

    } else if (strcmp(buffer, "queue") == 0) {
		curses_print_queue(node);

    } else if (strcmp(buffer, "shuffle mode") == 0) {
        print_commands(); 

    }else if(strncmp(buffer, "play", strlen("play")) == 0){
        player_reset();
        sp_playlist* pl = parse_play_command(session, buffer, node);
        if(pl!=NULL) queue_add_playlist(pl);
        else { 
            printf("ERROR playlist is null\n");
            return;
        }
        queue_go_next(session);


    }else if(strncmp(buffer, "shuffle", strlen("shuffle")) == 0){
        player_reset();
        shuffle_mode = TRUE;
        sp_playlist* pl = parse_play_command(session, buffer, node);
		if(pl!=NULL) queue_add_playlist(pl);
        else { 
            printf("ERROR playlist is null\n");
            return;
        }
        queue_shuffle();
        queue_go_next(session);

    } else if(strcmp(buffer, "pause") == 0 || strcmp(buffer, "p") == 0) {
        player_pause(session);
		play_info();

    } else if (strcmp(buffer, "next") == 0 || strcmp(buffer, "n") == 0) {
        end_track(session);

    } else if (strcmp(buffer, "stop") == 0) {

    } else if (strcmp(buffer, "info") == 0) {
        //play_info();
		//box(main_win, 0, 0);
    } else if (strcmp(buffer, "quit") == 0) {
        queue_free();
		curses_quit();
        quit_program(session);
    } else {
        printf("Unkown command!\n");
    }

	clear_menu_win();
	move(1, 3);
	mvcur(50, 50, 1, 3);
	return 0;
}

int create_windows() 
{
	int row,col, starty, startx, h, w;
	getmaxyx(stdscr,row,col);	

	h = LINES - 5 - 5;
	w = COLS;
	starty = LINES - h - 5;	
	startx = 0;	

	mvprintw(0, 0, "> ");
	menu_win = newwin(5, COLS - COLS/3, 0, 0);
	main_win = create_newwin(h, w, starty, startx);
	play_win = create_newwin(5, COLS, LINES-5, 0);
	info_win = create_newwin(5, COLS/3, 0, COLS - COLS/3);

	return 0;
}

void clear_menu_win() 
{
	wclear(menu_win);
	mvwprintw(menu_win, 0, 0, "> ");
	wrefresh(menu_win);
}

void clear_window(WINDOW *w) 
{
	wclear(w);
	box(w, 0, 0);
	wrefresh(w);
}

/* creates a window with the given dimmentions and start x, y values */
WINDOW *create_newwin(int height, int width, int starty, int startx)
{
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		
	wrefresh(local_win);

	return local_win;
}

/* destoryes a window given in as parameter */
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

int curses_quit()
{
	endwin();
	return 0;
}
