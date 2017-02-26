#ifndef FRUIT_OBJECT_HPP
#define FRUIT_OBJECT_HPP

#include "object.hpp"

class FruitObject : public Object {
    public:
        FruitObject(
            unsigned int x,
            unsigned int y,
            unsigned int value = 1
        );
        // How much this fruit is worth to eater.
        int getValue();
    private:
        int value;
};

#endif
