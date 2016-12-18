#include "drawable_object.hpp"
#include "object.hpp"
#include "single_texture_drawable_object.hpp"
#include "world.hpp"

SingleTextureDrawableObject::SingleTextureDrawableObject(SDL_Texture *texture) :
    texture(texture) {}
void SingleTextureDrawableObject::draw(
    const World& world,
    const Object& object,
    int cameraX,
    int cameraY
) const {
    SDL_Rect rect;
    rect.x = (object.getX() - cameraX) * world.getTileWidthPix();
    rect.y = ((world.getViewHeight() - 1) - (object.getY() - cameraY)) * world.getTileHeightPix();
    rect.w = world.getTileWidthPix();
    rect.h = world.getTileHeightPix();
    SDL_RenderCopy(world.getRenderer(), this->texture, NULL, &rect);
}
