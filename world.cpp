#include <cstdlib>

#include "action.hpp"
#include "actor.hpp"
#include "actor.hpp"
#include "do_nothing_actor.hpp"
#include "do_nothing_drawable_object.hpp"
#include "drawable_object.hpp"
#include "flee_human_actor.hpp"
#include "follow_human_actor.hpp"
#include "human_actor.hpp"
#include "move_down_actor.hpp"
#include "move_up_actor.hpp"
#include "object.hpp"
#include "object_view.hpp"
#include "random_actor.hpp"
#include "single_texture_drawable_object.hpp"
#include "world.hpp"
#include "world_view.hpp"

World::World(
    unsigned int width,
    unsigned int height,
    bool display,
    unsigned int windowWidthPix,
    unsigned int windowHeightPix,
    int showFovId,
    bool fixedRandomSeed,
    int randomSeed,
    bool multiHumanPlayer,
    std::string scenario
) :
    display(display),
    fixedRandomSeed(fixedRandomSeed),
    multiHumanPlayer(multiHumanPlayer),
    randomSeed(randomSeed),
    showFovId(showFovId),
    scenario(std::move(scenario)),
    height(height),
    width(width),
    windowHeightPix(windowHeightPix),
    windowWidthPix(windowWidthPix)
{
    this->window = NULL;
    this->renderer = NULL;
    if (this->display) {
        // Window setup.
        SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
        SDL_CreateWindowAndRenderer(this->windowWidthPix, this->windowHeightPix, 0, &this->window, &this->renderer);
        SDL_SetWindowTitle(window, __FILE__);
    }
    this->init();
}

World::~World() {
    if (this->display) {
        this->destroyTextures();
        SDL_DestroyRenderer(this->renderer);
        SDL_DestroyWindow(this->window);
        SDL_Quit();
    }
}

void World::draw() const {
    if (this->display) {
        SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 0);
        SDL_RenderClear(this->renderer);
        auto it = this->objects.begin();
        int dx, dy;
        if (this->showFov()) {
            auto const& object = *(this->objects.find(this->showFovId)->second);
            auto cameraX = object.getX() - (object.getFov() / 2);
            auto cameraY = object.getY() - (object.getFov() / 2);
            while (this->findNextObjectInFov<decltype(it)>(it, object, dx, dy)) {
                auto const& otherObject = *(it->second);
                otherObject.draw(*this, cameraX, cameraY);
                it++;
            }
        } else {
            for (auto const& pair : this->objects) {
                pair.second->draw(*this);
            }
        }
        SDL_RenderPresent(this->renderer);
    }
}

void World::destroyTextures() {
    for (auto& texture : this->textures) {
        SDL_DestroyTexture(texture);
    }
    this->textures.clear();
}

void World::init() {
    // Randomness.
    if (!this->fixedRandomSeed) {
        this->randomSeed = std::time(NULL);
    }
    std::srand(this->randomSeed);

    this->nHumanActions = 0;
    unsigned int fov = 5;
    //unsigned int fov = std::min(this->getWidth(), this->getHeight()) / 2;

    // Setup textures. Depends on fov if we are watching an object.
    if (this->display) {
        unsigned int fovWidth = 0;
        unsigned int fovHeight = 0;
        if (this->showFov()) {
            fovWidth = fov;
            fovHeight = fovWidth;
            this->viewHeight = fovHeight;
        } else {
            fovWidth = this->width;
            fovHeight = this->height;
            this->viewHeight = this->height;
        }
        this->tileWidthPix = windowWidthPix / fovWidth;
        this->tileHeightPix = windowHeightPix / fovHeight;
        createSolidTexture(COLOR_MAX, 0, 0, 0);
        createSolidTexture(0, COLOR_MAX, 0, 0);
        createSolidTexture(0, 0, COLOR_MAX, 0);
        createSolidTexture(COLOR_MAX, COLOR_MAX, 0, 0);
        createSolidTexture(COLOR_MAX, 0, COLOR_MAX, 0);
        createSolidTexture(0, COLOR_MAX, COLOR_MAX, 0);
        createSolidTexture(COLOR_MAX, COLOR_MAX, COLOR_MAX, 0);
        createSolidTexture(COLOR_MAX / 2, COLOR_MAX / 2, COLOR_MAX / 2, 0);
        createSolidTexture(0, COLOR_MAX / 2, COLOR_MAX, 0);
    }

    if (this->scenario == "plants") {
        this->createSingleTextureObject(
            this->getWidth() / 2,
            this->getHeight() / 2,
            Object::Type::PLANT_EATER,
            std::make_unique<HumanActor>(),
            fov,
            0
        );
        this->createSingleTextureObject(
            this->getWidth() / 2,
            this->getHeight() / 2,
            Object::Type::PLANT_EATER,
            std::make_unique<HumanActor>(),
            fov,
            1
        );

        // Walls closing off the scenario borders. .
        for (unsigned int y = 0; y < this->height; ++y) {
            this->createSingleTextureObject(0, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, 1);
            this->createSingleTextureObject(this->width - 1, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, 2);
        }
        for (unsigned int x = 0; x < this->width; ++x) {
            this->createSingleTextureObject(x, 0, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, 1);
            this->createSingleTextureObject(x, this->height - 1, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, 2);
        }

        // Randomly placed food in the center.
        for (unsigned int y = 1; y < this->height - 1; ++y) {
            for (unsigned int x = 1; x < this->width - 1; ++x) {
                if (std::rand() % 3 == 0 && this->isTileEmpty(x, y)) {
                    this->createSingleTextureObject(x, y, Object::Type::PLANT, std::make_unique<DoNothingActor>(), 0, 3);
                }
            }
        }
    } else {
        // Place objects.
        this->createSingleTextureObject(
            this->getWidth() / 2,
            this->getHeight() / 2,
            Object::Type::HUMAN,
            std::make_unique<HumanActor>(),
            fov,
            0
        );
        if (this->multiHumanPlayer) {
            this->createSingleTextureObject(
                (this->getWidth() / 2) + 1,
                (this->getHeight() / 2) + 1,
                Object::Type::HUMAN,
                std::make_unique<HumanActor>(),
                fov,
                1
            );
        }
        this->createSingleTextureObject(
            this->getWidth() / 4,
            this->getHeight() / 4,
            Object::Type::RANDOM,
            std::make_unique<RandomActor>(),
            0,
            2
        );
        this->createSingleTextureObject(
            3 * this->getWidth() / 4,
            this->getHeight() / 4,
            Object::Type::FOLLOW_HUMAN,
            std::make_unique<FollowHumanActor>(),
            fov,
            3
        );
        this->createSingleTextureObject(
            3 * this->getWidth() / 4,
            3 * this->getHeight() / 4,
            Object::Type::FLEE_HUMAN,
            std::make_unique<FleeHumanActor>(),
            fov,
            4
        );
        this->createSingleTextureObject(
            this->getWidth() / 4,
            3 * this->getHeight() / 4,
            Object::Type::DO_NOTHING,
            std::make_unique<DoNothingActor>(),
            0,
            5
        );
        for (unsigned int y = 0; y < this->height; ++y) {
            for (unsigned int x = 0; x < this->width; ++x) {
                unsigned int sum = x + y;
                if (sum % 5 == 0) {
                    this->createSingleTextureObject(x, y, Object::Type::MOVE_UP, std::make_unique<MoveUpActor>(), fov, 6);
                } else if (sum % 7 == 0) {
                    this->createSingleTextureObject(x, y, Object::Type::MOVE_DOWN, std::make_unique<MoveDownActor>(), fov, 7);
                }
            }
        }
    }
}

// Reset to initial state.
// Resets everything, except the main window which stays open.
void World::reset() {
    this->objects.clear();
    this->destroyTextures();
    this->init();
}

void World::update(const std::vector<std::unique_ptr<Action>>& humanActions) {
    auto humanActionsIt = humanActions.begin();
    for (auto &pair : this->objects) {
        auto& object = *(pair.second);
        Action action;
        auto& actor = object.getActor();
        if (actor.takesHumanAction()) {
            action = **humanActionsIt;
            humanActionsIt++;
        } else {
            action = actor.act(*createWorldView(object));
        }

        auto x = object.getX();
        auto y = object.getY();

        // X
        if (action.getMoveX() == Action::MoveX::LEFT) {
            if (x > 0) {
                x--;
            }
        } else if (action.getMoveX() == Action::MoveX::RIGHT) {
            if (x < this->getWidth() - 1) {
                x++;
            }
        }

        // Y
        if (action.getMoveY() == Action::MoveY::UP) {
            if (y < this->getHeight() - 1) {
                y++;
            }
        } else if (action.getMoveY() == Action::MoveY::DOWN) {
            if (y > 0) {
                y--;
            }
        }

        auto it = this->objects.begin();
        bool tileNonEmpty = this->findObjectAtTile(it, x, y);
        auto& objectAtTarget = *(it->second);
        bool shouldMove = false;
        if (tileNonEmpty) {
            auto objectType = object.getType();
            auto targetType = objectAtTarget.getType();
            if (objectType == Object::Type::PLANT_EATER && targetType == Object::Type::PLANT) {
                shouldMove = true;
                object.setScore(object.getScore() + 1);
                std::cout << object.getScore() << std::endl;
                this->objects.erase(it);
            }
        } else {
            shouldMove = true;
        }
        if (shouldMove) {
            object.setX(x);
            object.setY(y);
        }
    }
}

unsigned int World::getHeight() const { return this->height; }
unsigned int World::getNHumanActions() const { return this->nHumanActions; }

SDL_Renderer * World::getRenderer() const { return this->renderer; }
unsigned int World::getTileHeightPix() const { return this->tileHeightPix; }
unsigned int World::getTileWidthPix() const { return this->tileWidthPix; }
unsigned int World::getWidth() const { return this->width; }
unsigned int World::getViewHeight() const { return this->viewHeight; }

bool World::showFov() const { return this->showFovId >= 0; }

SDL_Texture * World::createSolidTexture(unsigned int r, unsigned int g, unsigned int b, unsigned int a) {
    int pitch = 0;
    void *pixels = NULL;
    SDL_Texture *texture = SDL_CreateTexture(this->renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, this->tileWidthPix, this->tileHeightPix);
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    unsigned long TILE_AREA = this->tileHeightPix * this->tileWidthPix;
    for (unsigned long idx = 0; idx < TILE_AREA; ++idx) {
        Uint8 *base = &((Uint8 *)pixels)[idx * N_COLOR_CHANNELS];
        *(base + 0) = b;
        *(base + 1) = g;
        *(base + 2) = r;
        *(base + 3) = a;
    }
    SDL_UnlockTexture(texture);
    textures.push_back(texture);
    return texture;
}

template<typename ITERATOR>
bool World::findNextObjectInFov(objects_t::const_iterator& it, const Object& object, int& dx, int& dy) const {
    return this->findNextObjectInRectangle<ITERATOR>(
        it,
        object.getX(),
        object.getY(),
        object.getFov(),
        object.getFov(),
        dx,
        dy
    );
}

template<typename ITERATOR>
bool World::findNextObjectInRectangle(
    ITERATOR& it,
    unsigned int centerX,
    unsigned int centerY,
    unsigned int width,
    unsigned int height,
    int& dx,
    int& dy
) const {
    // TODO: use quadtree
    auto const end = this->objects.end();
    while (it != end) {
        auto const& object = *(it->second);
        dx = (int)object.getX() - (int)centerX;
        dy = (int)object.getY() - (int)centerY;
        if (std::abs(dx) < (int)width && std::abs(dy) < (int)height) {
            return true;
        }
        it++;
    }
    return false;
}

template<typename ITERATOR>
bool World::findObjectAtTile(ITERATOR& it, unsigned int x, unsigned int y) const {
    int dx, dy;
    return this->findNextObjectInRectangle<ITERATOR>(it, x, y, 1, 1, dx, dy);
}

bool World::isTileEmpty(unsigned int x, unsigned int y) const {
    auto it = this->objects.begin();
    return !this->findObjectAtTile<decltype(it)>(it, x, y);
}

std::unique_ptr<WorldView> World::createWorldView(const Object &object) const {
    auto objectViews = std::make_unique<std::vector<std::unique_ptr<ObjectView>>>();
    auto it = this->objects.begin();
    int dx, dy;
    while (this->findNextObjectInFov<decltype(it)>(it, object, dx, dy)) {
        auto const& otherObject = *(it->second);
        objectViews->push_back(std::make_unique<ObjectView>(
            dx, dy, otherObject.getType()
        ));
        it++;
    }
    return std::make_unique<WorldView>(object.getFov(), object.getFov(), std::move(objectViews), object.getScore());
}

void World::createSingleTextureObject(
    unsigned int x,
    unsigned int y,
    Object::Type type,
    std::unique_ptr<Actor> actor,
    unsigned int fov,
    size_t textureId
) {
    std::unique_ptr<DrawableObject> drawableObject;
    if (this->display) {
        drawableObject = std::make_unique<SingleTextureDrawableObject>(this->textures[textureId]);
    } else {
        drawableObject = std::make_unique<DoNothingDrawableObject>();
    }
    if (actor->takesHumanAction()) {
        this->nHumanActions++;
    }
    auto object = std::make_unique<Object>(x, y, type, std::move(actor), fov, std::move(drawableObject));
    auto rbegin = this->objects.rbegin();
    objects_t::key_type id;
    if (rbegin != this->objects.rend()) {
        id = this->objects.rbegin()->first + 1;
    } else {
        id = 0;
    }
    this->objects.emplace(id, std::move(object));
}
