spotify_terminal
================
A project to create a simple, but fuctional terminal spotify client for a linux enviorment. 
The project is in early development, and cannot be considered stable!

Dependencies
------------
This program depends on libspotify, a C api develpoed by spotify. 
Download from [here](https://developer.spotify.com/technologies/libspotify/#libspotify-downloads).
More info on how to manage libspotify in 'Libspotify_howto' below.

You will also need the 'asoundlib.h' library, read about it [here](http://www.alsa-project.org/alsa-doc/alsa-lib/files.html).
This should be available from your distro's offical repositories.
(Ubuntu/debian: 'libasound2-dev'. Arch: 'alsa-lib')

Build and run
-------------
'make' for building the program. 'make debug' for building the program with debug outputs.

Libspotify howto
----------------
If you have root access, pretty simple. Just extract the tar, go to the folder, and run 
<code>'make install'</code>.

If you don't have root access, you can install libspotify in your home folder. For what i will explain i will
that you installed it to '~/usr/local/lib', with the command
<code>'make install prefix=~/usr/local/lib'</code> 

If you stilled it into the home folder, you have to update some PATHS, if you already haven't

<code>export PKG_CONFIG_PATH=$HOME/local/lib/pkgconfig:$PKG_CONFIG_PATH</code>

<code>export LD_LIBRARY_PATH=$HOME/local/lib:$LD_LIBRARY_PATH</code>

You can do this each time before running or launching spotify_terminal, or you can add the lines
to ~/.bashrc etc.
