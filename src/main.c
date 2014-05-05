#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include "string.h"
#include "debug.h"
#include "main.h"
#include "handler.h"
#include "search.h"
#include "audio.h"
#include "queue.h"
#include "playlist.h"

//#include "<libspotify/api.h>"
#include "/usr/local/include/libspotify/api.h"
void check_login();
char *username;
char *password;

static audio_fifo_t g_audiofifo;
sp_session *g_session;

extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;
extern sp_playlistcontainer* playlist_container;

/* Set when libspotify wants to process events */
static int notify_events;

/* synchronization mutex to protect shared data */
static pthread_mutex_t notify_mutex;

/* Synchronizing condition variable for the main thread  */
static pthread_cond_t notify_cond;

/* Synchronizing condition variable to disable promt temporoarily */
static pthread_cond_t promt_cond;

static sp_session_callbacks callbacks;
int next_timeout = 0;
int g_logged_in = 0;

/**
 * Note quite sure where to place these parts
 */
static void SP_CALLCONV logged_in(sp_session *session, sp_error error)
{
    debug("Callback on_login");
	if (error != SP_ERROR_OK) 
    {
		fprintf(stderr, "Error: unable to log in: %s\n", sp_error_message(error));
		exit(1);
	}
    g_logged_in = 1;
    printf("logged in");
}

static int SP_CALLCONV on_music_delivered(sp_session *session, 
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


static void SP_CALLCONV notify_main_thread(sp_session *session)
{
	debug("callback: on_main_thread_notified");
}


static void SP_CALLCONV on_log(sp_session *session, const char *data)
{
	// this method is *very* verbose, so this data should really be written out to a log file
}

static void SP_CALLCONV on_end_of_track(sp_session *session)
{
    debug("callback: on end of track");
	audio_fifo_flush(&g_audiofifo);
	sp_session_player_play(session, 0);
    notify_main_thread(g_session);
    g_playing = 0;
    g_process_running = 0;
}


int main(void)
{
    int selection;
    printf("Welcome to spotify_terminal\n");
    printf("Using libspotify %s\n", sp_build_id());
    debug("testing debug");
    init();
    get_user_info();
    printf("User: '%s'\n", username);
    log_in();
    sp_session_process_events(g_session, &next_timeout);
    
    /* *
     * the main program loop 
     * TODO: explain this shit!
     * */
    while(g_logged_in) {

        if(next_timeout == 0) {
            while(!notify_events && g_playing) {
                pthread_cond_wait(&notify_cond, &notify_mutex);
            }
        } else {
            struct timespec tv;
            clock_gettime(CLOCK_REALTIME, &tv);

            /*
            struct timeval ts;
            gettimeofday(&ts, NULL);
            TIMEVAL_TO_TIMESPEC(&ts, &tv);
            */

            tv.tv_sec += next_timeout / 1000;
            tv.tv_nsec += (next_timeout % 1000) * 1000000;
            if(tv.tv_nsec > 1000000000) {
                tv.tv_sec ++;
                tv.tv_nsec -= 1000000000;
            }
            while(!notify_events && g_playing) {
                if(pthread_cond_timedwait(&notify_cond, &notify_mutex, &tv));
                    break;
            }
        }

        /* take in process argument from user */
        if(!g_playing) {
            pthread_mutex_unlock(&notify_mutex);
            /* do things */
            handler(g_session);
            pthread_mutex_lock(&notify_mutex);
        }

        /* Process libspotify events */
        notify_events = 0;
        pthread_mutex_unlock(&notify_mutex);

        do {
            sp_session_process_events(g_session, &next_timeout);
        } while (next_timeout == 0);

        pthread_mutex_lock(&notify_mutex);
    }

    /** 
     * TODO: exit program stuff, or make we never get here
     * and do exit program stuff another place 
     */
    printf("Exiting..\n");
    return 1;
}

/* initializes some needed variables */
void init(void) 
{
    sp_session_config config;
    sp_error error;
    sp_session *session;

    username = malloc(sizeof(char)*30);
    password = malloc(sizeof(char)*30);
    audio_init(&g_audiofifo);

    memset(&config, 0, sizeof(config));
    /* Set config values */
    config.api_version = SPOTIFY_API_VERSION;
	config.cache_location = "tmp";
	config.settings_location = "tmp";
	config.application_key = g_appkey;
	config.application_key_size = g_appkey_size; 
	config.user_agent = "spot";

    /* Register callbacks */
    callbacks.logged_in = logged_in;
	callbacks.notify_main_thread = notify_main_thread;
    callbacks.music_delivery = on_music_delivered;
    callbacks.log_message = on_log;
    callbacks.end_of_track = on_end_of_track;
	config.callbacks = &callbacks;

    pthread_mutex_init(&notify_mutex, NULL);
    pthread_cond_init(&notify_cond, NULL);
    pthread_cond_init(&promt_cond, NULL);

    /* create the spotify session */
	error = sp_session_create(&config, &session);
	if (error != SP_ERROR_OK)
    {
		fprintf(stderr, "Error: unable to create spotify session: %s\n",
                sp_error_message(error));
		return;
	}
    playlist_init(session);
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
    pthread_mutex_lock(&notify_mutex);
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
