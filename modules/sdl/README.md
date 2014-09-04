LibSDL 2.0
=======

This build of LibSDL2 was done on the following host:

ggg@scilla:~$ uname -a
Linux scilla 3.2.0-64-generic #96-Ubuntu SMP Wed May 21 17:16:14 UTC 2014 x86_64 x86_64 x86_64 GNU/Linux
ggg@scilla:~$ lsb_release -a
LSB Version:    core-2.0-amd64:core-2.0-noarch:core-3.0-amd64:core-3.0-noarch:core-3.1-amd64:core-3.1-noarch:core-3.2-amd64:core-3.2-noarch:core-4.0-amd64:core-4.0-noarch
Distributor ID: Ubuntu
Description:    Ubuntu 12.04.5 LTS
Release:        12.04
Codename:       precise

The configure script to build was:
CFLAGS=-fPIC ./configure --prefix=/home/ggg/libsdl2-install --disable-sdl-dlopen --disable-video-opengl --disable-loadso --enable-static --disable-shared --disable-dbus

Note the importance of -fPIC !
