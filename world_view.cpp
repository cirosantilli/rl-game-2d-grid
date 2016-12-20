#include "object_view.hpp"
#include "world_view.hpp"

WorldView::WorldView(
    unsigned int width,
    unsigned int height,
    std::unique_ptr<std::vector<std::unique_ptr<ObjectView>>> objectViews,
    unsigned int score
) :
    objectViews(std::move(objectViews)), height(height), score(score), width(width) {}
unsigned int WorldView::getHeight() const { return this->height; }
const std::vector<std::unique_ptr<ObjectView>>& WorldView::getObjectViews() const { return *this->objectViews; }
unsigned int WorldView::getScore() const { return this->score; }
unsigned int WorldView::getWidth() const { return this->width; }
