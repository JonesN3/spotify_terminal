#include <libspotify/api.h>
#include <stdio.h>
#include "main.h"
#include "handler.h"
#include "search.h"
#include "playlist.h"

int g_menuChoice;
int g_playing;
int g_process_running;
int next_timeout2 = 0;

int handler(sp_session *session)
{
    printf("\nHandler \n");
    int index;
    int playlist_num;
    char input[256];
    int selection = 0;
    selection = handler_menu(session);

    if(selection == 1)
    { 
        run_search(session);
        g_playing = 1;
    }else if(selection == 2) {
        printf("playlist play test\n");
        //testPlaylistPlay(session, 0);
    } else if(selection == 3) {
        //printPlaylists(g_session);
    } else if(selection == 4) {
        //sp_playlist* pl = playlist_find_by_num();
        //listSongsInPlaylist(session, pl);
    } else if(selection == 5) {
        sp_playlist* pl = playlist_find_by_num();
        playthatlist(pl, session);
    } else if(selection == 6) {
        printf("Shuffle and play");
        //g_shuffleMode2 = 1;
        //playthatlist();
    } else {
        printf("\nerror: illegal menu choice\n");
        g_menuChoice = 9;
    }
    g_menuChoice = -1;
    return selection;
}


int handler_menu(sp_session *session)
{
    sp_session_process_events(session, &next_timeout2);
    int selected;
    if(g_menuChoice == 9)
    {
        printf("\n\n--Menu--\n");
        printf("0: exit\n");
        printf("1: search\n");
        printf("2: test playlist-play \n");
        printf("3: list playlists\n");
        printf("4: list songs in playlist [n]\n");
        printf("5: play playlist [n]\n");
        printf("6: shuffle and play\n");
        printf("9: help\n");
    }


    sp_session_process_events(session, &next_timeout2);
    char input[100];
    fputs("> ", stdout);
    fgets(input, sizeof(input) - 1, stdin);
    sscanf(input, "%d", &selected);
    printf("selected: %d\n", selected);

    return selected;
}

/**
 * Quits the program.
 */

void quit(void)
{
    /* TODO call all the nececasry function for quiting propperly.
     * free memory
     */
}
