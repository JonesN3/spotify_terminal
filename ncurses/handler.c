#include "includes.h"

extern int playlist_index;
extern int playlist_playing;
extern int shuffle_mode;
extern sp_playlist *g_playlist;
extern sp_playlistcontainer *pc;

void print_commands()
{
    printf("\n	Spotify_terminal, commands:\n" 
"'search'                   - Search and add first hit to top of queue\n"
"'list' | 'ls' <playlist>   - List all playlists for user\n"
"'play <playlist>'          - Start playing a playlist\n"
"'next' 'n'                 - Go to next track in the queue\n"
"'shuffle <playlist>'       - Shuffle a playlist and start playing\n"
"'qshuffle'                 - Randomly shuffle the play queue\n"
"'queue'                    - Print the play queue\n"
"'queueadd'                 - Get input for a playlist and song# and add to queue\n"
"'help'                     - Print this\n"
"\n"
"Everything you select to play will be added to a play queue\n"
"\n");
fflush(stdout);
}

void player_reset()
{
    playlist_index = 0;
    shuffle_mode = FALSE;
    playlist_playing = FALSE;
}

void set_active_playlist(sp_session *session, sp_playlist *pl, struct play_queue* node)
{
    g_playlist = pl;
}

/**
 * parse a 'play playlist' or like command.
 * Will first search for the palylist by name,
 * if this fails, search for it by index instead,
 * for a number has been parsed
 */
sp_playlist* parse_play_command(sp_session* session, char *buffer,
        struct play_queue* node)
{
    sp_playlist *playlist = NULL;

    /* split on ' ', leaving just the name or id of the playlist */
    char *tok;
    tok = strchr(buffer, ' ');

    /* If plain play command */
    if(tok == NULL) {
        playlist = playlist_find_by_num(session, pc);
        if(playlist != NULL){
            set_active_playlist(session, playlist, node);
            return playlist;
        }
        return NULL;
    } else tok = (tok +1);

    /* Check playlist name first */
    playlist = playlist_find_by_name(pc, tok);
    if(playlist != NULL){
	printf("Loading playlist %s\n", tok);
        set_active_playlist(session, playlist, node);
        return playlist;
    }

    /* Playlist name not found. Fallback on index */
    int playlist_id;
    if(sscanf( tok, "%d", &playlist_id ) == 1){
        printf("Match index!\n");
        playlist = playlist_play_by_index(session, pc, playlist_id);
        if(playlist != NULL){
            set_active_playlist(session, playlist, node);
            return playlist;
        }else {
             printf("Playlist nr.'%d' is not ready!\n", playlist_id );
            return NULL;
        }
    }
    printf("Could not find a playlist with name '%s'\n", buffer + strlen("play ") );
    return NULL;
}

/**
* Handler to check user input, and see if it matches any avaible commands.
* Will call the right methods for executing commands
*/  
void handle_keyboard(sp_session *session, struct play_queue* node) 
{
    char buffer[1024];

    fgets(buffer, sizeof(buffer), stdin);
    strtok(buffer, "\n");

    if (strcmp(buffer, "search") == 0) {
        player_reset();
       run_search(session);

    } else if ((strcmp(buffer, "list") == 0) || (strcmp(buffer, "ls") == 0 )) {
        print_playlists(session, pc);

	} else if(strcmp(buffer, "qshuffle") == 0) {
		queue_shuffle();

    } else if (strcmp(buffer, "queueadd") == 0) {
        sp_playlist* pl = parse_play_command(session, buffer, node);
        printf("done finding playlist \n");

        if(pl != NULL) printf("queueadd: %s\n", sp_playlist_name(pl));
        else{ printf("no playlist\n"); return; }

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
        queue_print(node);

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
        play_info();
    } else if (strcmp(buffer, "quit") == 0) {
        queue_free();
        quit_program(session);
    } else {
        printf("Unkown command!\n");
    }
    printf("> ");
    fflush(stdout);

    return;
}

void quit_program(sp_session *session)
{
    printf("Logging out..\n");
    sp_session_logout(session);
    free(username);
    free(password);
    printf("Goodbyte\n");
    exit(2);
}
