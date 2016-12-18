#ifndef WORLD_VIEW_HPP
#define WORLD_VIEW_HPP

#include <memory>
#include <vector>

class ObjectView;

/// The portion of the world that actors can see.
/// E.g.: actors cannot see what lies beyond their line of sight.
class WorldView {
    public:
        WorldView(unsigned int width, unsigned int height, std::unique_ptr<std::vector<std::unique_ptr<ObjectView>>> objectViews);
        unsigned int getHeight() const;
        const std::vector<std::unique_ptr<ObjectView>>& getObjectViews() const;
        unsigned int getScore() const;
        unsigned int getWidth() const;
    private:
        unsigned int height;
        unsigned int score;
        unsigned int width;
        std::unique_ptr<std::vector<std::unique_ptr<ObjectView>>> objectViews;
};

#endif
