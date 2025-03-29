// world.hpp

#pragma once

#include "common.hpp"
#include "entity.hpp"
#include "pathfinding.h"
#include <memory>
#include <vector>

namespace sim
{
    enum class EntityType { None, Sheep, Wolf, Herder };

    struct SelectedEntity {
        EntityType type = EntityType::None;
        void* entity = nullptr;
    };

    struct Ground;
    struct Grass;
    struct Sheep;
    struct Wolf;
    struct Manure;
    struct Herder;
    struct World {
        static constexpr int TILE_SIZE = 32;
        static constexpr int TILE_PADDING_X = 3;
        static constexpr int TILE_PADDING_Y = 2;

        World();

        void init(int width, int height, Texture* texture, Texture *pTexture, Texture *hTexture);
        void shut();
        bool update(float dt);
        void render() const;

        bool is_valid_coord(const Point& coord) const;
        bool is_walkable(const Point& coord) const;
        bool has_grass_at(const Point& coord) const;
        Point position_to_tile_coord(const Vector2& position) const;
        Vector2 tile_coord_to_position(const Point& coord) const;
        std::vector<Grass>& getGrass() { return m_grass; }
        Point findNearestGrass(const Point& start) const;
        Point findNearestSheep(const Point& start) const;
        void toggleDebugPath();

        SelectedEntity m_selectedEntity;
        void selectEntity(const Vector2& pos);

        bool m_running = true;
        bool m_debugPathVisible = true;
        Texture* m_texture{ nullptr };
        Texture* m_wolfTexture{ nullptr };
        Texture* m_herderTexture{ nullptr };
        Point m_tile_size;
        Point m_world_size;
        Point m_world_offset;
        Rectangle m_world_bounds{};

        std::vector<Ground> m_ground;
        std::vector<Grass> m_grass;
        std::vector<std::shared_ptr<Sheep>> m_sheep;
        std::vector<Wolf> m_wolf;
        std::vector<Manure> m_manure;
        std::unique_ptr<Herder> m_herder;
    };
} // !sim
