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
-   ddef3b5523fbfca746e83bfc31758b2c71f0787a `-H` limits FPS to the default `1.0` if you hold a key like LEFT, but `-b -H` does not limit FPS. `-f -1.0` works as workaround, but options confusing. Either missing `-f` should always mean unlimited FPS, or `-f -1.0` should be required.

## Plant eater game

-   toroidal world, or world with closed barriers (invisible walls are a hard mechanic for AI to grasp!)
-   non-uniform spacial distributions of plants and plant eaters
-   some condition where collaboration leads to victory vs desire to betray friends
-   only spawn new plants next to existing ones. This would create agriculture logistics.
