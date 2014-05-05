#include <libspotify/api.h>
#include <stdio.h>
#include "main.h"
#include "handler.h"
#include "search.h"
#include "playlist.h"

int g_menuChoice;
int g_playing;
int g_process_running;
int next_timeout2;

int handler_menu(sp_session *session)
{
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
