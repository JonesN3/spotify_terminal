/* the play_queue structure */
struct play_queue {
    sp_track *track;
    char* playlist_name;
    int id;
    struct play_queue *next;
};

/* entry for the play_queue which points to the play_queue*/
typedef struct {
	int size;
	int is_playing;
	struct play_queue *head;
    struct play_queue *tail;
    struct play_queue *playing;
} q_entry;

void queue_shuffle();
int rand_lim(int limit); 
void queue_remove(int id);
void queue_go_next(sp_session* s);
void queue_add(sp_track *track);
void queue_add_first(sp_track *track);
void queue_add_playlist(sp_playlist *playlist);
void queue_print(struct play_queue *node); 
void queue_free();
