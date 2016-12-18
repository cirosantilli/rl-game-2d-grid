#ifndef DO_NOTHING_DRAWABLE_OBJECT_HPP
#define DO_NOTHING_DRAWABLE_OBJECT_HPP

#include "drawable_object.hpp"

class Object;
class World;

/// Placeholder for when we don't have any display.
class DoNothingDrawableObject : public DrawableObject {
    public:
        DoNothingDrawableObject();
        virtual void draw(const World& world, const Object& object, int cameraX = 0, int cameraY = 0) const;
};

#endif
