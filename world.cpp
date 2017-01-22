#include <cstdlib>
#include <chrono>
#include <iomanip> // setprecision
#include <set>
#include <sstream>
#include <utility> // declval

#include <SDL2/SDL_image.h>
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
#include "plant_object.hpp"
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
    bool spawn,
    unsigned int windowPosX,
    unsigned int windowPosY,
    bool windowPosGiven
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
        SDL_SetWindowTitle(window, "General Game AI");
        if (windowPosGiven) {
            SDL_SetWindowPosition(this->window, windowPosX, windowPosY);
        }

        // TTF.
        TTF_Init();
        this->font = TTF_OpenFont("FreeSans.ttf", 24);
        if (this->font == NULL) {
            fprintf(stderr, "error: font not found\n");
            exit(EXIT_FAILURE);
        }
        auto margin = 0.05 * this->HUD_WIDTH_PIX;
        this->hud_text_x = this->windowWidthPix + margin;
        this->menu_text_x = margin;
        this->text_margin_y = margin;

        // Image.
        IMG_Init(IMG_INIT_PNG);
    }
    this->init(randomSeedGiven);
}

World::~World() {
    if (this->display) {
        this->destroyTextures();
        TTF_Quit();
        IMG_Quit();
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
        auto const& showObjectScore = worldView->getScore();
        if (this->menuMode) {
            std::map<
                decltype(std::declval<Object>().getScore()),
                std::set<const Object*>
            > m;
            auto type = showObject.getType();
            for (const auto &object : this->objects) {
                if (object->getType() == type) {
                    m[object->getScore()].insert(object.get());
                }
            }
            SDL_Rect rect;
            rect.y = this->text_margin_y;
            rect.h = 0;
            SDL_Color color{World::COLOR_MAX, World::COLOR_MAX, World::COLOR_MAX, World::COLOR_MAX};
            World::renderText(this->renderer, this->menu_text_x, rect.y + rect.h, "PAUSE", this->font, &rect, &color);
            World::renderText(this->renderer, this->menu_text_x, rect.y + rect.h, " ", this->font, &rect, &color);

            // Show current player rank.
            unsigned int count = 0;
            for (auto it = m.rbegin(), end = m.rend(); it != end; ++it) {
                const auto& pair = *it;
                const auto& score = pair.first;
                const auto& objects = pair.second;
                if (score == showObjectScore)
                    break;
                count += objects.size();
            }
            World::renderText(
                this->renderer,
                this->menu_text_x,
                rect.y + rect.h,
                std::string("Your rank: #" + std::to_string(count + 1)).c_str(),
                this->font, &rect, &color
            );
            World::renderText(this->renderer, this->menu_text_x, rect.y + rect.h, " ", this->font, &rect, &color);

            // Show top scores of the same type as the current player.
            count = 0;
            World::renderText(this->renderer, this->menu_text_x, rect.y + rect.h, "WINNERS", this->font, &rect, &color);
            World::renderText(this->renderer, this->menu_text_x, rect.y + rect.h, "rank id score type", this->font, &rect, &color);
            for (auto it = m.rbegin(), end = m.rend(); it != end; ++it) {
                const auto& pair = *it;
                const auto& score = pair.first;
                const auto& objects = pair.second;
                for (const auto& object : objects) {
                    World::renderText(
                        this->renderer,
                        this->menu_text_x,
                        rect.y + rect.h,
                        (
                            std::to_string(count + 1) + " " +
                            std::to_string(object->getId()) + " " +
                            std::to_string(score) + " " +
                            object->getActor().getTypeStr()
                        ).c_str(),
                        this->font,
                        &rect,
                        &color
                    );
                    count++;
                    if (count == 5)
                        goto scorePrintEnd;
                }
            }
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
        scorePrintEnd:

        // HUD.
        {
            SDL_Rect rect;

            // Separator line.
            SDL_SetRenderDrawColor(renderer, World::COLOR_MAX, World::COLOR_MAX, World::COLOR_MAX, World::COLOR_MAX);
            rect.x = this->windowWidthPix;
            rect.y = 0;
            rect.w = 1;
            rect.h = this->windowHeightPix;
            SDL_RenderFillRect(this->renderer, &rect);

            // Text.
            rect.y = this->text_margin_y;
            rect.h = 0;
            SDL_Color color{World::COLOR_MAX, World::COLOR_MAX, World::COLOR_MAX, World::COLOR_MAX};
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, "Score", this->font, &rect, &color);
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, std::to_string(showObjectScore).c_str(), this->font, &rect, &color);
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, "Time", this->font, &rect, &color);
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

            // Seed.
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, "Seed", this->font, &rect, &color);
            World::renderText(this->renderer, this->hud_text_x, rect.y + rect.h, std::to_string(this->randomSeed).c_str(), this->font, &rect, &color);

            SDL_RenderPresent(this->renderer);
        }
    }
}

void World::destroyTextures() {
    for (auto& pair : this->textures) {
        SDL_DestroyTexture(pair.second);
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
    this->skipUpdateOnce = false;
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
        //this->createSolidTexture("plant", 0.0, 1.0, 0.0);
        //this->createImageTexture("plant", "apple.png");
        this->createImageTextureBlitColor("eater", "rabbit.png", 1.0, 1.0, 1.0);
        this->createImageTextureBlitColor("human", "rabbit.png", 0.0, 0.0, 1.0);
        this->createImageTextureBlitColor("plant", "apple.png", 1.0, 0.0, 0.0);
        this->createImageTextureBlitColor("wall", "wall.png", 0.5, 0.5, 0.5);
        this->createImageTextureBlitColor("bad_plant", "apple.png", 0.6, 0.4, 0.1);
        this->createImageTextureBlitColor("great_plant", "apple.png", 1.0, 1.0, 0.0);
    }

    if (this->scenario == "empty") {
    } else if (this->scenario == "human") {
        this->createSingleTextureObject(
            std::make_unique<Object>(
                10,
                10,
                Object::Type::PLANT_EATER,
                std::make_unique<HumanActor>(),
                fov
            ),
            "human"
        );
    } else if (this->scenario == "wall") {
        this->createSingleTextureObject(
            std::make_unique<Object>(
                10,
                10,
                Object::Type::PLANT_EATER,
                std::make_unique<HumanActor>(),
                fov
            ),
            "human"
        );
        this->createSingleTextureObject(
            std::make_unique<Object>(
                10,
                11,
                Object::Type::WALL,
                std::make_unique<DoNothingActor>(),
                0
            ),
            "wall"
        );
    } else if (this->scenario == "plant") {
        this->createSingleTextureObject(
            std::make_unique<Object>(
                10,
                10,
                Object::Type::PLANT_EATER,
                std::make_unique<HumanActor>(),
                fov
            ),
            "human"
        );
        this->createSingleTextureObject(
            std::make_unique<PlantObject>(
                14,
                10,
                std::make_unique<DoNothingActor>()
            ),
            "plant"
        );
    } else {
        // Place human players.
        {
            decltype(this->nHumanPlayersInitial) totalPlayers = 0;
            while (totalPlayers < this->nHumanPlayersInitial) {
                unsigned int x = 1 + std::rand() % (this->width - 2);
                unsigned int y = 1 + std::rand() % (this->height - 2);
                if (this->isTileEmpty(x, y)) {
                    this->createSingleTextureObject(
                        std::make_unique<Object>(
                            x,
                            y,
                            Object::Type::PLANT_EATER,
                            std::make_unique<HumanActor>(),
                            fov
                        ),
                        "human"
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
                        std::make_unique<Object>(
                            x,
                            y,
                            Object::Type::PLANT_EATER,
                            std::make_unique<FollowTypeActor>(Object::Type::PLANT),
                            fov
                        ),
                        "eater"
                    );
                }
            }
        }

        // Random eaters.
        for (unsigned int y = 1; y < this->height - 1; ++y) {
            for (unsigned int x = 1; x < this->width - 1; ++x) {
                if (this->isTileEmpty(x, y) && (std::rand() % 100 == 0)) {
                    this->createSingleTextureObject(
                        std::make_unique<Object>(
                            x,
                            y,
                            Object::Type::PLANT_EATER,
                            std::make_unique<RandomActor>(),
                            fov
                        ),
                        "eater"
                    );
                }
            }
        }

        // Plants
        for (unsigned int y = 1; y < this->height - 1; ++y) {
            for (unsigned int x = 1; x < this->width - 1; ++x) {
                if (this->isTileEmpty(x, y)) {
                    if (std::rand() % 200 == 0) {
                        this->createSingleTextureObject(
                            std::make_unique<PlantObject>(x, y, std::make_unique<DoNothingActor>(), 5),
                            "great_plant"
                        );
                    } else if (std::rand() % 50 == 0) {
                        this->createSingleTextureObject(
                            std::make_unique<PlantObject>(x, y, std::make_unique<DoNothingActor>(), -1),
                            "bad_plant"
                        );
                    } else if (std::rand() % 5 == 0) {
                        this->createSingleTextureObject(
                            std::make_unique<PlantObject>(x, y, std::make_unique<DoNothingActor>()),
                            "plant"
                        );
                    }
                }
            }
        }

        // Walls closing off the scenario borders.
        for (unsigned int y = 0; y < this->height; ++y) {
            this->createSingleTextureObject(
                std::make_unique<Object>(0, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0),
                "wall"
            );
            this->createSingleTextureObject(
                std::make_unique<Object>(this->width - 1, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0),
                "wall"
            );
        }
        for (unsigned int x = 0; x < this->width; ++x) {
            this->createSingleTextureObject(
                std::make_unique<Object>(x, 0, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0),
                "wall"
            );
            this->createSingleTextureObject(
                std::make_unique<Object>(x, this->height - 1, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0),
                "wall"
            );
        }

        // Random walls.
        // These completely destroy robot players that just to straight to some random food, as they keep hitting that wall.
        // A* becomes necessary.
        for (unsigned int y = 1; y < this->height - 1; ++y) {
            for (unsigned int x = 1; x < this->width - 1; ++x) {
                if (this->isTileEmpty(x, y) && (std::rand() % 5 == 0)) {
                    this->createSingleTextureObject(
                        std::make_unique<Object>(x, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0),
                        "wall"
                    );
                }
            }
        }
    }
}

void World::pause() {
    this->skipUpdateOnce = this->menuMode;
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
    if (!this->menuMode) {
        if (this->skipUpdateOnce) {
            this->skipUpdateOnce = false;
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
                        // TODO: avoid renaming here. But then rtree.remove fails.
                        PlantObject *plantObjectAtTarget = static_cast<PlantObject*>(objectAtTarget);
                        shouldMove = true;
                        object->setScore(object->getScore() + plantObjectAtTarget->getValue());
                        this->deleteObject(objectAtTarget);
                    }
                } else {
                    shouldMove = true;
                }
                if (shouldMove) {
                    this->updatePosition(*object, x, y);
                }
            }
        }

        // Spawn new objects randomly.
        if (this->spawn) {
            // Plants
            for (unsigned int y = 1; y < this->height - 1; ++y) {
                for (unsigned int x = 1; x < this->width - 1; ++x) {
                    if (this->isTileEmpty(x, y) && (std::rand() % 200 == 0)) {
                        this->createSingleTextureObject(
                            std::make_unique<PlantObject>(x, y, std::make_unique<DoNothingActor>()),
                            "plant"
                        );
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

void World::createImageTexture(
    std::string id,
    std::string path
) {
    this->textures.emplace(id, IMG_LoadTexture(renderer, path.c_str()));
}

SDL_Surface* World::createSolidSurface(
    double r,
    double g,
    double b,
    double a
) {
    SDL_Surface *surface;
    surface = SDL_CreateRGBSurface(0, this->tileWidthPix, this->tileHeightPix, 32, 0, 0, 0, 0);
    SDL_LockSurface(surface);
    void *pixels = surface->pixels;
    unsigned long tile_area = this->tileHeightPix * this->tileWidthPix;
    for (unsigned long idx = 0; idx < tile_area; ++idx) {
        Uint8 *base = &((Uint8 *)pixels)[idx * N_COLOR_CHANNELS];
        *(base + 0) = b * World::COLOR_MAX;
        *(base + 1) = g * World::COLOR_MAX;
        *(base + 2) = r * World::COLOR_MAX;
        *(base + 3) = a * World::COLOR_MAX;
    }
    SDL_UnlockSurface(surface);
    return surface;
}

void World::createImageTextureBlitColor(
    std::string id,
    std::string path,
    double r,
    double g,
    double b,
    double a
) {
    SDL_Surface *src, *dst;
    SDL_Texture *texture;
    src = IMG_Load(path.c_str());
    dst = this->createSolidSurface(r, g, b, a);
    SDL_BlitScaled(src, NULL, dst, NULL);

    // TODO make outter part (originally opaque)
    // transparent to show actual background.
    // Difficulty: must first scale down input image.
    //SDL_LockSurface(src);
    //void *pixels = src->pixels;
    //unsigned long tile_area = this->tileHeightPix * this->tileWidthPix;
    //for (unsigned long idx = 0; idx < tile_area; ++idx) {
        //Uint8 *base = &((Uint8 *)pixels)[idx * N_COLOR_CHANNELS + 3];
        //*base = World::COLOR_MAX - *base;
    //}
    //SDL_UnlockSurface(src);

    texture = SDL_CreateTextureFromSurface(this->renderer, dst);
    SDL_FreeSurface(src);
    SDL_FreeSurface(dst);
    this->textures.emplace(id, texture);
}

void World::createSolidTexture(
    std::string id,
    double r,
    double g,
    double b,
    double a
) {
    SDL_Surface *surface;
    SDL_Texture *texture;
    surface = this->createSolidSurface(r, g, b, a);
    texture = SDL_CreateTextureFromSurface(this->renderer, surface);
    SDL_FreeSurface(surface);
    this->textures.emplace(id, texture);
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
            std::set<const Object*>
        >
    > m;
    for (const auto &object : this->objects) {
        m[object->getType()][object->getScore()].insert(object.get());
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
            for (const auto &object : ids) {
                std::cout
                    << type << " "
                    << object->getId() << " "
                    << score << " "
                    << object->getActor().getTypeStr()
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

void World::addObject(std::unique_ptr<Object>&& object) {
    this->rtree.insert(object.get());
    this->objects.insert(std::move(object));
}

void World::createSingleTextureObject(
    std::unique_ptr<Object>&& object,
    std::string textureId
) {
    object->setId(this->getNextFreeObjectId());

    // Set drawable object.
    std::unique_ptr<DrawableObject> drawableObject;
    if (this->display) {
        auto it = this->textures.find(textureId);
        assert(it != this->textures.end());
        drawableObject = std::make_unique<SingleTextureDrawableObject>(it->second);
    } else {
        drawableObject = std::make_unique<DoNothingDrawableObject>();
    }
    object->setDrawableObject(std::move(drawableObject));

    if (object->getActor().takesHumanAction()) {
        this->nHumanActions++;
    }
    this->addObject(std::move(object));
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
