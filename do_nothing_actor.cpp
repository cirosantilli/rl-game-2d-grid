#include "action.hpp"
#include "do_nothing_actor.hpp"

Action DoNothingActor::act(const WorldView&) {
    return Action();
}
