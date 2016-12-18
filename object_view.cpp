#include "object_view.hpp"

ObjectView::ObjectView(int x, int y, Object::Type type)
    : x(x), y(y), type(type) {}
int ObjectView::getX() const { return this->x; }
int ObjectView::getY() const { return this->y; }
Object::Type ObjectView::getType() const { return this->type; }
std::ostream& operator<<(std::ostream& os, const ObjectView& o) {
    return os << "ObjectView, x = " << o.getX() << ", y = " << o.getY();
}
