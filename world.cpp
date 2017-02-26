#include <cstdlib>
#include <chrono>
#include <functional> // function
#include <iomanip> // setprecision
#include <numeric> // partial_sum
#include <random> // random_device
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
#include "fruit_object.hpp"
#include "random_actor.hpp"
#include "single_texture_drawable_object.hpp"
#include "teleport_object.hpp"
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
    bool windowPosGiven,
    std::unique_ptr<std::map<std::string,std::string>> config
) :
    display(display),
    showFov(showFov),
    spawn(spawn),
    verbose(verbose),
    config(std::move(config)),
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
        this->randomSeed = std::random_device()();
    }
    this->prng.seed(this->randomSeed);
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
        this->createImageTextureBlitColor("eater", "rabbit.png", 1.0, 1.0, 1.0);
        this->createImageTextureBlitColor("human", "rabbit.png", 0.0, 0.0, 1.0);
        this->createImageTextureBlitColor("fruit", "apple.png", 1.0, 0.0, 0.0);
        this->createImageTextureBlitColor("wall", "wall.png", 0.5, 0.5, 0.5);
        this->createImageTextureBlitColor("bad_fruit", "apple.png", 0.6, 0.4, 0.1);
        this->createImageTextureBlitColor("great_fruit", "apple.png", 1.0, 1.0, 0.0);
        this->createImageTextureBlitColor("tree", "tree.png", 0.0, 1.0, 0.0);
        this->createImageTextureBlitColor("teleport", "teleport.png", 0.8, 0.0, 0.4);
    }

    if (this->scenario == "empty") {
    } else if (this->scenario == "human") {
        this->createSingleTextureObject(
            std::make_unique<Object>(
                10,
                10,
                Object::Type::FRUIT_EATER,
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
                Object::Type::FRUIT_EATER,
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
    } else if (this->scenario == "fruit") {
        this->createSingleTextureObject(
            std::make_unique<Object>(
                10,
                10,
                Object::Type::FRUIT_EATER,
                std::make_unique<HumanActor>(),
                fov
            ),
            "human"
        );
        this->createSingleTextureObject(
            std::make_unique<FruitObject>(14, 10),
            "fruit"
        );
    } else if (this->scenario == "teleport") {
        this->createSingleTextureObject(
            std::make_unique<Object>(
                10,
                10,
                Object::Type::FRUIT_EATER,
                std::make_unique<HumanActor>(),
                fov
            ),
            "human"
        );
        auto tp1 = std::make_unique<TeleportObject>(8, 10);
        auto tp2 = std::make_unique<TeleportObject>(12, 10, tp1.get());
        auto tp3 = std::make_unique<TeleportObject>(12, 9, tp1.get());
        tp1->setDestination(tp2.get());
        this->createSingleTextureObject(std::move(tp1), "teleport");
        this->createSingleTextureObject(std::move(tp2), "teleport");
        this->createSingleTextureObject(std::move(tp3), "teleport");
        this->createSingleTextureObject(std::make_unique<FruitObject>(13, 10), "fruit");
        this->createSingleTextureObject(std::make_unique<Object>(12, 11, Object::Type::WALL), "wall");
    } else {
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

        // Monuments are large pre-fabricated chunks (Rust terminology).
        // Two monuments cannot overlap, and some random elements cannot appear
        // inside monments, e.g. walls could block off entries.
        // TODO:find more elegant way to place them randomly:
        // http://stackoverflow.com/questions/716558/place-random-non-overlapping-rectangles-on-a-panel
        typedef bgm::point<int, 2, bg::cs::cartesian> MonumentPoint;
        typedef bgm::box<MonumentPoint> MonumentBox;
        typedef bgi::rtree<MonumentBox, bgi::linear<16>> MonumentRtree;
        MonumentRtree monumentRtree;

        // Retangular rooms with a single door entry.
        {
            auto wmin = 5u;
            auto hmin = 5u;
            auto density = 20u;
            if (wmin < this->width - 6 && wmin < this->height - 4) {
                auto placed = 0u;
                auto attempts = 0u;
                auto max_placed = this->width * this->height / (density * density);
                auto max_attempts = 2 * max_placed;
                while(placed < max_placed && attempts < max_attempts) {
                    auto w = wmin + this->randUint() % (this->width / density);
                    auto h = hmin + this->randUint() % (this->height / density);
                    auto x0 = 2 + this->randUint() % (this->width - w - 3);
                    auto y0 = 2 + this->randUint() % (this->height - h - 3);
                    auto monumentBox = MonumentBox(MonumentPoint(x0 - 1, y0 - 1), MonumentPoint(x0 + w + 1, y0 + h + 1));
                    auto it = monumentRtree.qbegin(bgi::intersects(monumentBox));
                    auto end = monumentRtree.qend();
                    if (it == end) {
                        auto xmax = x0 + w - 1;
                        auto ymax = y0 + h - 1;
                        auto door_side = this->randUint() % 4;
                        decltype(x0) door_pos = x0 + (this->randUint() % w);
                        if (door_side < 2) {
                            door_pos = y0 + this->randUint() % h;
                        } else {
                            door_pos = x0 + 1 + this->randUint() % (w - 2);
                        }
                        for (auto y = y0; y <= ymax; ++y) {
                            if (door_side != 0 || y != door_pos) {
                                this->createSingleTextureObject(
                                    std::make_unique<Object>(x0, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0),
                                    "wall"
                                );
                            }
                            if (door_side != 1 || y != door_pos) {
                                this->createSingleTextureObject(
                                    std::make_unique<Object>(xmax, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0),
                                    "wall"
                                );
                            }
                        }
                        for (auto x = x0 + 1; x <= xmax - 1; ++x) {
                            if (door_side != 2 || x != door_pos) {
                                this->createSingleTextureObject(
                                    std::make_unique<Object>(x, y0, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0),
                                    "wall"
                                );
                            }
                            if (door_side != 3 || x != door_pos) {
                                this->createSingleTextureObject(
                                    std::make_unique<Object>(x, ymax, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0),
                                    "wall"
                                );
                            }
                        }
                        monumentRtree.insert(monumentBox);
                        placed++;
                    }
                    attempts++;
                }
            }
        }

        // Human players. The number must be exact.
        {
            decltype(this->nHumanPlayersInitial) totalPlayers = 0;
            while (totalPlayers < this->nHumanPlayersInitial) {
                unsigned int x = 1 + this->randUint() % (this->width - 2);
                unsigned int y = 1 + this->randUint() % (this->height - 2);
                if (this->isTileEmpty(x, y)) {
                    this->createSingleTextureObject(
                        std::make_unique<Object>(
                            x,
                            y,
                            Object::Type::FRUIT_EATER,
                            std::make_unique<HumanActor>(),
                            fov
                        ),
                        "human"
                    );
                    totalPlayers++;
                }
            }
        }

        // Other objects.
        {
            typedef std::pair<double, std::function<void(unsigned int, unsigned int)>> P;
            auto p = std::vector<P> {
                {
                    this->config.getConfigDouble("frac-follow-eaters", 0.0025),
                    [&](unsigned int x, unsigned int y){
                        this->createSingleTextureObject(
                            std::make_unique<Object>(
                                x,
                                y,
                                Object::Type::FRUIT_EATER,
                                std::make_unique<FollowTypeActor>(Object::Type::FRUIT),
                                fov
                            ),
                            "eater"
                        );
                    }
                },
                {
                    this->config.getConfigDouble("frac-random-eaters", 0.0025),
                    [&](unsigned int x, unsigned int y){
                        this->createSingleTextureObject(
                            std::make_unique<Object>(
                                x,
                                y,
                                Object::Type::FRUIT_EATER,
                                std::make_unique<RandomActor>(this->randomSeed),
                                fov
                            ),
                            "eater"
                        );
                    }
                },
                {
                    this->config.getConfigDouble("frac-great-fruit", 0.0002),
                    [&](unsigned int x, unsigned int y){
                        this->createSingleTextureObject(
                            std::make_unique<FruitObject>(x, y, 5),
                            "great_fruit"
                        );
                    }
                },
                {
                    this->config.getConfigDouble("frac-great-fruit", 0.002),
                    [&](unsigned int x, unsigned int y){
                        this->createSingleTextureObject(
                            std::make_unique<FruitObject>(x, y, -1),
                            "bad_fruit"
                        );
                    }
                },
                {
                    this->config.getConfigDouble("frac-fruit", 0.01),
                    [&](unsigned int x, unsigned int y){
                        this->createSingleTextureObject(
                            std::make_unique<FruitObject>(x, y),
                            "fruit"
                        );
                    }
                },
                {
                    this->config.getConfigDouble("frac-wall", 0.01),
                    [&](unsigned int x, unsigned int y){
                        auto it = monumentRtree.qbegin(bgi::intersects(MonumentPoint(x, y)));
                        auto end = monumentRtree.qend();
                        if (it == end) {
                            this->createSingleTextureObject(
                                std::make_unique<Object>(x, y, Object::Type::WALL, std::make_unique<DoNothingActor>(), 0),
                                "wall"
                            );
                        }
                    }
                },
                {
                    this->config.getConfigDouble("frac-tree", 0.001),
                    [&](unsigned int x, unsigned int y){
                        auto it = monumentRtree.qbegin(bgi::intersects(MonumentPoint(x, y)));
                        auto end = monumentRtree.qend();
                        if (it == end) {
                            this->createSingleTextureObject(
                                std::make_unique<Object>(x, y, Object::Type::TREE, std::make_unique<DoNothingActor>(), 0),
                                "tree"
                            );
                        }
                    }
                },
                {
                    this->config.getConfigDouble("frac-teleport", 0.001),
                    [&](unsigned int x, unsigned int y){
                        auto it = monumentRtree.qbegin(bgi::intersects(MonumentPoint(x, y)));
                        auto end = monumentRtree.qend();
                        if (it == end) {
                            this->createSingleTextureObject(
                                std::make_unique<Object>(x, y, Object::Type::TELEPORT, std::make_unique<DoNothingActor>(), 0),
                                "teleport"
                            );
                        }
                    }
                },
            };
            std::partial_sum(p.begin(), p.end(), p.begin(),
                [](const P& x, const P& y){return P(x.first + y.first, y.second);}
            );
            std::map<P::first_type, P::second_type> cumulative(p.begin(), p.end());
            auto last_p = cumulative.rbegin()->first;
            if (last_p < 1.0) {
                cumulative.emplace(1.0, [&](unsigned int, unsigned int){});
            } else if (last_p > 1.0) {
                throw std::runtime_error(std::string("probability sum > 1"));
            }
            for (unsigned int y = 1; y < this->height - 1; ++y) {
                for (unsigned int x = 1; x < this->width - 1; ++x) {
                    if (this->isTileEmpty(x, y)) {
                        cumulative.lower_bound(this->randDouble())->second(x, y);
                    }
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

                auto oldX = object->getX();
                auto oldY = object->getY();
                decltype(oldX) x = oldX;
                decltype(oldY) y = oldY;

                // X
                if (action.getMoveX() == Action::MoveX::LEFT) {
                    if (x > 0) {
                        x = oldX - 1;
                    }
                } else if (action.getMoveX() == Action::MoveX::RIGHT) {
                    if (x < this->getWidth() - 1) {
                        x = oldX + 1;
                    }
                }

                // Y
                if (action.getMoveY() == Action::MoveY::DOWN) {
                    if (y < this->getHeight() - 1) {
                        y = oldY - 1;
                    }
                } else if (action.getMoveY() == Action::MoveY::UP) {
                    if (y > 0) {
                        y = oldY + 1;
                    }
                }

                // Teleport.
                {
                    Object *objectAtTarget;
                    bool targetNonEmpty = this->findObjectAtTile(&objectAtTarget, x, y);
                    if (targetNonEmpty) {
                        auto targetType = objectAtTarget->getType();
                        if (targetType == Object::Type::TELEPORT) {
                            auto teleportObjectAtTarget = static_cast<TeleportObject*>(objectAtTarget);
                            auto dest = teleportObjectAtTarget->getDestination();
                            x = dest->getX() + (x - oldX);
                            y = dest->getY() + (y - oldY);
                        }
                    }
                }

                auto objectType = object->getType();
                if (objectType == Object::Type::FRUIT_EATER) {
                    Object *objectAtTarget;
                    bool tileNonEmpty = this->findObjectAtTile(&objectAtTarget, x, y);
                    bool shouldMove = false;
                    if (tileNonEmpty) {
                        auto targetType = objectAtTarget->getType();
                        if (targetType == Object::Type::FRUIT) {
                            // TODO: avoid renaming here. But then rtree.remove fails.
                            auto fruitObjectAtTarget = static_cast<FruitObject*>(objectAtTarget);
                            shouldMove = true;
                            object->setScore(object->getScore() + fruitObjectAtTarget->getValue());
                            this->deleteObject(objectAtTarget);
                        }
                    } else {
                        shouldMove = true;
                    }
                    if (shouldMove) {
                        this->updatePosition(*object, x, y);
                    }
                } else if (objectType == Object::Type::TREE) {
                    auto tree_radius = 5u;
                    for (unsigned int yfruit = std::max((int)y - (int)tree_radius, 0); yfruit <= std::min(y + tree_radius, this->height - 1); ++yfruit) {
                        for (unsigned int xfruit = std::max((int)x - (int)tree_radius, 0); xfruit <= std::min(x + tree_radius, this->width - 1); ++xfruit) {
                            if (this->randDouble() < this->config.treeFracFruitSpawn && this->isTileEmpty(xfruit, yfruit)) {
                                this->createSingleTextureObject(
                                    std::make_unique<FruitObject>(xfruit, yfruit),
                                    "fruit"
                                );
                            }
                        }
                    }
                }
            }
        }

        // Spawn new objects randomly.
        if (this->spawn) {
            // Fruits
            for (unsigned int y = 1; y < this->height - 1; ++y) {
                for (unsigned int x = 1; x < this->width - 1; ++x) {
                    if (
                        this->isTileEmpty(x, y) &&
                        (this->randDouble() < this->config.fracFruitSpawn)
                    ) {
                        this->createSingleTextureObject(
                            std::make_unique<FruitObject>(x, y),
                            "fruit"
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

World::objects_t::iterator World::addObject(std::unique_ptr<Object>&& object) {
    this->rtree.insert(object.get());
    return this->objects.insert(std::move(object)).first;
}

World::objects_t::iterator World::createSingleTextureObject(
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
    return this->addObject(std::move(object));
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

unsigned int World::randUint() {
    return this->uniformUintDistribution(this->prng);
}

double World::randDouble() {
    return this->uniformDoubleDistribution(this->prng);
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
