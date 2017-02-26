#include "teleport_object.hpp"

#include "actor.hpp"
#include "drawable_object.hpp"

TeleportObject::TeleportObject(
    unsigned int x,
    unsigned int y,
    TeleportObject *destination
) :
    Object(x, y, Type::TELEPORT),
    destination(destination)
{}

TeleportObject* TeleportObject::getDestination() {
    return this->destination;
}

void TeleportObject::setDestination(TeleportObject *destination) {
    this->destination = destination;
}
