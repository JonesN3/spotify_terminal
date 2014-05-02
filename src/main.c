#include <stdio.h>
#include <stdlib.h>

#include "string.h"
#include "debug.h"
#include "main.h"
#include "handler.h"
#include "audio.h"
#include "queue.h"

//#include "<libspotify/api.h>"
#include "/usr/local/include/libspotify/api.h"

char *username;
char *password;

static audio_fifo_t g_audiofifo;
sp_session *g_session;

extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;

/**
 * Note quite sure where to place these parts
 */

static void on_login(sp_session *session, sp_error error)
{
	if (error != SP_ERROR_OK) {
		fprintf(stderr, "Error: unable to log in: %s\n", sp_error_message(error));
		exit(1);
	}
}

static int on_music_delivered(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames)
{
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


static void on_main_thread_notified(sp_session *session)
{
	//debug("callback: on_main_thread_notified");
}


static void on_log(sp_session *session, const char *data)
{
	// this method is *very* verbose, so this data should really be written out to a log file
}

static void on_end_of_track(sp_session *session)
{
	audio_fifo_flush(&g_audiofifo);
	sp_session_player_play(session, 0);
}

static sp_session_callbacks session_callbacks = {
	.logged_in = &on_login,
	.notify_main_thread = &on_main_thread_notified,
	.music_delivery = &on_music_delivered,
	.log_message = &on_log,
	.end_of_track = &on_end_of_track
};

static sp_session_config spconfig = {
	.api_version = SPOTIFY_API_VERSION,
	.cache_location = "tmp",
	.settings_location = "tmp",
	.application_key = g_appkey,
	.application_key_size = 0, // set in main()
	.user_agent = "spot",
	.callbacks = &session_callbacks,
	NULL
};

int main(void)
{
    printf("Welcome to spotify_terminal\n");
    debug("testing debug");
    init();
    get_user_info();
    printf("User: '%s'\n", username);
    log_in();
    return 1;
}

/* initializes some needed variables */
void init(void) 
{
    username = malloc(sizeof(char)*30);
    password = malloc(sizeof(char)*30);
    
    audio_init(&g_audiofifo);
}

/* grabs username and password from user */
void get_user_info(void) 
{
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
	sp_error error;

	/* create the spotify session */
	spconfig.application_key_size = g_appkey_size;
	error = sp_session_create(&spconfig, &g_session);
	if (error != SP_ERROR_OK)
    {
		fprintf(stderr, "Error: unable to create spotify session: %s\n", sp_error_message(error));
		return -1;
	}
    
    /* Use the session to log in */
	sp_session_login(g_session, username, password, 0, NULL);
	return 1;
}
