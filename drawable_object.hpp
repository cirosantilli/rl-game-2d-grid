#ifndef DRAWABLE_OBJECT_HPP
#define DRAWABLE_OBJECT_HPP

#include <SDL2/SDL.h>

class Object;
class World;

class DrawableObject {
    public:
        virtual ~DrawableObject();
        // cameraX / Y: lower left corner of the camera.
        // Signed as it might go out of the scenario if player goes to world corner.
        virtual void draw(
            const World& world,
            const Object& object,
            int cameraX = 0,
            int cameraY = 0
        ) const = 0;
};

#endif
