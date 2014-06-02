#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include <sys/select.h>
#include <libspotify/api.h>

#include "string.h"
#include "debug.h"
#include "play_queue.h"
#include "main.h"
#include "handler.h"
#include "search.h"
#include "audio.h"
#include "queue.h"
#include "playlist.h"
#include "play.h"


#define TRUE 1
#define FALSE 0
#define PLAY 2
#define SHUFFLE 3 


