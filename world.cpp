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
    width(width),
    height(height),
    display(display),
    windowWidthPix(windowWidthPix),
    windowHeightPix(windowHeightPix),
    showFovId(showFovId),
    fixedRandomSeed(fixedRandomSeed),
    randomSeed(randomSeed),
    multiHumanPlayer(multiHumanPlayer),
    scenario(std::move(scenario))
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
    int dx, dy;
    if (this->display) {
        Uint8 *base;
        SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 0);
        SDL_RenderClear(this->renderer);
        auto it = this->objects.begin();
        int dx, dy;
        if (this->showFov()) {
            auto const& object = *(this->objects[this->showFovId]);
            auto cameraX = object.getX() - (object.getFov() / 2);
            auto cameraY = object.getY() - (object.getFov() / 2);
            while (this->findNextObjectInFov(it, object, dx, dy)) {
                auto const& otherObject = **it;
                otherObject.draw(*this, cameraX, cameraY);
                it++;
            }
        } else {
            for (auto const& object : this->objects) {
                object->draw(*this);
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

    if (this->scenario == "food") {
        this->createSingleTextureObject(
            this->getWidth() / 2,
            this->getHeight() / 2,
            Object::Type::HUMAN,
            std::make_unique<HumanActor>(),
            fov,
            0
        );

        // Walls closing off the scenario borders. .
        for (unsigned int y = 0; y < this->height; ++y) {
            this->createSingleTextureObject(0, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, 1);
            this->createSingleTextureObject(this->width - 1, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, 1);
        }
        for (unsigned int x = 0; x < this->width; ++x) {
            this->createSingleTextureObject(x, 0, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, 1);
            this->createSingleTextureObject(x, this->height - 1, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, 1);
        }

        // Randomly placed food in the center.
        for (unsigned int y = 1; y < this->height - 1; ++y) {
            for (unsigned int x = 1; x < this->width - 1; ++x) {
                if (std::rand() % 3 == 0 && this->isTileEmpty(x, y)) {
                    this->createSingleTextureObject(x, y, Object::Type::FOOD, std::make_unique<DoNothingActor>(), 0, 2);
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
    for (auto &object : this->objects) {
        Action action;
        auto& actor = object->getActor();
        if (actor.takesHumanAction()) {
            action = **humanActionsIt;
            humanActionsIt++;
        } else {
            action = actor.act(*createWorldView(*object));
        }

        // X
        if (action.getMoveX() == Action::MoveX::LEFT) {
            auto x = object->getX();
            if (x > 0) {
                object->setX(x - 1);
            }
        } else if (action.getMoveX() == Action::MoveX::RIGHT) {
            auto x = object->getX();
            if (x < this->getWidth() - 1) {
                object->setX(x + 1);
            }
        }

        // Y
        if (action.getMoveY() == Action::MoveY::UP) {
            auto y = object->getY();
            if (y < this->getHeight() - 1) {
                object->setY(y + 1);
            }
        } else if (action.getMoveY() == Action::MoveY::DOWN) {
            auto y = object->getY();
            if (y > 0) {
                object->setY(y - 1);
            }
        }
    }
}

unsigned int World::getHeight() const { return this->height; }
unsigned int World::getNHumanActions() const { return this->nHumanActions; }

const std::vector<std::unique_ptr<Object>>& World::getObjects() const { return this->objects; }
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

bool World::findNextObjectInFov(std::vector<std::unique_ptr<Object>>::const_iterator& it, const Object& object, int& dx, int& dy) const {
    return this->findNextObjectInRectangle(
        it,
        object.getX(),
        object.getY(),
        object.getFov(),
        object.getFov(),
        dx,
        dy
    );
}

bool World::findNextObjectInRectangle(
    std::vector<std::unique_ptr<Object>>::const_iterator& it,
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
        auto const& object = **it;
        dx = (int)object.getX() - (int)centerX;
        dy = (int)object.getY() - (int)centerY;
        int fov = object.getFov();
        if (std::abs(dx) < (int)width && std::abs(dy) < (int)height) {
            return true;
        }
        it++;
    }
    return false;
}

bool World::isTileEmpty(unsigned int x, unsigned int y) const {
    auto it = this->objects.begin();
    int dx, dy;
    return !this->findNextObjectInRectangle(it, x, y, 1, 1, dx, dy);
}

std::unique_ptr<WorldView> World::createWorldView(const Object &object) const {
    auto objectViews = std::make_unique<std::vector<std::unique_ptr<ObjectView>>>();
    auto it = this->objects.begin();
    int dx, dy;
    while (this->findNextObjectInFov(it, object, dx, dy)) {
        auto const& otherObject = **it;
        objectViews->push_back(std::make_unique<ObjectView>(
            dx, dy, otherObject.getType()
        ));
        it++;
    }
    return std::make_unique<WorldView>(object.getFov(), object.getFov(), std::move(objectViews));
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
    this->objects.push_back(std::move(object));
}

