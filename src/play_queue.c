#include "includes.h"
#include <time.h>
int id_tracker;
extern struct queue_entry *qu_entry;

void queue_add(struct play_queue *node, sp_track *track)
{
    struct play_queue *current = node;

    while(current->next != NULL) {
       current = current->next; 
   }
    current->next = malloc(sizeof(struct play_queue));

    current = current->next;
    current->track = track;
    current->id = id_tracker;
    id_tracker++;
    current->next = NULL;
	qu_entry->size++;
}
 
void queue_remove(int id)
{

}

void queue_shuffle(struct play_queue *node)
{
	printf("queue size: %d \n", qu_entry->size);
	struct play_queue *shuffle_array[qu_entry->size-1];
	int cnt = 0;

	struct play_queue *current = node;
	current = current->next;
	while(cnt < qu_entry->size) {
		printf("current track: %s", sp_track_name(current->track));
		printf(" %d\n", cnt);
		shuffle_array[cnt] = current;
		current = current->next;
		cnt++;
	}

	int i = qu_entry->size-1;
	struct play_queue *tmp;
	for(i = qu_entry->size-1; i > 0; i--) {
		int rand = rand_lim(i);
		printf("random: %d, ", rand);
		tmp = shuffle_array[rand];
		shuffle_array[rand] = shuffle_array[i];
		shuffle_array[i] = tmp;
	}
	printf("\n");
	
	cnt = 0;
	current = node;
	current = shuffle_array[0];
	while(cnt < qu_entry->size-1) {
		printf("current track: %s cnt: %d\n", sp_track_name(current->track), cnt);
		cnt++;
		current->next = shuffle_array[cnt];
		current = current->next;
	}
	current->next = NULL;
}

/* correct way to get random number withing a limit */
int rand_lim(int limit) {
/* return a random number between 0 and limit inclusive.
 */

	srand(time(NULL));	
    int divisor = RAND_MAX/(limit+1);
    int retval;

    do { 
        retval = rand() / divisor;
    } while (retval > limit);

    return retval;
}

void queue_add_first(struct play_queue *node, sp_track *track)
{
    printf("adding '%s' to top of queue\n", sp_track_name(track));
    struct play_queue *current = node;
    struct play_queue *tmp = NULL;
    
    current = current->next;
    tmp = malloc(sizeof(struct play_queue));
    tmp->track = track;
    tmp->id = id_tracker;
    id_tracker++;
    tmp->next = current->next;
    current->next = tmp;
	qu_entry->size++;
}

void queue_add_playlist(struct play_queue *node, sp_playlist *playlist)
{
    int i = 0;
    
    for(i = 0; i < sp_playlist_num_tracks(playlist) - 1; i++) {
        queue_add(node, sp_playlist_track(playlist, i));
    }
}

void queue_print(struct play_queue *node) 
{
    printf("printing queue\n");
	printf("queue size: %d\n", qu_entry->size);
    struct play_queue *current = node;
    current = current->next;
    sp_track *cur_track;

	int i = 0;
    while(current != NULL) {
        printf("%d: %s\n", i++, sp_track_name(current->track));
        current = current->next;
    }
    printf("done!\n");
}

void queue_free(struct play_queue *node)
{

}
