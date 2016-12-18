#ifndef OBJECT_VIEW_HPP
#define OBJECT_VIEW_HPP

#include <iostream>

// Because: http://stackoverflow.com/questions/2238170/forward-declaration-of-nested-enum
#include "object.hpp"

/// The properties of an object that actors can see.
/// E.g., actors may not know the line of sight of other objects.
class ObjectView {
    public:
        ObjectView(int x, int y, Object::Type type);
        int getX() const;
        int getY() const;
        Object::Type getType() const;
    private:
        int x;
        int y;
        Object::Type type;
};

std::ostream& operator<<(std::ostream& os, const ObjectView& o);

#endif
