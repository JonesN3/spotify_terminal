/* the play_queue structure */
struct play_queue {
    sp_track *track;
    char* playlist_name;
    int id;
    struct play_queue *next;
	struct play_queue *tail;
};

/* entry for the play_queue which contains the size */
typedef struct {
	int size;
	struct play_queue *head;
    struct play_queue *tail;
} q_entry;

void queue_add(sp_track *track);
void queue_shuffle();
int rand_lim(int limit); 
void queue_remove(int id);
void queue_go_next(sp_session* s);
void queue_add_first(struct play_queue *node, sp_track *track);
void queue_add_playlist(sp_playlist *playlist);
void queue_print(struct play_queue *node); 
