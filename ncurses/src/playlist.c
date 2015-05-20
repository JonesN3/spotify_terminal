#include "includes.h"

#define TRUE 1
#define FALSE 0

//static sp_playlistcontainer_callbacks pc_callbacks;
sp_playlist *current_playlist;
int playlist_tracks, playlist_index;
extern sp_session *g_session; 
extern struct queue_entry *qu_entry;

extern int playlist_playing;
extern int shuffle_mode;
int *g_shuffle_array;
int next_timeout;

int print_playlists(sp_session *g_session, sp_playlistcontainer* pc) 
{
    printf("\n-- Print playlists --\n");
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

void print_tracks_in_playlist(sp_session *session, sp_playlist* playlist)
{
    debug("Print tracks_in_playlist");
    printf("\n--Listing songs in playlist: %s--", sp_playlist_name(playlist));
    sp_track* track;
    int i;

	for(i = 0; i < sp_playlist_num_tracks(playlist); ++i)
	{
		track = sp_playlist_track(playlist, i);
		printf("\n%d. %s", i, sp_track_name(track));
	}
}

sp_playlist* playlist_play_by_index(sp_session *session, sp_playlistcontainer *pc, int index)
{
    sp_playlist *find = NULL;
    find = sp_playlistcontainer_playlist(pc, index);

    if(!sp_playlist_is_loaded(find)){
        return NULL;
    }
    return find;
}


sp_playlist* playlist_find_by_num(sp_session *session, sp_playlistcontainer* pc)
{
    debug("find playlist by index");
    sp_playlist* find;

    char input[1024];
    int index;
    fputs("Playlist number: ", stdout);
    fgets(input, sizeof(input) - 1, stdin);

    sscanf(input, "%d", &index);

    find = sp_playlistcontainer_playlist(pc, index); 

    if(find == NULL) {
        printf("Error in loading playlist. It probably does not excist.\n");
        return NULL;
    }

    if(!sp_playlist_is_loaded(find)) {
        debug("playlist is not loaded");
        printf("playlist is not loaded\n");
        return NULL;
    } else debug("found playlist %s\n", sp_playlist_name(find));

    printf("playlist IS  loaded\n");
    return find;
}

sp_track* pl_find_song_by_id(sp_playlist* pl, int id)
{
    if(id > sp_playlist_num_tracks(pl)) {
        printf("index too high\n");
        return NULL;
    }

    sp_track* track = sp_playlist_track(pl, id);
    return track;
}

sp_playlist* playlist_find_by_name(sp_playlistcontainer *pc, char *name)
{
    int pc_length = sp_playlistcontainer_num_playlists(pc);
    if(pc_length < 0){
        return NULL;
    }

    sp_playlist *sp = NULL;
    int i;
    for(i = 0; i < pc_length; i++){
        sp = sp_playlistcontainer_playlist(pc, i);
        if(sp_playlist_is_loaded(sp)){
            /* Compare name */
            if(strcmp(name, sp_playlist_name(sp)) == 0){
                return sp;
            }
        }
    }
    return NULL;
}
