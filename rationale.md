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

-   insanely configurable human controls, ranging from TAS, rogue to ARPG,
-   simply via different command line options, for you to manually play games to
    get a feeling for them, and build better AI later
-   clearly separated interfaces for world state consumed by actors (players),
    and action given back
