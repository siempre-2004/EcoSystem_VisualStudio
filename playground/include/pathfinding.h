#pragma once
#ifndef GSW_PLAYGROUND_PATHFINDING_H
#define GSW_PLAYGROUND_PATHFINDING_H

#pragma once
#include "common.hpp"
#include <vector>
#include <limits>
#include <cmath>
#include <algorithm>

namespace sim {
    struct World;
    std::vector<Point> findPath(const World& world, const Point& start, const Point& goal);
    std::vector<sim::Point> findPath(const sim::World& world, const sim::Point& start, const sim::Point& goal);
}

struct Node {
    sim::Point coord;
    int gCost = std::numeric_limits<int>::max();
    int hCost = 0;
    Node* parent = nullptr;
    bool walkable = true;

    int fCost() const { return gCost + hCost; }
};

inline int heuristic(const sim::Point& a, const sim::Point& b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

//std::vector<sim::Point> findPath(const sim::World& world, const sim::Point& start, const sim::Point& goal);

#endif //GSW_PLAYGROUND_PATHFINDING_H
