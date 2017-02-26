#include "fruit_object.hpp"

#include "actor.hpp"
#include "drawable_object.hpp"

FruitObject::FruitObject(
    unsigned int x,
    unsigned int y,
    unsigned int value
) :
    Object(x, y, Type::FRUIT),
    value(value)
{}

int FruitObject::getValue() {
    return this->value;
}
