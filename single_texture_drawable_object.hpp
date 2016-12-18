#ifndef SINGLE_TEXTURE_DRAWABLE_OBJECT_HPP
#define SINGLE_TEXTURE_DRAWABLE_OBJECT_HPP

#include "drawable_object.hpp"

class Object;
class World;

class SingleTextureDrawableObject : public DrawableObject {
    public:
        SingleTextureDrawableObject(SDL_Texture *texture);
        virtual void draw(
            const World& world,
            const Object& object,
            int cameraX,
            int cameraY
        ) const;
    private:
        /// Pointer to texture shared across all objects that look the same.
        SDL_Texture * texture;
};

#endif
