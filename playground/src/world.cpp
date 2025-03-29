// world.cpp

#include "world.hpp"
#include "entity.hpp"
#include <iostream>
#include "float.h"

namespace sim
{
    constexpr float TILE_SIZE = 32.0f;

    World::World()
        : m_tile_size(TILE_SIZE, TILE_SIZE)
    {
        m_sheep.reserve(200);// Avoid memory crash
        m_wolf.reserve(150);
        m_manure.reserve(100);

        for (int i = 0; i < 40; i++) {
            m_sheep.push_back(std::make_shared<Sheep>(*this));
        }

        for (int i = 0; i < 3; i++) {
            m_wolf.emplace_back(*this);
        }
    }

    bool World::is_valid_coord(const Point& coord) const
    {
        bool x_axis = coord.x >= 0 && coord.x < m_world_size.x;
        bool y_axis = coord.y >= 0 && coord.y < m_world_size.y;
        return x_axis && y_axis;
    }

    bool World::is_walkable(const Point& coord) const
    {
        if (!is_valid_coord(coord)) {
            return false;
        }
        return m_ground[coord.y * m_world_size.x + coord.x].is_walkable();
    }

    bool World::has_grass_at(const Point& coord) const
    {
        if (!is_valid_coord(coord)) {
            return false;
        }
        const Grass& grass = m_grass[coord.y * m_world_size.x + coord.x];
        return m_grass[coord.y * m_world_size.x + coord.x].is_alive();
    }

    Point World::position_to_tile_coord(const Vector2& position) const
    {
        Point result = position;
        result = result - m_world_offset;
        result = result / m_tile_size;
        return result;
    }

    Vector2 World::tile_coord_to_position(const Point& coord) const
    {
        Point pos = coord * m_tile_size + m_world_offset;
        Vector2 posVec = pos.to_vec2();
        posVec.x += m_tile_size.x / 2.0f;
        posVec.y += m_tile_size.y / 2.0f;
        return posVec;
    }

    Point World::findNearestGrass(const Point& start) const
    {
        float minDist = FLT_MAX;
        Point nearest = { -1, -1 };

        for (const auto& grass : m_grass) {
            if (grass.is_alive()) {
                Point grassCoord = grass.m_tile_coord;
                float dist = Vector2Distance(start.to_vec2(), grassCoord.to_vec2());
                if (dist < minDist) {
                    minDist = dist;
                    nearest = grassCoord;
                }
            }
        }
        if (!is_valid_coord(nearest)) {
            return { 0, 0 };
        }

        return nearest;
    }

    Point World::findNearestSheep(const Point& start) const
    {
        float minDist = FLT_MAX;
        Point nearest = { -1, -1 };

        for (const auto& sheep : m_sheep) {
            if (sheep->getState() != Sheep::SheepState::DEAD) {
                Point sheepCoord = position_to_tile_coord(sheep->m_position);
                float dist = Vector2Distance(start.to_vec2(), sheepCoord.to_vec2());
                if (dist < minDist) {
                    minDist = dist;
                    nearest = sheepCoord;
                }
            }
        }
        return nearest;
    }


    void World::toggleDebugPath() {
        m_debugPathVisible = !m_debugPathVisible;
    }

    void World::selectEntity(const Vector2& pos) {
        m_selectedEntity.type = EntityType::None;
        m_selectedEntity.entity = nullptr;
        const float tolerance = 10.0f;
        // Clear the current selection before each selection, avoid keeping the previous selection
        for (auto& sheep : m_sheep) {
            float distance = Vector2Distance(pos, sheep->m_position);
            if (distance < sheep->m_radius + tolerance) {
                m_selectedEntity.type = EntityType::Sheep;
                m_selectedEntity.entity = sheep.get();// Save the original pointer for debugging rendering
                return;
            }
        }
        for (auto& wolf : m_wolf) {
            float distance = Vector2Distance(pos, wolf.m_position);
            if (distance < wolf.m_radius + tolerance) {
                m_selectedEntity.type = EntityType::Wolf;
                m_selectedEntity.entity = &wolf;
                return;
            }
        }
        if (m_herder) {
            float distance = Vector2Distance(pos, m_herder->get_position());
            if (distance < 25.0f + tolerance) {
                m_selectedEntity.type = EntityType::Herder;
                m_selectedEntity.entity = m_herder.get();
                return;
            }
        }
    }
}