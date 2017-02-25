#include "fruit_object.hpp"

#include "actor.hpp"
#include "drawable_object.hpp"

FruitObject::FruitObject(
    unsigned int x,
    unsigned int y,
    std::unique_ptr<Actor> actor,
    unsigned int value
) :
    Object(
        x,
        y,
        Type::FRUIT,
        std::move(actor),
        0
    ),
    value(value)
{}

int FruitObject::getValue() {
    return this->value;
}
