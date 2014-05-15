
int print_playlists(sp_session *g_session, sp_playlistcontainer* pc); 
void playthatlist(sp_session* session, sp_playlist* playlist); 
sp_playlist* playlist_find_by_num(sp_session *session, sp_playlistcontainer* pc);
void playlist_init(sp_session* session);
void print_tracks_in_playlist(sp_session *session, sp_playlist* pl);
void playlist_go_next(sp_session *session, sp_playlist* pl, int index);
void shuffle(sp_playlist* playlist); 
void playlist_play_track(sp_session* session, sp_track *track);
