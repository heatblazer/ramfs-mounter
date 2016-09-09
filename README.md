ramfs-mount
============

Simple app for ramfs mounting in the curent directory.
This is SUID-flag based app, so no root permision needed to do it.

The main purpuse of ramfs-mount is to prevents SSD disk from frequent writes
when QT-Creator build your projects .


## Build

    $ cd ramfs-mount
    $ qmake
    $ make install


## Usage

    $ cd my/big/project/shadow-build-dir
    $ ramfs-mount


## QtCreator integration

Just make it your first-build-step in the project settings.

![qt-integration](../plain/readme-qt-integration.png)


Enjoy :)
Btel masters.
