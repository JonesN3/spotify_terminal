int print_playlists(sp_session *g_session, sp_playlistcontainer* pc); 
sp_playlist* playlist_find_by_num(sp_session *session, sp_playlistcontainer* pc);
void playlist_init(sp_session* session);
void print_tracks_in_playlist(sp_session *session, sp_playlist* pl);
sp_playlist* playlist_play_by_index(sp_session *session, sp_playlistcontainer *pc, int index);
sp_playlist* playlist_find_by_name(sp_playlistcontainer* pc, char *name);
void playqueue_go_next(sp_session *session, struct play_queue** node); 
sp_track* pl_find_song_by_id(sp_playlist* pl, int id);
