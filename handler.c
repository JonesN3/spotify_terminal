int handler(sp_session *session)
{
    printf("\nHandler \n");
    int next_timeout = 0;
    int index;
    int selection = 0;
    char input[100];

    globPlaying = 0;
    selection = g_menuChoice;
    if(g_menuChoice == -1 || g_menuChoice == 9) selection = menu();
    g_menuChoice = selection;
    if(selection == 0) quit();

    while(1) {
        sp_session_process_events(session, &next_timeout);
        signal(SIGINT, intHandler);

        if(!globPlaying)
        {
            if(selection == -1) handler(session);
            if(g_menuChoice == -1 || g_menuChoice == 9) handler(session);

            if(selection == 1)
            { 
                run_search(session);
                globPlaying = 1;
            }else if(selection == 2) {
                printf("playlist play test");
                testPlaylistPlay(session, 0);
            } else if(selection == 3) {
                printPlaylists(g_session);
            } else if(selection == 4) {
                fputs("Playlist number: ", stdout);
                fgets(input, sizeof(input) - 1, stdin);
                sscanf(input, "%d", &index);
                listSongsInPlaylist(session, index);
            } else if(selection == 5) {
                playthatlist();
            } else if(selection == 6) {
                printf("Shuffle and play");
                g_shuffleMode2 = 1;
                playthatlist();
            } else {
                printf("\nerror: illegal menu choice\n");
                g_menuChoice = 9;
            }
        } else {
            //usleep(4000);
            //if(g_playbackOn == 1) playShell();
            while(g_playbackOn == 1)
            {

            }
        }
    }
}