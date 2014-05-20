#include "includes.h"
#include "playlist.h"
#include "main.h"

extern int playlist_index;
extern int playlist_playing;
extern int shuffle_mode;
extern sp_playlist *g_playlist;
extern sp_playlistcontainer *pc;

void print_commands()
{
    printf("Spotify_terminal, commands:\n" 
            "'search'        - Search by artist and/or song\n"
            "'list' 'ls'     - List playlist for user\n"
            "'play'          - Select and play a playlist (by number)\n"
            "'next' 'n'      - Go to next track in playlist\n"
            "'help'          - Print this\n"
            "\n");
}

void player_reset()
{
    playlist_index = 0;
    shuffle_mode = FALSE;
    playlist_playing = FALSE;
}

void set_active_playlist(sp_session *session, sp_playlist *pl)
{
    g_playlist = pl;
    playthatlist(session, g_playlist);
}

/**
 * parse a 'play playlist' or like command.
 * Will first search for the palylist by name,
 * if this fails, search for it by index instead,
 * for a number has been parsed
 */
void parse_play_command(sp_session* session, char *buffer)
{
    printf("parse play command");
    sp_playlist *playlist = NULL;

    /* split on ' ', leaving just the name or id of the playlist */
    char *tok;
    tok = strchr(buffer, ' ');

    /* If plain play command */
    if(tok == NULL) {
        playlist = playlist_find_by_num(session, pc);
        if(playlist != NULL){
            set_active_playlist(session, playlist);
            return;
        }
        return;
    } else tok = (tok +1);

    /* Check playlist name first */
    playlist = playlist_find_by_name(pc, tok);
    if(playlist != NULL){
        set_active_playlist(session, playlist);
        return;
    }

    /* Playlist name not found. Fallback on index */
    int playlist_id;
    if(sscanf( tok, "%d", &playlist_id ) == 1){
        printf("Match index!\n");
        playlist = playlist_play_by_index(session, pc, playlist_id);
        if(playlist != NULL){
            set_active_playlist(session, playlist);
            return;
        }else {
             printf("Playlist nr.'%d' is not ready!\n", playlist_id );
            return;
        }
    }
    printf("Could not find a playlist with name '%s'\n", buffer + strlen("play ") );
    return;
}
 
void handle_keyboard(sp_session *session) 
{
    char buffer[1024];

    fgets(buffer, sizeof(buffer), stdin);
    strtok(buffer, "\n");

    if (strcmp(buffer, "search") == 0) {
        player_reset();
       run_search(session);

    } else if ((strcmp(buffer, "list") == 0) || (strcmp(buffer, "ls") == 0 )) {
        print_playlists(session, pc);

    } else if (strcmp(buffer, "list songs") == 0 ) { 
        sp_playlist* pl = playlist_find_by_num(session, pc);
        print_tracks_in_playlist(session, pl);
        
    } else if (strcmp(buffer, "help") == 0) {
        print_commands();

    } else if (strcmp(buffer, "shuffle mode") == 0) {
        print_commands(); 

    }else if(strncmp(buffer, "play", strlen("play")) == 0){
        player_reset();
        parse_play_command(session, buffer);

    }else if(strncmp(buffer, "shuffle", strlen("shuffle")) == 0){
        player_reset();
        shuffle_mode = TRUE;
        parse_play_command(session, buffer);

    } else if(strcmp(buffer, "pause") == 0 || strcmp(buffer, "p") == 0) {
        player_pause(session);

    } else if (strcmp(buffer, "next") == 0 || strcmp(buffer, "n") == 0) {
        if(!playlist_playing) {
            printf("There is no playlist playing!\n\n");
            return;
        }
        end_track(session);

    } else if (strcmp(buffer, "stop") == 0) {

    } else if (strcmp(buffer, "info") == 0) {
        play_info();
    } else if (strcmp(buffer, "quit") == 0) {
        quit_program(session);
    } else {
        printf("Unkown command!");
    }
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
