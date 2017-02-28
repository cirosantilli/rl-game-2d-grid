# TODO

-   use protobuf serialization for full world state, controller world view, and controller actions
-   put players in separate processes, and control resources:
    - time
    - RAM
    - disk: loopback + seccomp should suffice
    But hopefully allow for access to:
    - threading
    - GPU (TODO how to enforce fair resource sharing of GPU shaders?)
    Should we use raw seccomp, or a more full blown docker?
-   shortcut to redo previous human action, specially for TAS mode
-   optionally draw grids to screen for debugging
-   different action types for different actors
-   choose which players the human will control from the CLI, allow any player to be chosen by ID (currently only possible to choose which player to observe, not control)
-   if the simulation is large, most keys get ignored. Why?
-   save current world state. Hard, as it requires saving the entire actor state, so basically saving a program for later. We could require actors to implement a serialize method however...

## Plant eater game

-   toroidal world
    - https://en.wikipedia.org/wiki/Wraparound_(video_games)
    - semi achievable with current teleports (except vision does not cross them)
-   some condition where collaboration leads to victory vs desire to betray friends
    - two players must stand on a tile at the same time for a door to open, filled with great fruits
-   only spawn new plants next to existing ones. This would create agriculture logistics.
-   Gaussian probability for tree fruit spawn
-   doors and controlling buttons
