#include <libspotify/api.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "debug.h"
#include "handler.h"
#include "search.h"
#include "play.h"

static void SP_CALLCONV search_complete(sp_search *search, void *userdata);

void run_search(sp_session *session)
{
	// ask the user for an artist and track
	printf("\n--Search and play--\n");
	char artist[1024];
	printf("Artist: ");
	fgets(artist, 1024, stdin);
	artist[strlen(artist)-1] = '\0';

	char track[1024];
	printf("Track: ");
	fgets(track, 1024, stdin);
	track[strlen(track)-1] = '\0';

	// format the query, e.g. "artist:<artist> track:<track>"
	char q[4096];
	sprintf(q, "artist:\"%s\" track:\"%s\"", artist, track);

	// start the search
	sp_search_create(session, q, 0, 1, 0, 0, 0, 0, 0, 0, SP_SEARCH_STANDARD,
			&search_complete, session);
}

static void SP_CALLCONV search_complete(sp_search *search, void *userdata)
{
    debug("callback: search is complete");
	//g_menuChoice = -1;
	sp_error error = sp_search_error(search);
	if (error != SP_ERROR_OK) {
		fprintf(stderr, "Error: %s\n", sp_error_message(error));
		return;
	}

	int num_tracks = sp_search_num_tracks(search);
	if (num_tracks == 0) {
		printf("\nSorry, couldn't find that track.\n");
		return;
	}

	printf("Found track!\n");
	sp_track *track = sp_search_track(search, 0);
    queue_add_first(track);
    printf("Track added next in queue\n");
}
