#include "plant_object.hpp"

#include "actor.hpp"
#include "drawable_object.hpp"

PlantObject::PlantObject(
    unsigned int x,
    unsigned int y,
    std::unique_ptr<Actor> actor,
    unsigned int value
) :
    Object(
        x,
        y,
        Type::PLANT,
        std::move(actor),
        0
    ),
    value(value)
{}

int PlantObject::getValue() {
    return this->value;
}
