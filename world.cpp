#include <cstdlib>
#include <chrono>
#include <iomanip> // setprecision
#include <set>
#include <sstream>
#include <utility> // declval

#include <SDL2/SDL_ttf.h>

#include "action.hpp"
#include "actor.hpp"
#include "actor.hpp"
#include "do_nothing_actor.hpp"
#include "do_nothing_drawable_object.hpp"
#include "drawable_object.hpp"
#include "flee_type_actor.hpp"
#include "follow_type_actor.hpp"
#include "human_actor.hpp"
#include "move_down_actor.hpp"
#include "move_up_actor.hpp"
#include "object.hpp"
#include "object_view.hpp"
#include "random_actor.hpp"
#include "single_texture_drawable_object.hpp"
#include "utils.hpp"
#include "world.hpp"
#include "world_view.hpp"

World::World(
    unsigned int width,
    unsigned int height,
    bool display,
    unsigned int windowWidthPix,
    unsigned int windowHeightPix,
    unsigned int showPlayerId,
    bool showFov,
    bool randomSeedGiven,
    int randomSeed,
    unsigned int nHumanPlayers,
    std::string scenario,
    int timeLimit,
    bool verbose,
    bool spawn
) :
    display(display),
    showFov(showFov),
    spawn(spawn),
    verbose(verbose),
    timeLimit(timeLimit),
    scenario(std::move(scenario)),
    height(height),
    nHumanPlayersInitial(nHumanPlayers),
    randomSeed(randomSeed),
    showPlayerId(showPlayerId),
    width(width),
    windowHeightPix(windowHeightPix),
    windowWidthPix(windowWidthPix)
{
    this->window = NULL;
    this->renderer = NULL;
    if (this->display) {
        // Window setup.
        SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
        SDL_CreateWindowAndRenderer(
            this->windowWidthPix + this->HUD_WIDTH_PIX,
            this->windowHeightPix,
            0,
            &this->window,
            &this->renderer
        );
        SDL_SetWindowTitle(window, __FILE__);

        // TTF.
        TTF_Init();
        this->font = TTF_OpenFont("FreeSans.ttf", 24);
        if (this->font == NULL) {
            fprintf(stderr, "error: font not found\n");
            exit(EXIT_FAILURE);
        }
        this->hud_text_x = this->windowWidthPix + 0.05 * this->HUD_WIDTH_PIX;
    }
    this->init(randomSeedGiven);
}

World::~World() {
    if (this->display) {
        this->destroyTextures();
        TTF_Quit();
        SDL_DestroyRenderer(this->renderer);
        SDL_DestroyWindow(this->window);
        SDL_Quit();
    }
}

/*
- x, y: upper left corner of string
- rect output Width and height contain rendered dimensions.
*/
void World::renderText(
    SDL_Renderer *renderer,
    int x,
    int y,
    const char *text,
    TTF_Font *font,
    SDL_Rect *rect,
    SDL_Color *color
) {
    SDL_Surface *surface;
    SDL_Texture *texture;

    surface = TTF_RenderText_Solid(font, text, *color);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect->x = x;
    rect->y = y;
    rect->w = surface->w;
    rect->h = surface->h;
    /* This is wasteful for textures that stay the same.
     * But makes things less stateful and easier to usec
     * Not going to code an atlas solution here... are we? */
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, rect);
    SDL_DestroyTexture(texture);
}

void World::draw() const {
    if (this->display) {
        SDL_SetRenderDrawColor(this->renderer, 0, 0, 0, 0);
        SDL_RenderClear(this->renderer);
        auto const& showObject = **(this->objects.find(this->showPlayerId));
        auto worldView = createWorldView(showObject);
        if (this->menuMode) {
        } else {
            if (this->getShowFov()) {
                auto cameraX = showObject.getX() - showObject.getFov() + 1;
                auto cameraY = showObject.getY() - showObject.getFov() + 1;
                // TODO: loop over existing world view here,
                // to keep as close as possible to actual observable state and code drier.
                auto it = this->queryObjectsInFov(showObject);
                auto end = this->rtree.qend();
                while (it != end) {
                    (*it)->draw(*this, cameraX, cameraY);
                    ++it;
                }
            } else {
                for (auto const& object : this->objects) {
                    object->draw(*this);
                }
            }
        }

        // HUD.
        {
            SDL_Rect rect;

            // Separator.
            SDL_SetRenderDrawColor(renderer, World::COLOR_MAX, World::COLOR_MAX, World::COLOR_MAX, World::COLOR_MAX);
            rect.x = this->windowWidthPix;
            rect.y = 0;
            rect.w = 1;
            rect.h = this->windowHeightPix;
            SDL_RenderFillRect(this->renderer, &rect);

            // Text.
            rect.y = 0;
            rect.h = 0;
            SDL_Color color{World::COLOR_MAX, World::COLOR_MAX, World::COLOR_MAX, World::COLOR_MAX};
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, "score", this->font, &rect, &color);
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, std::to_string(worldView->getScore()).c_str(), this->font, &rect, &color);
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, "time", this->font, &rect, &color);
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, std::to_string(this->ticks).c_str(), this->font, &rect, &color);

            // FPS.
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, "FPS", this->font, &rect, &color);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(3) << utils::fps::get();
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, ss.str().c_str(), this->font, &rect, &color);

            // x, y
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, "x, y", this->font, &rect, &color);
            World::renderText(
                this->renderer,
                this->hud_text_x,
                rect.y + rect.h,
                (
                    std::to_string(showObject.getX()) + ", " +
                    std::to_string(showObject.getY())
                ).c_str(),
                this->font,
                &rect,
                &color
            );

            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, "seed", this->font, &rect, &color);
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, std::to_string(this->randomSeed).c_str(), this->font, &rect, &color);

            SDL_RenderPresent(this->renderer);
        }
    }
}

void World::destroyTextures() {
    for (auto& texture : this->textures) {
        SDL_DestroyTexture(texture);
    }
    this->textures.clear();
}

void World::init(bool reuseRandomSeed) {
    // Randomness.
    if (!reuseRandomSeed) {
        this->randomSeed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    std::srand(this->randomSeed);
    if (this->verbose) {
        std::cout << "randomSeed " << randomSeed << '\n';
    }

    if (this->needFpsUpdate()) {
        utils::fps::init();
    }
    this->menuMode = false;
    this->timeout = false;
    this->ticks = 0;
    this->nHumanActions = 0;
    unsigned int fov = 5;

    // Setup textures. Depends on fov if we are watching an object.
    if (this->display) {
        unsigned int fovWidth = 0;
        unsigned int fovHeight = 0;
        if (this->getShowFov()) {
            fovWidth = 2 * fov - 1;
            fovHeight = fovWidth;
            this->viewHeight = fovHeight;
        } else {
            fovWidth = this->width;
            fovHeight = this->height;
            this->viewHeight = this->height;
        }
        this->tileWidthPix = this->windowWidthPix / fovWidth;
        this->tileHeightPix = this->windowHeightPix / fovHeight;
        createSolidTexture(World::COLOR_MAX, 0, 0, 0);
        createSolidTexture(0, World::COLOR_MAX, 0, 0);
        createSolidTexture(0, 0, World::COLOR_MAX, 0);
        createSolidTexture(World::COLOR_MAX, World::COLOR_MAX, 0, 0);
        createSolidTexture(World::COLOR_MAX, 0, World::COLOR_MAX, 0);
        createSolidTexture(0, World::COLOR_MAX, World::COLOR_MAX, 0);
        createSolidTexture(World::COLOR_MAX, World::COLOR_MAX, World::COLOR_MAX, 0);
        createSolidTexture(World::COLOR_MAX / 2, World::COLOR_MAX / 2, World::COLOR_MAX / 2, 0);
        createSolidTexture(0, World::COLOR_MAX / 2, World::COLOR_MAX, 0);
    }

    if (this->scenario == "debug") {
        // Place objects.
        this->createSingleTextureObject(
            this->getWidth() / 2,
            this->getHeight() / 2,
            Object::Type::HUMAN,
            std::make_unique<HumanActor>(),
            fov,
            0
        );
        // TODO use nHumanPlayers
        //if (this->multiHumanPlayer) {
            //this->createSingleTextureObject(
                //(this->getWidth() / 2) + 1,
                //(this->getHeight() / 2) + 1,
                //Object::Type::HUMAN,
                //std::make_unique<HumanActor>(),
                //fov,
                //1
            //);
        //}
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
            std::make_unique<FollowTypeActor>(),
            fov,
            3
        );
        this->createSingleTextureObject(
            3 * this->getWidth() / 4,
            3 * this->getHeight() / 4,
            Object::Type::FLEE_HUMAN,
            std::make_unique<FleeTypeActor>(),
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
    } else if (this->scenario == "empty") {
    } else if (this->scenario == "human") {
        this->createSingleTextureObject(
            10,
            10,
            Object::Type::PLANT_EATER,
            std::make_unique<HumanActor>(),
            fov,
            0
        );
    } else if (this->scenario == "wall") {
        this->createSingleTextureObject(
            10,
            10,
            Object::Type::PLANT_EATER,
            std::make_unique<HumanActor>(),
            fov,
            0
        );
        this->createSingleTextureObject(
            10,
            11,
            Object::Type::WALL,
            std::make_unique<DoNothingActor>(),
            0,
            2
        );
    } else if (this->scenario == "plant") {
        this->createSingleTextureObject(
            10,
            10,
            Object::Type::PLANT_EATER,
            std::make_unique<HumanActor>(),
            fov,
            0
        );
        this->createSingleTextureObject(
            14,
            10,
            Object::Type::PLANT,
            std::make_unique<DoNothingActor>(),
            0,
            3
        );
    } else if (this->scenario == "plants") {
        this->createSingleTextureObject(
            10,
            10,
            Object::Type::PLANT_EATER,
            //std::make_unique<HumanActor>(),
            //std::make_unique<RandomActor>(),
            std::make_unique<FollowTypeActor>(Object::Type::PLANT),
            fov,
            0
        );
        //this->createSingleTextureObject(
             //9,
            //10,
            //Object::Type::WALL,
            //std::make_unique<DoNothingActor>(),
            //0,
            //2
        //);
        //this->createSingleTextureObject(
            //14,
            //10,
            //Object::Type::PLANT,
            //std::make_unique<DoNothingActor>(),
            //0,
            //3
        //);
        //this->createSingleTextureObject(
            //0,
            //0,
            //Object::Type::WALL,
            //std::make_unique<DoNothingActor>(),
            //0,
            //2
        //);
    } else {
        // Place human players.
        {
            decltype(this->nHumanPlayersInitial) totalPlayers = 0;
            while (totalPlayers < this->nHumanPlayersInitial) {
                unsigned int x = 1 + std::rand() % (this->width - 2);
                unsigned int y = 1 + std::rand() % (this->height - 2);
                if (this->isTileEmpty(x, y)) {
                    this->createSingleTextureObject(
                        x,
                        y,
                        Object::Type::PLANT_EATER,
                        std::make_unique<HumanActor>(),
                        fov,
                        0
                    );
                    totalPlayers++;
                }
            }
        }

        // Eaters that follow food.
        for (unsigned int y = 1; y < this->height - 1; ++y) {
            for (unsigned int x = 1; x < this->width - 1; ++x) {
                if (this->isTileEmpty(x, y) && (std::rand() % 100 == 0)) {
                    this->createSingleTextureObject(
                        x,
                        y,
                        Object::Type::PLANT_EATER,
                        std::make_unique<FollowTypeActor>(Object::Type::PLANT),
                        fov,
                        4
                    );
                }
            }
        }

        // Random eaters.
        for (unsigned int y = 1; y < this->height - 1; ++y) {
            for (unsigned int x = 1; x < this->width - 1; ++x) {
                if (this->isTileEmpty(x, y) && (std::rand() % 100 == 0)) {
                    this->createSingleTextureObject(
                        x,
                        y,
                        Object::Type::PLANT_EATER,
                        std::make_unique<RandomActor>(),
                        fov,
                        1
                    );
                }
            }
        }

        // Plants
        for (unsigned int y = 1; y < this->height - 1; ++y) {
            for (unsigned int x = 1; x < this->width - 1; ++x) {
                if (this->isTileEmpty(x, y) && (std::rand() % 5 == 0)) {
                    this->createSingleTextureObject(x, y, Object::Type::PLANT, std::make_unique<DoNothingActor>(), 0, 3);
                }
            }
        }

        // Walls closing off the scenario borders.
        textures_t::size_type wall_texture = 2;
        for (unsigned int y = 0; y < this->height; ++y) {
            this->createSingleTextureObject(0, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, wall_texture);
            this->createSingleTextureObject(this->width - 1, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, wall_texture);
        }
        for (unsigned int x = 0; x < this->width; ++x) {
            this->createSingleTextureObject(x, 0, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, wall_texture);
            this->createSingleTextureObject(x, this->height - 1, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, wall_texture);
        }

        // Random walls.
        // These completely destroy robot players that just to straight to some random food, as they keep hitting that wall.
        // A* becomes necessary.
        for (unsigned int y = 1; y < this->height - 1; ++y) {
            for (unsigned int x = 1; x < this->width - 1; ++x) {
                if (this->isTileEmpty(x, y) && (std::rand() % 20 == 0)) {
                    this->createSingleTextureObject(x, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0, wall_texture);
                }
            }
        }
    }
}

void World::pause() {
    this->menuMode = !this->menuMode;
}

void World::quit() {
    if (this->menuMode) {
        this->userQuit = true;
    } else {
        this->menuMode = true;
    }
}

// Reset to initial state.
// Resets everything, except the main window which stays open.
void World::reset(bool reuseRandomSeed) {
    this->rtree.clear();
    this->objects.clear();
    this->destroyTextures();
    this->init(reuseRandomSeed);
}

void World::update(const std::vector<std::unique_ptr<Action>>& humanActions) {
    if (this->menuMode) {
    } else {
        auto humanActionsIt = humanActions.begin();

        // Update existing objects.
        for (const auto &object : this->objects) {
            Action action;
            auto& actor = object->getActor();
            if (actor.takesHumanAction()) {
                action = **humanActionsIt;
                humanActionsIt++;
            } else {
                action = actor.act(*createWorldView(*object));
            }

            auto x = object->getX();
            auto y = object->getY();

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

            Object *objectAtTarget;
            bool tileNonEmpty = this->findObjectAtTile(&objectAtTarget, x, y);
            bool shouldMove = false;
            if (tileNonEmpty) {
                auto objectType = object->getType();
                auto targetType = objectAtTarget->getType();
                if (objectType == Object::Type::PLANT_EATER && targetType == Object::Type::PLANT) {
                    shouldMove = true;
                    object->setScore(object->getScore() + 1);
                    this->deleteObject(objectAtTarget);
                }
            } else {
                shouldMove = true;
            }
            if (shouldMove) {
                this->updatePosition(*object, x, y);
            }

        }

        // Spawn new objects randomly.
        if (this->spawn) {
            // Plants
            for (unsigned int y = 1; y < this->height - 1; ++y) {
                for (unsigned int x = 1; x < this->width - 1; ++x) {
                    if (this->isTileEmpty(x, y) && (std::rand() % 100 == 0)) {
                        this->createSingleTextureObject(x, y, Object::Type::PLANT, std::make_unique<DoNothingActor>(), 0, 3);
                    }
                }
            }
        }

        this->ticks++;
        if (this->ticks == (unsigned int)this->timeLimit) {
            this->timeout = true;
            this->menuMode = true;
        }
    }

    // FPS.
    if (this->needFpsUpdate()) {
        utils::fps::update();
    }
    if (this->verbose) {
        utils::fps::print();
    }
}

unsigned int World::getHeight() const { return this->height; }
unsigned int World::getNHumanActions() const { return this->nHumanActions; }

SDL_Renderer * World::getRenderer() const { return this->renderer; }
unsigned int World::getTileHeightPix() const { return this->tileHeightPix; }
unsigned int World::getTileWidthPix() const { return this->tileWidthPix; }
unsigned int World::getWidth() const { return this->width; }
unsigned int World::getViewHeight() const { return this->viewHeight; }

bool World::getShowFov() const { return this->showFov; }

bool World::needFpsUpdate() const {
    return this->verbose || this->display;
}

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

World::Rtree::const_query_iterator World::queryObjectsInFov(const Object& object) const {
    auto fov = object.getFov();
    return this->queryObjectsInRectangle(object.getX(), object.getY(), fov, fov);
}

World::Rtree::const_query_iterator World::queryObjectsInRectangle(
    unsigned int centerX,
    unsigned int centerY,
    unsigned int width,
    unsigned int height
) const {
    if (width == 0 || height == 0) {
        return rtree.qend();
    } else {
        width--;
        height--;
        return this->rtree.qbegin(bgi::intersects(QueryBox(
            QueryPoint(centerX - width, centerY - height),
            QueryPoint(centerX + width, centerY + height)
        )));
    }
}

bool World::findObjectAtTile(Object **object, unsigned int x, unsigned int y) const {
    auto it = this->queryObjectsInRectangle(x, y, 1, 1);
    if (it == this->rtree.qend()) {
        return false;
    } else {
        *object = *it;
        return true;
    }
}

bool World::isGameOver() const {
    return
        this->userQuit || (!this->display && this->timeout)
    ;
}

void World::printScores() const {
    std::map<
        Object::Type,
        std::map<
            decltype(std::declval<Object>().getScore()),
            std::set<std::pair<
                unsigned int,
                const Object*
            >>
        >
    > m;
    for (const auto &object : this->objects) {
        m[object->getType()][object->getScore()].insert(std::make_pair(object->getId(), object.get()));
    }

    if (this->verbose) {
        std::cout << std::endl;
        std::cout << "scores" << std::endl;
    }
    std::cout << "type id score actor" << std::endl;
    for (const auto &pair : m) {
        const auto &type = pair.first;
        const auto &scoresIds = pair.second;
        for (auto it = scoresIds.rbegin(), end = scoresIds.rend(); it != end; ++it) {
            const auto &score = it->first;
            const auto &ids = it->second;
            for (const auto &pair : ids) {
                const auto &id = pair.first;
                const auto &object = *(pair.second);
                std::cout
                    << type << " "
                    << id << " "
                    << score << " "
                    << object.getActor().getTypeStr()
                << std::endl;
            }
        }
    }
}

bool World::isTileEmpty(unsigned int x, unsigned int y) const {
    Object *object;
    return !this->findObjectAtTile(&object, x, y);
}

std::unique_ptr<WorldView> World::createWorldView(const Object &object) const {
    auto objectViews = std::make_unique<std::vector<std::unique_ptr<ObjectView>>>();
    auto it = this->queryObjectsInFov(object);
    auto end = this->rtree.qend();
    while (it != end) {
        auto const& otherObject = **it;
        objectViews->push_back(std::make_unique<ObjectView>(
            otherObject.getX() - object.getX(),
            otherObject.getY() - object.getY(),
            otherObject.getType()
        ));
        ++it;
    }
    return std::make_unique<WorldView>(object.getFov(), object.getFov(), std::move(objectViews), object.getScore());
}

void World::createObject(std::unique_ptr<Object> object) {
    this->rtree.insert(object.get());
    this->objects.insert(std::move(object));
}

void World::createSingleTextureObject(
    unsigned int x,
    unsigned int y,
    Object::Type type,
    std::unique_ptr<Actor> actor,
    unsigned int fov,
    textures_t::size_type textureId
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
    this->createObject(std::make_unique<Object>(
        x,
        y,
        type,
        std::move(actor),
        fov,
        std::move(drawableObject),
        this->getNextFreeObjectId()
    ));
}

void World::deleteObject(Object *object) {
    assert(this->rtree.remove(object) == 1);
    this->objects.erase(this->objects.find(object->getId()));
}
void World::updatePosition(Object& object, unsigned int x, unsigned int y) {
    this->rtree.remove(&object);
    object.setX(x);
    object.setY(y);
    this->rtree.insert(&object);
}

unsigned int World::getNextFreeObjectId() {
    auto rbegin = this->objects.rbegin();
    unsigned int id;
    if (rbegin != this->objects.rend()) {
        id = (*rbegin)->getId() + 1;
    } else {
        id = 0;
    }
    return id;
}

std::string World::toString(const World::Rtree& rtree) {
    std::stringstream ss;
    for (const auto& object : rtree) {
        ss << *object << std::endl;
    }
    return ss.str();
}
