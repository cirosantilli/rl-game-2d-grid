#ifndef WORLD_HPP
#define WORLD_HPP

#include <ctime>
#include <set>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <boost/geometry.hpp>

#include "object.hpp"

class Action;
class ObjectView;
class WorldView;

// TODO make local to class.
namespace bg = boost::geometry;
namespace bgi = bg::index;

class World {
    public:
        World(
            unsigned int width,
            unsigned int height,
            bool display,
            unsigned int windowWidthPix,
            unsigned int windowHeightPix,
            unsigned int showPlayerId,
            bool showFov,
            bool fixedRandomSeed,
            int randomSeed,
            unsigned int nHumanPlayers,
            std::string scenario,
            int timeLimit,
            bool verbose
        );
        ~World();
        void draw() const;
        void destroyTextures();
        /// Collect desired actions from all objects, and resolve them
        unsigned int getHeight() const;
        unsigned int getNHumanActions() const;
        SDL_Renderer * getRenderer() const;
        unsigned int getTileWidthPix() const;
        unsigned int getTileHeightPix() const;
        unsigned int getWidth() const;
        unsigned int getViewHeight() const;
        /// Initialize world.
        /// When placing objects, prefer to put human controlled objects first,
        /// to make them easier to select from the command line.
        void init();
        bool isGameOver() const;
        void printScores() const;
        void reset();
        /// Update to the next world state. E.g.: what happens if two objects
        /// want to move to the same place next tick? Or if an object
        /// wants to move into a wall?
        void update(const std::vector<std::unique_ptr<Action>> &humanActions);
    private:
        // Types
        /// id -> object.
        typedef std::map<unsigned int, std::unique_ptr<Object>> objects_t;
        //typedef std::set<std::unique_ptr<Object>, Object::PointerCmp> objects_t;
        typedef std::vector<SDL_Texture*> textures_t;

        // Boost types.
        typedef bg::model::box<Object> Box;
        typedef bgi::rtree<Object*, bgi::linear<16>> Rtree;

        // Data.
        bool
            display,
            fixedRandomSeed,
            showFov,
            verbose
        ;
        int
            randomSeed,
            timeLimit
        ;
        std::string scenario;
        unsigned int
            height,
            hud_text_x,
            // Number of human actions required.
            // May be != nHumanPlayers, since human players might die in the middle of the game.
            nHumanActions,
            // How many human players existed when the game started.
            // Not affected by human players that died.
            nHumanPlayersInitial,
            showPlayerId,
            ticks,
            tileHeightPix,
            tileWidthPix,
            width,
            windowHeightPix,
            windowWidthPix,
            viewHeight
        ;
        SDL_Renderer *renderer;
        SDL_Window *window;
        Rtree rtree;
        TTF_Font *font;
        objects_t objects;
        std::vector<SDL_Texture *> textures;

        // Methods.
        SDL_Texture * createSolidTexture(unsigned int r, unsigned int g, unsigned int b, unsigned int a);
        std::unique_ptr<WorldView> createWorldView(const Object &object) const;
        /// Should always be used for object creation instead of raw insertion into data types,
        /// ince a single insertion may require multiple index updates.
        void createObject(std::unique_ptr<Object> object);
        void createSingleTextureObject(
            unsigned int x,
            unsigned int y,
            Object::Type type,
            std::unique_ptr<Actor> actor,
            unsigned int fov,
            textures_t::size_type textureId
        );
        void deleteObject(objects_t::iterator& it);
        template<typename ITERATOR>
        bool findNextObjectInFov(objects_t::const_iterator& it, const Object& object, int& dx, int& dy) const;
        /// Advance iterator until the next object in the FOV of object, including it itself.
        /// Return true if such object exists, false if there are no more.
        /// The caller is responsible for incrementing the pointer to ask for the next object.
        template<typename ITERATOR>
        bool findNextObjectInRectangle(
            ITERATOR& it,
            unsigned int centerX,
            unsigned int centerY,
            unsigned int width,
            unsigned int height,
            int& dx,
            int& dy
        ) const;
        /// Make iterator point to the object at a given tile.
        /// Return true iff an object is present at that position.
        template<typename ITERATOR>
        bool findObjectAtTile(ITERATOR& it, unsigned int x, unsigned int y) const;
        objects_t::key_type getNextFreeObjectId();
        /// Should we only show the FOV for a single object on screen? Or show every object?
        bool getShowFov() const;
        bool needFpsUpdate() const;
        /// Check if a given tile is empty.
        bool isTileEmpty(unsigned int x, unsigned int y) const;
        void updatePosition(Object& object, unsigned int x, unsigned int y);

        // Static const.
        constexpr static const unsigned int COLOR_MAX = 255;
        constexpr static const unsigned int N_COLOR_CHANNELS = 4;
        constexpr static const unsigned int HUD_WIDTH_PIX = 200;

        static void render_text(
            SDL_Renderer *renderer,
            int x,
            int y,
            const char *text,
            TTF_Font *font,
            SDL_Rect *rect,
            SDL_Color *color
        );
};

#endif
