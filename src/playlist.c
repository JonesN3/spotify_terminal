#include <libspotify/api.h>
#include <stdio.h>
#include <stdlib.h>
#include "playlist.h"
#include "play.h"
#include "debug.h"

sp_playlist *current_playlist;
sp_playlistcontainer *playlist_container;
int playlist_tracks, playlist_index;

int print_playlists(sp_session *g_session) 
{
	printf("\n-- Print playlists --\n");
	sp_playlistcontainer *pc = sp_session_playlistcontainer(g_session);
	sp_playlist *playlist;
	int i; //for the for loop;

	for(i = 0; i < sp_playlistcontainer_num_playlists(pc); ++i) {
		switch (sp_playlistcontainer_playlist_type(pc, i)) {
			case SP_PLAYLIST_TYPE_PLAYLIST: 
				playlist = sp_playlistcontainer_playlist(pc, i);
				printf("%d. %s\n", i, sp_playlist_name(playlist));
				break;
			default:
				break;
		}
	}
	return 0;
}

void playthatlist(sp_playlist *playlist, sp_session *session)
{
    sp_error error;

    error = sp_playlist_is_loaded(playlist);
    
    if(error != SP_ERROR_OK) {
        printf("Playlist is not loaded!");
        return;
    }

	printf("\nPlay that playlist!\n");

	if(playlist_tracks < playlist_index) {
		printf("no more tracks in playlist\n");
		return;
    }

    play(session, sp_playlist_track(playlist, 0));
}

sp_playlist* playlist_find_by_num()
{
    debug("find playlist by index");
    sp_playlist* find;
    char input[1024];
    int index;
    fputs("Playlist number: ", stdout);
    fgets(input, sizeof(input) - 1, stdin);
    printf("fgets is done\n");
    sscanf(input, "%d", &index);
    printf("sscanf is done\n");

    bool retval = sp_playlistcontainer_is_loaded(playlist_container);
    printf("playlistcontainer check\n");
    if(!retval) {
        printf("playlistcontainer not loaded!\n");
        return NULL;
    }

    printf("playlistcontainer IS loaded\n");
        
    find = sp_playlistcontainer_playlist(playlist_container, index);
}

void playlist_find_by_name(char* name)
{

}

void playlist_init(sp_session* session)
{
    playlist_container = sp_session_playlistcontainer(session);
}
