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
-   use quadtree (same as B-tree width width 4?) for searches.
    Boost geometry has some classes: http://www.boost.org/doc/libs/1_58_0/libs/geometry/doc/html/geometry/spatial_indexes/introduction.html
    R-tree is even more efficient as it is not restricted to halves only.
-   shortcut to redo previous human action, specially for TAS mode
-   pause button when -b is given
-   optionally draw grids to screen for debugging
-   more human understandable textures (?) This allows human to cheat by using external world knowledge. Possibly use random letters as textures? 
-   give string names to textures
-   different action types for different actors
-   choose which players the human will control from the CLI, allow any player to be chosen by ID (currently only possible to choose which player to observe)
-   use `std::random_device` for the random numbers as in http://stackoverflow.com/questions/322938/recommended-way-to-initialize-srand/13004555#13004555 Difficulty: RandomActor would have to be treated like HumanActor, as the input is not calculated entirely from the world view.

## Plant eater game

-   toroidal world, or world with closed barriers (invisible walls are a hard mechanic for AI to grasp!)
-   non-uniform spacial distributions of plants and plant eaters
-   some condition where collaboration leads to victory vs desire to betray friends
-   only spawn new plants next to existing ones. This would create agriculture logistics.
