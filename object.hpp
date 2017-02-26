#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <memory>
#include <iostream>

#include <boost/geometry.hpp>

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
            FRUIT,
            FRUIT_EATER,
            TREE,
            TELEPORT,
        };
        Object();
        virtual ~Object();
        Object(
            unsigned int x,
            unsigned int y,
            Type type,
            std::unique_ptr<Actor> actor,
            unsigned int fov = 0,
            std::unique_ptr<DrawableObject> drawableObject = nullptr,
            unsigned int id = 0
        );
        Object(
            unsigned int x,
            unsigned int y,
            Type type
        );
        void draw(const World& world, int cameraX = 0, int cameraY = 0) const;
        Actor& getActor() const;
        unsigned int getFov() const;
        unsigned int getId() const;
        int getScore() const;
        unsigned int getX() const;
        unsigned int getY() const;
        auto getType() const -> Type;
        void setDrawableObject(std::unique_ptr<DrawableObject>&& drawableObject);
        void setId(unsigned int id);
        void setScore(int score);
        void setX(unsigned int x);
        void setY(unsigned int y);

        // Types.
        struct PointerCmp {
            // Required for the heterogeneous searches to be found by compiler.
            // Value does not seem to matter, only presence, but std::true_type is a good choice semantically.
            typedef std::true_type is_transparent;
            typedef std::unique_ptr<Object> T;
            bool operator()(const T& c, unsigned int id) const;
            bool operator()(unsigned int id, const T& c) const;
            bool operator()(const T& lhs, const T& rhs) const;
        };
    protected:
        std::unique_ptr<Actor> actor;
        std::unique_ptr<DrawableObject> drawableObject;
        Type type;
        int score;
        unsigned int
            /// How much can this object see.
            /// 0 means nothing, not even itself.
            /// 1 means just itself.
            /// 2 means itself + the 8 adjacent squares (infinite norm).
            /// n means sees a rectangle of width 2 * (n - 1) + 1
            fov,
            id,
            x,
            y
        ;
};

std::ostream& operator<<(std::ostream& os, const Object& o);
std::ostream& operator<<(std::ostream& os, const Object::Type& t);

namespace bg = boost::geometry;
namespace bgi = bg::index;

namespace boost { namespace geometry {
    // Adapt to Object to Boost Point.
    namespace traits {
        template<> struct tag<Object>
        { typedef point_tag type; };

        template<> struct coordinate_type<Object>
        { typedef int type; };

        template<> struct coordinate_system<Object>
        { typedef cs::cartesian type; };

        template<> struct dimension<Object> : boost::mpl::int_<2> {};

        template<>
        struct access<Object, 0> {
            static int get(Object const& p) {
                return p.getX();
            }
            static void set(Object& p, int const& value) {
                p.setX(value);
            }
        };

        template<>
        struct access<Object, 1> {
            static int get(Object const& p) {
                return p.getY();
            }
            static void set(Object& p, int const& value) {
                p.setY(value);
            }
        };
    }

    // Adapt Object* to Object.
    namespace index {
        template <typename Box>
        struct indexable<Box*> {
            typedef Box* V;
            typedef Box const& result_type;
            result_type operator()(V const& v) const { return *v; }
        };
    }
}}

// Simplified object for making queries.
typedef bg::model::point<int, 2, bg::cs::cartesian> QueryPoint;
typedef bg::model::box<QueryPoint> QueryBox;

#endif
