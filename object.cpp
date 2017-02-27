#include <cassert>

#include "actor.hpp"
#include "do_nothing_actor.hpp"
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
    std::unique_ptr<DrawableObject> drawableObject,
    unsigned int id
) :
    actor(std::move(actor)),
    drawableObject(std::move(drawableObject)),
    type(type),
    score(0),
    fov(fov),
    id(id),
    x(x),
    y(y)
{}
Object::Object(
    unsigned int x,
    unsigned int y,
    Type type
) :
    Object(x, y, type, std::make_unique<DoNothingActor>())
{}
Object::~Object() {}
void Object::draw(const World& world, int cameraX, int cameraY) const {
    this->drawableObject->draw(world, *this, cameraX, cameraY);
}
Actor& Object::getActor() const { return *this->actor; }
unsigned int Object::getFov() const { return this->fov; }
unsigned int Object::getId() const { return this->id; }
int Object::getScore() const { return this->score; }
auto Object::getType() const -> Type { return this->type; }
unsigned int Object::getX() const { return this->x; }
unsigned int Object::getY() const { return this->y; }
void Object::setDrawableObject(std::unique_ptr<DrawableObject>&& drawableObject) {
    this->drawableObject = std::move(drawableObject);
}
void Object::setId(unsigned int id) { this->id = id; }
void Object::setScore(int score) { this->score = score; }
void Object::setX(unsigned int x) { this->x = x; }
void Object::setY(unsigned int y) { this->y = y; }

bool Object::PointerCmp::operator()(const T& l, unsigned int r) const { return l->getId() < r; }
bool Object::PointerCmp::operator()(unsigned int l, const T& r) const { return l < r->getId(); }
// TODO get this version working.
bool Object::PointerCmp::operator()(const T& l, const Object *r) const { return l->getId() < r->getId(); }
bool Object::PointerCmp::operator()(const Object *l, const T& r) const { return l->getId() < r->getId(); }
bool Object::PointerCmp::operator()(const T& l, const T& r) const {
    return l->getId() < r->getId();
}

std::ostream& operator<<(std::ostream& os, const Object& o) {
    return os
        << "Object"
        << ", x = " << o.getX()
        << ", y = " << o.getY()
        << ", type = " << o.getType()
        << ", score = " << o.getScore()
        << ", fov = " << o.getFov()
    ;
}

std::ostream& operator<<(std::ostream& os, const Object::Type& t) {
    switch (t) {
        OBJECT_TYPE_FOREACH(CASE_COUT)
    }
    return os;
}
