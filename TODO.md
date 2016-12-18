# TODO

-   implement one type of score
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
-   redo previous human action
-   pause
-   draw grids to screen
-   toroidal world, or world with closed barriers (invisible walls are a hard mechanic for AI to grasp!)
-   more human understandable textures (?) This allows human to cheat by using external world knowledge. Possibly use random letters as textures? 
-   advance world if `-H -f 2.0` and some key is being held at the end.
