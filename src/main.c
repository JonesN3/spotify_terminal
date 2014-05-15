#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include <sys/select.h>

#include "string.h"
#include "debug.h"
#include "main.h"
#include "handler.h"
#include "search.h"
#include "audio.h"
#include "queue.h"
#include "playlist.h"

#include <libspotify/api.h>
#define TRUE 1
#define FALSE 0


void check_login();
char *username;
char *password;

static audio_fifo_t g_audiofifo;
sp_session *g_session;
sp_playlistcontainer *pc;
sp_playlist* g_playlist;

extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;
//extern sp_playlistcontainer* playlist_container;

/* Set when libspotify wants to process events */
static int notify_events;

/* synchronization mutex to protect shared data */
static pthread_mutex_t notify_mutex;

/* Synchronizing condition variable for the main thread  */
static pthread_cond_t notify_cond;

/* Synchronizing condition variable to disable promt temporoarily */
static pthread_cond_t promt_cond;

int next_timeout = 0;
int g_logged_in = 0;
int playlist_loading = 0;

int playlist_playing = 0;
int playlist_index;
int shuffle_mode = 0;

int *g_shuffle_array;

sp_playlistcontainer* playlist_container;

/**
 * About callbacks
 *
 **/

/* -- Playlist callbacks -- */
static void tracks_added(sp_playlist *pl, sp_track * const *tracks,
        int num_tracks, int position, void *userdata)
{
    printf("Callback tracks_added: %d tracks were added\n", num_tracks);
    playthatlist(g_session, pl);
    fflush(stdout);
}

static void tracks_removed(sp_playlist *pl, const int *tracks,
        int num_tracks, void *userdata)
{
    debug("Callback: tracks removed");
}

static void tracks_moved(sp_playlist *pl, const int *tracks,
        int num_tracks, int new_position, void *userdata)
{
    debug("Callback: tracks moved");
}

static void playlist_renamed(sp_playlist *pl, void *userdata)
{
    debug("on playlist renamed");
}

/* Register playlist callbacks */
static sp_playlist_callbacks pl_callbacks = {
    .tracks_added = &tracks_added,
    .tracks_removed = &tracks_removed,
    .tracks_moved = &tracks_moved,
    .playlist_renamed = &playlist_renamed,
};

/* -- Playlistcontainer callbacks -- */
static void playlist_added(sp_playlistcontainer *pc, sp_playlist *pl,
        int position, void *userdata)
{
    debug("Callback: playlist_added");
}

static void playlist_removed(sp_playlistcontainer *pc, sp_playlist *pl,
        int position, void *userdata)
{
    debug("Callback: playlist_removed");
}

static void container_loaded(sp_playlistcontainer *pc, void *userdata)
{
    debug("Callback: playlistcontainer_loaded");
    fprintf(stderr, "jukebox: Rootlist synchronized (%d playlists)\n",
            sp_playlistcontainer_num_playlists(pc));
}

/* Register playlistcontainer callbacks */
static sp_playlistcontainer_callbacks pc_callbacks = {
    .playlist_added = &playlist_added,
    .playlist_removed = &playlist_removed,
    .container_loaded = &container_loaded,
};

/* -- Session callbacks -- */
static void logged_in(sp_session *session, sp_error error)
{
    printf("callback login\n");
    if (error != SP_ERROR_OK) 
    {
        fprintf(stderr, "Error: unable to log in: %s\n", sp_error_message(error));
        exit(1);
    }

    pc = sp_session_playlistcontainer(session);

    sp_playlistcontainer_add_callbacks( pc, &pc_callbacks, NULL);

    g_logged_in = 1;
    printf("logged in\n");
}

static int music_delivered(sp_session *session, 
        const sp_audioformat *format, const void *frames, int num_frames)
{
    //debug("Callback on_music_deliverd");
    audio_fifo_t *af = &g_audiofifo;
    audio_fifo_data_t *afd;
    size_t s;

    if (num_frames == 0)
        return 0; // Audio discontinuity, do nothing

    pthread_mutex_lock(&af->mutex);

    /* Buffer one second of audio */
    if (af->qlen > format->sample_rate) {
        pthread_mutex_unlock(&af->mutex);

        return 0;
    }

    s = num_frames * sizeof(int16_t) * format->channels;

    afd = malloc(sizeof(*afd) + s);
    memcpy(afd->samples, frames, s);

    afd->nsamples = num_frames;

    afd->rate = format->sample_rate;
    afd->channels = format->channels;

    TAILQ_INSERT_TAIL(&af->q, afd, link);
    af->qlen += num_frames;

    pthread_cond_signal(&af->cond);
    pthread_mutex_unlock(&af->mutex);

    return num_frames;
}


static void notify_main_thread(sp_session *session)
{
    FILE *file;
    int file_exsists = 0;

    file = fopen("log.spotify", "a");
    fprintf(file, "%s", data);
    fclose(file);

    debug("log callback: >%s\n", data);
}


static void on_log(sp_session *session, const char *data)
{
    // this method is *very* verbose, so this data should really be written out to a log file
}

static void on_end_of_track(sp_session *session)
{
    debug("callback: on end of track");
    audio_fifo_flush(&g_audiofifo);
    sp_session_player_play(session, 0);
    notify_main_thread(g_session);

    if(playlist_playing) playlist_go_next(g_session, g_playlist, ++playlist_index);
    g_playing = 0;
    g_process_running = 0;
}

static void message_to_user(sp_session *s, const char* msg)
{
    printf("message to user: >%s<\n", msg);
}

static void connection_error(sp_session *s, sp_error error)
{
    printf("callback connection_error\n");
    if(error != SP_ERROR_OK) printf("connection error\n");
}

/* Register session callbacks */
static sp_session_callbacks session_callbacks = {
    .logged_in = &logged_in,
    .notify_main_thread = &notify_main_thread,
    .music_delivery = &music_delivered,
    .message_to_user = &message_to_user,
    .connection_error = &connection_error,
    .log_message = &on_log,
    .end_of_track = &on_end_of_track,
};

/* Register session config */
static sp_session_config session_config = {
    .api_version = SPOTIFY_API_VERSION,
    .cache_location = "tmp",
    .settings_location = "tmp",
    .application_key = g_appkey,
    .application_key_size = 0, //set in main? g_appkey_size, 
    .user_agent = "spot",
    .callbacks = &session_callbacks,
    NULL,
};

/* initializes some needed variables */
void init(void) 
{
    FILE *f = fopen("log.spotify", "wb");

    sp_error error;
    sp_session *session;

    username = malloc(sizeof(char)*30);
    password = malloc(sizeof(char)*30);
    audio_init(&g_audiofifo);

    session_config.application_key_size = g_appkey_size;

    pthread_mutex_init(&notify_mutex, NULL);
    pthread_cond_init(&notify_cond, NULL);
    pthread_cond_init(&promt_cond, NULL);

    /* create the spotify session */
    error = sp_session_create(&session_config, &session);
    if (error != SP_ERROR_OK)
    {
        fprintf(stderr, "Error: unable to create spotify session: %s\n",
                sp_error_message(error));
        return;
    }
    g_session = session;
}

/* grabs username and password from user */
void get_user_info(void) 
{
    debug("get user info");
    char *username_in = malloc(sizeof(char)*30);
    char *password_in = malloc(sizeof(char)*30);

    fputs("username: ", stdout);
    fgets(username_in, 30, stdin);
    fputs("password: ", stdout);
    fgets(password_in, 30, stdin);

    username_in = strtok(username_in, "\n");
    password_in = strtok(password_in, "\n");

    strcpy(username, username_in);
    strcpy(password, password_in);
    free(username_in);
    free(password_in);
}

/**
 * creates a spotify session, and logs in with it
 */
int log_in(void)
{
    /* Use the session to log in */
    sp_session_login(g_session, username, password, 0, NULL);
    //pthread_mutex_lock(&notify_mutex);
    return 1;
}

void check_login()
{
    debug("check login");
    sp_connectionstate s = sp_session_connectionstate(g_session);
    if(s == 0) {
        printf("Could not log in, try again\n");
        get_user_info();
        log_in();
    }
    printf("%d\n", s);
}

void check_playlist_status(sp_playlist *playlist)
{
    printf("Playlist needs to load. Will cache on HD, so you won't need to wait this long again\n");
    printf("waiting for playlist %s\n", sp_playlist_name(playlist));
    printf("number of tracks %d\n", sp_playlist_num_tracks(playlist));
}

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

void set_active_playlist(sp_playlist *pl)
{
    g_playlist = pl;
    playthatlist(g_session, g_playlist);
}

void parse_play_command(char *buffer)
{
    sp_playlist *playlist = NULL;

    /* If plain play command */
    if(strlen(buffer) <= strlen("play ")){
        playlist = playlist_find_by_num(g_session, pc);
        if(playlist != NULL){
            set_active_playlist(playlist);
            return;
        }
    }

    char playlist_name_buffer[1024];
    /* Check playlist name first */
    if(sscanf(buffer, "play %s", playlist_name_buffer) == 1){
        playlist = playlist_find_by_name(pc, buffer + strlen("play "));
        if(playlist != NULL){
            set_active_playlist(playlist);
            return;
        }
    }

    /* Playlist name not found. Fallback on index */
    int playlist_id;
    if(sscanf( buffer, "play %d", &playlist_id ) == 1){
        printf("Match index!\n");
       playlist = playlist_play_by_index(g_session, pc, playlist_id);
        if(playlist != NULL){
            set_active_playlist(playlist);
            return;
        }else {
             printf("Could not find a playlist with id '%d'\n", playlist_id );
            return;
        }
    }

    printf("Could not find a playlist with name '%s'\n", buffer + strlen("play ") );
}
 
void handle_keyboard() 
{
    char buffer[1024];
    char in_buffer[1024];
    char* retval;


    retval = fgets(buffer, sizeof(buffer), stdin);
    debug("buffer is %s\n", buffer);
    strtok(buffer, "\n");
    printf("\r\r");
    notify_events = 1;

    if (strcmp(buffer, "search") == 0) {
        player_reset();
       run_search(g_session);

    } else if ((strcmp(buffer, "list") == 0) || (strcmp(buffer, "ls") == 0 )) {
        print_playlists(g_session, pc);

    } else if (strcmp(buffer, "list songs") == 0 ) { 
        sp_playlist* pl = playlist_find_by_num(g_session, pc);
        print_tracks_in_playlist(g_session, pl);
        
    } else if (strcmp(buffer, "help") == 0) {
        print_commands();

    } else if (strcmp(buffer, "play playlist") == 0 || strcmp(buffer, "play") == 0 ) {
        player_reset();
        sp_playlist* pl = playlist_find_by_num(g_session, pc);
        g_playlist = pl;
        playthatlist(g_session, g_playlist);
    } else if (strcmp(buffer, "shuffle play") == 0 || strcmp(buffer, "shuffle") == 0) {
        player_reset();
        shuffle_mode = 1;
        sp_playlist* pl = playlist_find_by_num(g_session, pc);
        g_playlist = pl;
        playthatlist(g_session, g_playlist);

    } else if (strcmp(buffer, "shuffle mode") == 0) {
  
    }else if(strncmp(buffer, "play", strlen("play")) == 0){
        parse_play_command(buffer);
    
    } else if (strcmp(buffer, "next") == 0 || strcmp(buffer, "n") == 0) {
        if(!playlist_playing) {
            printf("There is no playlist playing!\n\n");
            return;
        }
        on_end_of_track(g_session);
        //playlist_go_next(g_session, g_playlist, ++playlist_index);
    } else if (strcmp(buffer, "stop") == 0) {

    } else if (strcmp(buffer, "info") == 0) {
        play_info();
    } else if (strcmp(buffer, "exit") == 0) {
            shutdown();        
    } else {
        printf("\rUnkown command!");
    }
}

void shutdown()
{

}

int main(void)
{
    int selection;
    int select_ret;
    fd_set  read_set;

    struct timeval tv;

    printf("Welcome to spotify_terminal\n");
    printf("Using libspotify %s\n", sp_build_id());
    debug("testing debug");
    init();
    get_user_info();
    printf("User: '%s'\n", username);
    log_in();
    sp_session_process_events(g_session, &next_timeout);
    printf("ready\n");

    /* i will make a select loop */
    while(1) {
        //fd_set = read_set;
        FD_ZERO( &read_set );
        FD_SET(STDIN_FILENO, &read_set); /* keyboard input */

        tv.tv_sec = 0;
        tv.tv_usec = 1000000;
        select_ret = select ( 1, &read_set, 0, 0, &tv );

        switch(select_ret) 
        {
            case -1 :
                printf("error in select");
                break;
            
            case 0 :
                /* nothing has happend on the keyboard, timeout expired? */
                /*debug("case 0 %d ", notify_events);
                if(next_timeout == 0) {
                    while(!notify_events && g_playing) {
                        debug("while(!notify_events && g_playing");
                        pthread_cond_wait(&notify_cond, &notify_mutex);
                    }
                } else {
                    struct timespec tv;
                    clock_gettime(CLOCK_REALTIME, &tv);

                    tv.tv_sec += next_timeout / 1000;
                    tv.tv_nsec += (next_timeout % 1000) * 1000000;
                    if(tv.tv_nsec > 1000000000) {
                        tv.tv_sec ++;
                        tv.tv_nsec -= 1000000000;
                    }
                    while(!notify_events && g_playing) {
                        debug("while(!notify_events && g_playing2");
                        printf("in while 2\n");
                        if(pthread_cond_timedwait(&notify_cond, &notify_mutex, &tv));
                        break;
                        printf("while2\n");
                    }
                    debug("after while 2\n");
                }*/

                /*
                if(!g_playing) {
                    pthread_mutex_unlock(&notify_mutex);
                    //handler(g_session, pc);
                    pthread_mutex_lock(&notify_mutex);
                }*/


                /* Process libspotify events */
                notify_events = 0;
                pthread_mutex_unlock(&notify_mutex);

                do {
                    debug("do, while(next_timeout == 0)");
                    sp_session_process_events(g_session, &next_timeout);
                    if( playlist_loading )check_playlist_status(g_playlist);
                } while (next_timeout == 0);

                pthread_mutex_lock(&notify_mutex);
                if( playlist_loading )check_playlist_status(g_playlist);

            default :
                debug("default");
                if( FD_ISSET (STDIN_FILENO, &read_set)) handle_keyboard(); 

        }

    }

    /* *
     * the main program loop 
     * TODO: explain this shit!
     * */
    while(g_logged_in) {
        debug("main loop start");

    }
    /** 
     * TODO: exit program stuff, or make we never get here
     * and do exit program stuff another place 
     */
    sp_session_process_events(g_session, &next_timeout);
    sp_session_process_events(g_session, &next_timeout);
    sp_session_process_events(g_session, &next_timeout);
    sp_session_process_events(g_session, &next_timeout);
    sp_session_process_events(g_session, &next_timeout);     
    printf("Exiting..\n");
    return 1;
}


