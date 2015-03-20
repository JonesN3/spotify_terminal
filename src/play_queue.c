#include "includes.h"
#include <time.h>

/* global variable for the entry of the queue */
extern q_entry *queue_entry;

/*
 * Print some useful info about the queue
 */
void queue_info()
{
    printf("--queue_info--\n");
    printf("queue_head: %s, queue_tail: %s\n", 
            sp_track_name(queue_entry->head->track),
            sp_track_name(queue_entry->tail->track));
    printf("size: %d\n", queue_entry->size);
}

/*
 * Add track to the back (tail) of the queue
 */
void queue_add(sp_track *track)
{
    struct play_queue *current;

    if(queue_entry->head == NULL) {
        queue_entry->head = malloc(sizeof(struct play_queue));
        queue_entry->tail = malloc(sizeof(struct play_queue));
        current = queue_entry->head;
    } else {
        current = queue_entry->tail;
        current->next = malloc(sizeof(struct play_queue));
        current = current->next;
    }

    current->track = track;
    current->id = 99;
	queue_entry->size++;
    queue_entry->tail = current;
    current->next = NULL;
}

/* 
 * Add track to the start (head) of the queue
 */
void queue_add_first(sp_track *track)
{
    struct play_queue *current;
    struct play_queue *tmp;

    tmp = malloc(sizeof(struct play_queue));
    tmp->track = track;
    tmp->id = -1; /* remove this */
    
    if(queue_entry->head == NULL) {
        queue_entry->head = tmp;
        queue_entry->tail = tmp;
        tmp->next = NULL;
    } else {
        current = queue_entry->head;
        tmp->next = current->next;
        current->next = tmp;
    }
	queue_entry->size++;
}
 
void queue_remove(int id)
{

}

/*
 * Randomly shuffle the entire play queue
 *
 * Moves the entrie queue into an array of play_queue pointers.
 * Shuffles the array, and relink the pointers from the array
 * as the play_queue.
 */
void queue_shuffle()
{
	struct play_queue *shuffle_array[queue_entry->size-1];
	int cnt = 0;

	struct play_queue *current = queue_entry->head;
    if(current == NULL || current->next == NULL) {
        printf("Not enough tracks in queue to shuffle\n");
        return;
    }

	while(cnt < queue_entry->size) {
		shuffle_array[cnt] = current;
		current = current->next;
		cnt++;
	}

	int i = queue_entry->size-1;
	struct play_queue *tmp;

	for(i = queue_entry->size-1; i > 0; i--) {
		int rand = rand_lim(i);
		tmp = shuffle_array[rand];
		shuffle_array[rand] = shuffle_array[i];
		shuffle_array[i] = tmp;
	}

	cnt = 0;
    queue_entry->head = shuffle_array[0];
	current = queue_entry->head;
    cnt++;

	while(cnt < queue_entry->size-1) {
		current->next = shuffle_array[cnt];
		current = current->next;
		cnt++;
	}

    current->next = shuffle_array[cnt];
    current = current->next;
    queue_entry->tail = current;
    current->next = NULL;

    queue_info();
}

/* 
 * Correct way to get random number withing a limit 
 */
int rand_lim(int limit) {
	srand(time(NULL));	
    int divisor = RAND_MAX/(limit+1);
    int retval;

    do { 
        retval = rand() / divisor;
    } while (retval > limit);

    return retval;
}

/*
 * Add an entire playlist to the queue
 */
void queue_add_playlist(sp_playlist *playlist)
{
    int i = 0;
    
    for(i = 0; i < sp_playlist_num_tracks(playlist) - 1; i++) {
        queue_add(sp_playlist_track(playlist, i));
    }
}

/*
 * Jump to next in queue and play the track
 */
void queue_go_next(sp_session* s)
{
    struct play_queue *fre;
    fre = queue_entry->head;
    free(fre);
    queue_entry->head = queue_entry->head->next;
    queue_entry->size--;
    if(queue_entry->head == NULL) {
        printf("End of queue!");
        return;
    }
    struct play_queue *current = queue_entry->head;
    play(s, current->track);
    
}

/*
 * Print the full queue
 */
void queue_print(struct play_queue *node) 
{
    printf("printing queue\n");
	printf("queue size: %d\n", queue_entry->size);
    struct play_queue *current = queue_entry->head;

    int i;
    while(current != NULL) {
        printf("%d: %s\n", i++, sp_track_name(current->track));
        current = current->next;
    }
    printf("done!\n");
}

/* 
 * Free memory of entire queue. Used for exit of program 
 */
void queue_free()
{
    struct play_queue *tmp;
    struct play_queue *cur;
    
    cur = queue_entry->head;
    
    while(cur != NULL) {
        tmp = cur->next;
        free(cur);
        cur = tmp;
    }
}

