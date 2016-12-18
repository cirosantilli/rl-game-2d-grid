# General Game AI

Ubuntu 16.10:

    ./configure
    make
    ./main.out -h
    ./main.out -w 10 -i

Platform for General Game AI competition. See:

- <https://en.wikipedia.org/wiki/General_game_playing>
- <http://www.gvgai.net/>
- <http://togelius.blogspot.co.uk/2016/07/which-games-are-useful-for-testing.html>

The goal is to design a game to be played for computers, to have an interesting problem (looks like a simplified real world) to develop artificial intelligence.

Currently only supports one tile based rogue like game, but it is designed so that any game type can be added.

Features:

- insanely configurable human controls, ranging from TAS, rogue to ARPG, simply via different command line options, for you to manually play games to get a feeling for them, and build better AI later
- clearly separated interfaces for world state consumed by actors (players), and action given back

TODO features:

- actors run in separate processes to prevent cheating by reading memory space of full world + allowing any language to be used. Requires serialization of world views and actions.
