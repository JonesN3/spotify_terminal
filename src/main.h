char* username;
char* password;

extern int g_playing;
extern int g_process_running;

void init(void); 
void get_user_info(void); 
int log_in(void);
void shutdown();
