#include <cassert>

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
    std::unique_ptr<DrawableObject> drawableObject,
    unsigned int id
) :
    actor(std::move(actor)),
    drawableObject(std::move(drawableObject)),
    type(type),
    fov(fov),
    id(id),
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
unsigned int Object::getId() const { return this->id; }
unsigned int Object::getScore() const { return this->score; }
auto Object::getType() const -> Type { return this->type; }
unsigned int Object::getX() const { return this->x; }
unsigned int Object::getY() const { return this->y; }
void Object::setScore(unsigned int score) { this->score = score; }
void Object::setX(unsigned int x) { this->x = x; }
void Object::setY(unsigned int y) { this->y = y; }

bool Object::PointerCmp::operator()(const T& c, unsigned int id) const { return c->getId() < id; }
bool Object::PointerCmp::operator()(unsigned int id, const T& c) const { return id < c->getId(); }
bool Object::PointerCmp::operator()(const T& lhs, const T& rhs) const {
    return lhs->getId() < rhs->getId();
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
        case Object::Type::DO_NOTHING:
            os << "DO_NOTHING";
        break;
        case Object::Type::MOVE_UP:
            os << "MOVE_UP";
        break;
        case Object::Type::MOVE_DOWN:
            os << "MOVE_DOWN";
        break;
        case Object::Type::RANDOM:
            os << "RANDOM";
        break;
        case Object::Type::FOLLOW_HUMAN:
            os << "FOLLOW_HUMAN";
        break;
        case Object::Type::FLEE_HUMAN:
            os << "FLEE_HUMAN";
        break;
        case Object::Type::HUMAN:
            os << "HUMAN";
        break;
        case Object::Type::WALL:
            os << "WALL";
        break;
        case Object::Type::PLANT:
            os << "PLANT";
        break;
        case Object::Type::PLANT_EATER:
            os << "PLANT_EATER";
        break;
        default:
            assert(false);
        break;
    }
    return os;
}
