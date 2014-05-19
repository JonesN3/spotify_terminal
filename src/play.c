#include <stdio.h>
#include <libspotify/api.h>
#include "debug.h"
#include "play.h"

sp_track* current_track; 
extern int playlist_playing;
extern int playlist_index;
extern int shuffle_mode;
extern sp_playlist* g_playlist;
int playing = 0;

void play(sp_session *session, sp_track *track)
{
    debug("in player");
    sp_error error;
    g_playing = 1;
    playing = 1;
    debug("loading track into player");
	error = sp_session_player_load(session, track);
	if (error != SP_ERROR_OK) {
		fprintf(stderr, "\nError: %s\n", sp_error_message(error));
		//printf("track: %s, artist: %s\n", sp_track_name(track), sp_artist_name(sp_track_artist(track, 0)));
		//if (g_selectedList != NULL){ playlistGoNext(); printf("playing next song...\n\n");}
    }

    debug("calling sp_session_player_play");
    current_track = track;
	sp_session_player_play(session, 1);
    play_info();
}

void player_pause(sp_session *session)
{
    if(playing == 1) {
        sp_session_player_play(session, 0);
        playing = 0;
    } else if(playing == 0) {
        sp_session_player_play(session, 1);
        playing = 1;
    }
}

void play_info()
{
    sp_album *album;
    sp_artist *artist;

    album = sp_track_album(current_track);
    artist = sp_track_artist(current_track, 0);

    printf("\033[2J\033[1;1H");
    printf("Spotify_terminal\n");
    printf("Using libspotify %s\n\n", sp_build_id());

    char ESC=27;
    printf("## Playing ##\n");
    if(shuffle_mode) printf("Shuffle mode\n");
    if(playlist_playing){ 
        printf("Playlist: %s", sp_playlist_name(g_playlist));
        printf(" (%d/%d) \n\n", playlist_index+1, sp_playlist_num_tracks(g_playlist)); 
    }
    printf("%c[1m",ESC);  /*- turn on bold */
    printf("%s\n", sp_track_name(current_track));
    printf("%c[0m",ESC); /* turn off bold */
    printf("%s\n", sp_artist_name(artist));
    printf("%s\n", sp_album_name(album));
    printf("\n");
}
