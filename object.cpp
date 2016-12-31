#include "actor.hpp"
#include "do_nothing_drawable_object.hpp"
#include "drawable_object.hpp"
#include "object.hpp"

Object::Object() : drawableObject(new DoNothingDrawableObject()) {}
Object::Object(
    unsigned int x,
    unsigned int y,
    Type type,
    std::unique_ptr<Actor> actor,
    unsigned int fov,
    std::unique_ptr<DrawableObject> drawableObject
) :
    actor(std::move(actor)),
    drawableObject(std::move(drawableObject)),
    type(type),
    fov(fov),
    score(0),
    x(x),
    y(y)
{}
Object::~Object() {}
void Object::draw(const World& world, int cameraX, int cameraY) const {
    this->drawableObject->draw(world, *this, cameraX, cameraY);
}
Actor& Object::getActor() const { return *this->actor; }
unsigned int Object::getFov() const { return this->fov; }
unsigned int Object::getScore() const { return this->score; }
Object::Type Object::getType() const { return this->type; }
unsigned int Object::getX() const { return this->x; }
unsigned int Object::getY() const { return this->y; }
void Object::setScore(unsigned int score) { this->score = score; }
void Object::setX(unsigned int x) { this->x = x; }
void Object::setY(unsigned int y) { this->y = y; }

std::ostream& operator<<(std::ostream& os, const Object& o) {
    return os
        << "Object"
        << ", x = " << o.getX()
        << ", y = " << o.getY()
        << ", type = " << static_cast<typename std::underlying_type<Object::Type>::type>(o.getType())
        << ", score = " << o.getScore()
        << ", fov = " << o.getFov()
    ;
}
