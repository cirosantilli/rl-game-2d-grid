#include "object_view.hpp"
#include "world_view.hpp"

WorldView::WorldView(unsigned int width, unsigned int height, std::unique_ptr<std::vector<std::unique_ptr<ObjectView>>> objectViews) :
    width(width), height(height), objectViews(std::move(objectViews)) {}
unsigned int WorldView::getHeight() const { return this->height; }
const std::vector<std::unique_ptr<ObjectView>>& WorldView::getObjectViews() const { return *this->objectViews; }
unsigned int WorldView::getScore() const { return this->score; }
unsigned int WorldView::getWidth() const { return this->width; }
