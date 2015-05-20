#include <ncurses.h>
int curses_init();
void curses_print_playlists();
void destroy_win(WINDOW *local_win);
int curses_quit();
void curses_print_playlists(sp_session *session, sp_playlistcontainer *pc);
int create_windows();
int curses_keyboard_handler();
curses_print_playinfo(sp_track* current_track);
static void SP_CALLCONV search_complete(sp_search *search, void *userdata);

#define MARGIN 2
#define BASEW 3
