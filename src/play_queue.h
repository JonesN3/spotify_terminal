struct play_queue {
    sp_track *track;
    char* playlist_name;
    int id;
    struct play_queue *next;
};

void queue_add(struct play_queue *node, sp_track *track);
void queue_remove(int id);
void queue_add_first(struct play_queue *node, sp_track *track);
void queue_add_playlist(struct play_queue *node, sp_playlist *playlist);
void queue_print(struct play_queue *node); 
