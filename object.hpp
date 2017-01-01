#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <memory>
#include <iostream>

class Actor;
class DrawableObject;
class World;

class Object {
    public:
        enum class Type {
            DO_NOTHING,
            MOVE_UP,
            MOVE_DOWN,
            RANDOM,
            FOLLOW_HUMAN,
            FLEE_HUMAN,
            HUMAN,
            WALL,
            PLANT,
            PLANT_EATER
        };
        Object();
        virtual ~Object();
        Object(
            unsigned int x,
            unsigned int y,
            Type type,
            std::unique_ptr<Actor> actor,
            unsigned int fov,
            std::unique_ptr<DrawableObject> drawableObject
        );
        void draw(const World& world, int cameraX = 0, int cameraY = 0) const;
        Actor& getActor() const;
        unsigned int getFov() const;
        unsigned int getScore() const;
        unsigned int getX() const;
        unsigned int getY() const;
        auto getType() const -> Type;
        void setScore(unsigned int score);
        void setX(unsigned int x);
        void setY(unsigned int y);
    protected:
        std::unique_ptr<Actor> actor;
        std::unique_ptr<DrawableObject> drawableObject;
        Type type;
        unsigned int
            /// How much can this object see.
            /// 0 means nothing, not even itself.
            /// 1 means just itself.
            /// 2 means itself + the 8 adjacent squares (infinite norm).
            /// n means sees a rectangle of width 2 * (n - 1) + 1
            fov,
            score,
            x,
            y
        ;
};

std::ostream& operator<<(std::ostream& os, const Object& o);
std::ostream& operator<<(std::ostream& os, const Object::Type& t);

#endif
