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

void playthatlist(sp_session *session, sp_playlist* pl, struct play_queue* node)
{
    printf("\nPlay that playlist!\n");

    if(!sp_playlist_is_loaded(pl)) {
        printf("Playlist is not loaded!\n");
        return;
    }

    playlist_playing = TRUE;
    
    debug("playing playlist '%s'", sp_playlist_name(pl));

    sp_track* track = sp_playlist_track(pl, 0);
    shuffle(pl);
    printf("asdfsadf\n");
    
    playlist_play_track(session, track);
    printf("asdfsadf\n");
    queue_add_playlist(node, pl);
}

int play_song_in_playlist(sp_session *session, int playlist, int index)
{
    return -1;
}

void playqueue_go_next(sp_session *session, struct play_queue** node) 
{

    struct play_queue *current = *node;
    struct play_queue *tmp = NULL;

    //current = current->next;

    tmp = current->next;
    current->next = tmp->next;
	qu_entry->size--;
    free(tmp);

    play_queue(session, current->next);
    //playlist_play_track(session, current->next->track);
}

void playlist_go_next(sp_session *session, sp_playlist* pl, int index)
{
    if(index < 0) {
        printf("This is the first track, can't prev!\n\n");
        return;
    }
    if(index > sp_playlist_num_tracks(pl)-1) {
        printf("no more tracks in playlist, last track: %d\n\n", index);
        playlist_playing = FALSE;
        return;
    }
    if(shuffle_mode) play(session, sp_playlist_track(pl, g_shuffle_array[index]));
    else playlist_play_track(session, sp_playlist_track(pl, index)); 
}

void playlist_play_track(sp_session* session, sp_track *track)
{
    printf("playlist play track\n");
    if(sp_track_is_loaded(track)) {
        printf("This track is loaded\n");
    } else {
        printf("This track is not loaded\n");
        FILE* f = fopen("log.spotify", "a");
        fprintf(f, "## track was not ready, here i needed to load it\n");
        fclose(f);
        printf("Waiting for track to load. This is a one time thing.\n");
    }

    while(!sp_track_is_loaded(track)) {
        sp_session_process_events(session, &next_timeout);
    }
    
    play(session, track);
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

void shuffle(sp_playlist* playlist) 
{
    int size = sp_playlist_num_tracks(playlist);
    int i = 0, random, tmp;
    int ascending[size];

    g_shuffle_array = malloc(size * sizeof(int));	

    for(i = 0; i < size; ++i) {
        ascending[i] = i;
    }

    /* shuffle the numbers */
    for(i = size-1; i > 0; i--) {
        random = rand() % size;
        tmp = ascending[i];
        ascending[i] = ascending[random];
        ascending[random] = tmp;
    }

    for(i = 0; i < size; i++) {
        g_shuffle_array[i] = ascending[i];
    }
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
            printf(">%s< ?= >%s<\n", name, sp_playlist_name(sp));
            /* Compare name */
            if(strcmp(name, sp_playlist_name(sp)) == 0){
                printf("Match on %s\n", sp_playlist_name(sp));
                return sp;
            }
        }
    }
    return NULL;
}
