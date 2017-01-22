/*
This file deals with
*/

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include <SDL2/SDL.h>

#include "action.hpp"
#include "utils.hpp"
#include "world.hpp"

static bool activateKey(
    SDL_Scancode scancode,
    const Uint8 *keyboardState,
    const Uint8 *lastKeyboardState,
    bool holdKey
) {
    return
        keyboardState[scancode] &&
        (
            holdKey ||
            !lastKeyboardState[scancode]
        );
}

static bool activateKeyActionModified(
    SDL_Scancode scancode,
    const Uint8 *keyboardState,
    const Uint8 *lastKeyboardState,
    bool holdKey,
    bool& currentActionModified
) {
    bool ret = activateKey(scancode, keyboardState, lastKeyboardState, holdKey);
    currentActionModified = currentActionModified || ret;
    return ret;
}

// Controls that may not act immediately.
static bool holdControls(
    const Uint8* keyboardState,
    const Uint8* lastKeyboardState,
    bool holdKey,
    Action& action
) {
    bool currentActionModified = false;
    if (activateKeyActionModified(SDL_SCANCODE_LEFT, keyboardState, lastKeyboardState, holdKey, currentActionModified)) {
        action.setMoveX(Action::MoveX::LEFT);
    }
    if (activateKeyActionModified(SDL_SCANCODE_RIGHT, keyboardState, lastKeyboardState, holdKey, currentActionModified)) {
        action.setMoveX(Action::MoveX::RIGHT);
    }
    if (activateKeyActionModified(SDL_SCANCODE_UP, keyboardState, lastKeyboardState, holdKey, currentActionModified)) {
        action.setMoveY(Action::MoveY::UP);
    }
    if (activateKeyActionModified(SDL_SCANCODE_DOWN, keyboardState, lastKeyboardState, holdKey, currentActionModified)) {
        action.setMoveY(Action::MoveY::DOWN);
    }
    if (activateKeyActionModified(SDL_SCANCODE_SPACE, keyboardState, lastKeyboardState, holdKey, currentActionModified)) {
    }
    return currentActionModified;
}

static void cat(std::string filename) {
    std::ifstream file(filename);
    std::string str;
    while (std::getline(file, str)) {
        std::cerr << str << std::endl;
    }
}

static void printHelp() {
    cat("controls.md");
    std::cerr << std::endl;
    cat("examples.md");
    std::cerr << std::endl;
    cat("cli.md");
}

int main(int argc, char **argv) {
    SDL_Event event;
    std::unique_ptr<World> world;
    bool
        blockOnPlayer = true,
        display = true,
        randomSeedGiven = false,
        holdKey = false,
        immediateAction = true,
        limitFps = false,
        showFov = false,
        spawn = true,
        windowPosGiven = false,
        verbose = false
    ;
    std::string scenario;
    double
        targetFps = 1.0,
        lastTime;
    ;
    int timeLimit = -1;
    unsigned int
        nHumanPlayers = 1,
        randomSeed = 0,
        showPlayerId = 0,
        width = 20,
        windowPosX = 0,
        windowPosY = 0,
        windowWidthPix = 500
    ;

    // Treat CLI arguments.
    for (decltype(argc) i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {

            // Interactive play options.
            if (std::strcmp(argv[i], "-b") == 0) {
                blockOnPlayer = !blockOnPlayer;
            } else if (std::strcmp(argv[i], "-d") == 0) {
                display = !display;
            } else if (std::strcmp(argv[i], "-f") == 0) {
                limitFps = !limitFps;
                targetFps = std::strtod(argv[i + 1], NULL);
                i++;
            } else if (std::strcmp(argv[i], "-H") == 0) {
                holdKey = !holdKey;
            } else if (std::strcmp(argv[i], "-i") == 0) {
                immediateAction = !immediateAction;
            } else if (std::strcmp(argv[i], "-P") == 0) {
                windowPosGiven = true;
                windowPosX = std::strtol(argv[i + 1], NULL, 10);
                windowPosY = std::strtol(argv[i + 2], NULL, 10);
                i += 2;
            } else if (std::strcmp(argv[i], "-v") == 0) {
                showPlayerId = std::strtol(argv[i + 1], NULL, 10);
                i++;
                showFov = true;
            } else if (std::strcmp(argv[i], "-W") == 0) {
                windowWidthPix = std::strtol(argv[i + 1], NULL, 10);
                i++;

            // World state options.
            } else if (std::strcmp(argv[i], "-p") == 0) {
                nHumanPlayers = std::strtol(argv[i + 1], NULL, 10);
                i++;
            } else if (std::strcmp(argv[i], "-r") == 0) {
                randomSeed = std::strtol(argv[i + 1], NULL, 10);
                i++;
                randomSeedGiven = true;
            } else if (std::strcmp(argv[i], "-s") == 0) {
                scenario = argv[i + 1];
            } else if (std::strcmp(argv[i], "-t") == 0) {
                timeLimit = std::strtol(argv[i + 1], NULL, 10);
                i++;
            } else if (std::strcmp(argv[i], "-w") == 0) {
                width = std::strtol(argv[i + 1], NULL, 10);
                i++;

            // Debug options.
            } else if (std::strcmp(argv[i], "-h") == 0) {
                printHelp();
                std::exit(EXIT_SUCCESS);
            } else if (std::strcmp(argv[i], "-S") == 0) {
                spawn = !spawn;
            } else if (std::strcmp(argv[i], "-V") == 0) {
                verbose = !verbose;
            } else {
                printHelp();
                std::exit(EXIT_FAILURE);
            }
        }
    }
    auto windowHeightPix = windowWidthPix;
    auto targetSpf = 1.0 / targetFps;
    auto height = width;
    if (!display) {
        nHumanPlayers = 0;
    }

    world = std::make_unique<World>(
        width,
        height,
        display,
        windowWidthPix,
        windowHeightPix,
        showPlayerId,
        showFov,
        randomSeedGiven,
        randomSeed,
        nHumanPlayers,
        std::move(scenario),
        timeLimit,
        verbose,
        spawn,
        windowPosX,
        windowPosY,
        windowPosGiven
    );
main_loop:
    lastTime = utils::get_secs();

    // Keyboard state.
    int numkeys;
    const Uint8 * keyboardState = SDL_GetKeyboardState(&numkeys);
    size_t keyboardStateSize = numkeys * sizeof(*keyboardState);
    auto lastKeyboardState = std::make_unique<Uint8[]>(keyboardStateSize);

    // Human actions.
    std::vector<std::unique_ptr<Action>> humanActions;
    for (decltype(world->getNHumanActions()) i = 0; i < world->getNHumanActions(); ++i) {
        humanActions.push_back(std::make_unique<Action>());
    }

    while (!world->isGameOver()) {
        world->draw();
        double nextTarget = lastTime + targetSpf;
        decltype(humanActions.size()) currentActionIdx = 0;
        bool needMoreHumanActions = currentActionIdx < world->getNHumanActions();
        for (auto &action : humanActions) {
            action->reset();
        }
        // Action being currently built, possibly with multiple keypresses.
        Action currentAction, holdAction;
        bool loop = true;
        do {
            double slack = nextTarget - utils::get_secs();
            bool
                slackOver = (slack < 0.0),
                currentActionModified = false,
                done = false
            ;
            if (display) {
                std::memcpy(lastKeyboardState.get(), keyboardState, keyboardStateSize);
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        goto quit;
                    }
                }

                // Game meta controls.
                if (keyboardState[SDL_SCANCODE_P] && !lastKeyboardState[SDL_SCANCODE_P]) {
                    world->pause();
                    done = true;
                }
                if (keyboardState[SDL_SCANCODE_Q] && !lastKeyboardState[SDL_SCANCODE_Q]) {
                    world->quit();
                    done = true;
                }
                if (keyboardState[SDL_SCANCODE_R] && !lastKeyboardState[SDL_SCANCODE_R]) {
                    auto reuseSeed = keyboardState[SDL_SCANCODE_LSHIFT] || keyboardState[SDL_SCANCODE_RSHIFT];
                    world->reset(reuseSeed);
                    goto main_loop;
                }

                // Actions that may be built with multiple key-presses.
                if (needMoreHumanActions) {
                    currentActionModified = holdControls(
                        keyboardState,
                        lastKeyboardState.get(),
                        false,
                        currentAction
                    );
                    if (
                        activateKey(SDL_SCANCODE_SPACE, keyboardState, lastKeyboardState.get(), false)
                        || (currentActionModified && immediateAction)
                    ) {
                        *humanActions[currentActionIdx] = currentAction;
                        currentActionIdx++;
                        needMoreHumanActions = currentActionIdx < world->getNHumanActions();
                        currentAction.reset();
                        currentActionModified = false;
                    }
                }
                if (slackOver && holdKey && world->getNHumanActions() > 0) {
                    holdAction.reset();
                    bool holdActionModified = holdControls(
                        keyboardState,
                        lastKeyboardState.get(),
                        true,
                        currentAction
                    );
                    if (holdActionModified) {
                        *humanActions[0] = currentAction;
                        done = true;
                    }
                }
            }
            loop =
                !done
                &&
                (
                    (blockOnPlayer && needMoreHumanActions)
                    ||
                    (limitFps && !slackOver)
                )
            ;
        } while (loop);
        lastTime = utils::get_secs();
        world->update(humanActions);
    }
quit:
    world->printScores();
    return EXIT_SUCCESS;
}
