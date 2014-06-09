#include "includes.h"
int id_tracker;

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
}
 
void queue_remove(int id)
{

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
    struct play_queue *current = node;
    current = current->next;
    sp_track *cur_track;

    while(current != NULL) {
        printf("%d: %s\n", current->id, sp_track_name(current->track));
        current = current->next;
    }
    printf("done!\n");
}
