#include "pathfinding.h"
#include "world.hpp"


namespace sim {
    std::vector<Point> findPath(const World& world, const Point& start, const Point& goal) {
        int gridWidth = world.m_world_size.x; //Get the map size
        int gridHeight = world.m_world_size.y;
        std::vector<Node> nodes(gridWidth * gridHeight);//Each tile corresponds to a node

        auto index = [gridWidth](const Point& p) -> int {
            return p.y * gridWidth + p.x;//Converts xy coordinates to a 1D index
            };
        //Initialization
        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                Node& node = nodes[y * gridWidth + x];
                node.coord = Point(x, y);
                node.walkable = world.is_walkable(node.coord);
                node.gCost = std::numeric_limits<int>::max();
                node.hCost = 0;
                node.parent = nullptr;
            }
        }
        //Obtain start and goal points and initialize the cost
        Node* startNode = &nodes[index(start)];
        Node* goalNode = &nodes[index(goal)];
        startNode->gCost = 0;
        startNode->hCost = heuristic(start, goal);

        std::vector<Node*> openList;
        std::vector<Node*> closedList;
        openList.push_back(startNode);

        while (!openList.empty()) { //Locate the node with the smallest fCost in openList
            auto currentIt = std::min_element(openList.begin(), openList.end(),
                [](Node* a, Node* b) {
                    return a->fCost() < b->fCost();
                });
            Node* currentNode = *currentIt;
            //A classical method for constructing least-cost paths
            if (currentNode == goalNode) {
                std::vector<Point> path;
                while (currentNode != nullptr) {
                    path.push_back(currentNode->coord);
                    currentNode = currentNode->parent;
                }
                std::reverse(path.begin(), path.end());
                return path;
            }

            openList.erase(currentIt); //openList removes the current node and adds it to closedList
            closedList.push_back(currentNode);
            //Iterate four directions, simplifying A*
            std::vector<Point> directions = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };
            for (const auto& d : directions) {
                Point neighborCoord = currentNode->coord + d;
                if (neighborCoord.x < 0 || neighborCoord.y < 0 ||
                    neighborCoord.x >= gridWidth || neighborCoord.y >= gridHeight)
                    continue;
                Node* neighbor = &nodes[index(neighborCoord)];
                //Complies with the tile restrictions ingame
                if (!neighbor->walkable ||
                    std::find(closedList.begin(), closedList.end(), neighbor) != closedList.end())
                    continue;

                int tentativeGCost = currentNode->gCost + 1;

                if (tentativeGCost < neighbor->gCost ||
                    std::find(openList.begin(), openList.end(), neighbor) == openList.end()) {
                    neighbor->gCost = tentativeGCost;
                    neighbor->hCost = heuristic(neighborCoord, goal);
                    neighbor->parent = currentNode;
                    if (std::find(openList.begin(), openList.end(), neighbor) == openList.end()) {
                        openList.push_back(neighbor);
                    }
                }
            }
        }
        return std::vector<Point>();
    }
} // namespace sim

