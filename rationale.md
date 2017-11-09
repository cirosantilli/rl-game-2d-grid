# Rationale

See:

-   <https://en.wikipedia.org/wiki/General_game_playing>
-   <http://www.gvgai.net/>. Large overlap. But we aim at "better" features like
    process separation, C++
-   <http://togelius.blogspot.co.uk/2016/07/which-games-are-useful-for-testing.html>
-   <https://github.com/cirosantilli/gpu-robot-vision-simulation-full-ai> superset
    of this

This is a game designed to be interesting for both computers and humans to play.

The goal is to serve as an interesting problem for people to develop intelligent
algorithms.

New mechanics will be added from time to time. The final goal is to have a
single AI that can understand those new mechanics.

Mechanics will have some analogy with the real world. E.g., eating plants and
escaping from predators.

Features:

-   headless mode: game visualization is completely optional:
    training can be done without any GUI as fast as the world simulation allows.

    Note: at some point we might start adding games that use GPU render output,
    then things will get [more complicated](https://stackoverflow.com/questions/3191978/how-to-use-glut-opengl-to-render-to-a-file/14324292#14324292).

-   insanely configurable human controls types, ranging from TAS, to rogue-like,
    to real time, simply via different command line options.

-   clearly separated interfaces for world state consumed by actors (players),
    and action given back. In the future, this will allow to:

    - prevent actors from cheating by reading extra world state
    - sandbox actors to limit their resources
