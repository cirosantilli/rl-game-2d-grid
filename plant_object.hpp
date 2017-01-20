#ifndef PLANT_OBJECT_HPP
#define PLANT_OBJECT_HPP

#include "object.hpp"

class PlantObject : public Object {
    public:
        PlantObject(
            unsigned int x,
            unsigned int y,
            std::unique_ptr<Actor> actor,
            unsigned int value = 1
        );
        // How much this plant is worth to eater.
        int getValue();
    private:
        int value;
};

#endif
