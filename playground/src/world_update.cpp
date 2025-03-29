// world_update.cpp

#include "world.hpp"

namespace sim
{
    template <typename T>

    void contain_within_bounds(T& entity, const Rectangle& bounds)
    {
        if (entity.m_position.x < bounds.x + entity.m_radius) {
            entity.m_position.x = bounds.x + entity.m_radius;
            entity.m_direction.x = -entity.m_direction.x;
        }
        if (entity.m_position.x > bounds.x + bounds.width - entity.m_radius) {
            entity.m_position.x = bounds.x + bounds.width - entity.m_radius;
            entity.m_direction.x = -entity.m_direction.x;
        }
        if (entity.m_position.y < bounds.y + entity.m_radius) {
            entity.m_position.y = bounds.y + entity.m_radius;
            entity.m_direction.y = -entity.m_direction.y;
        }
        if (entity.m_position.y > bounds.y + bounds.height - entity.m_radius) {
            entity.m_position.y = bounds.y + bounds.height - entity.m_radius;
            entity.m_direction.y = -entity.m_direction.y;
        }
    }

    bool World::update(float dt)
    {
        if (IsKeyReleased(KEY_ESCAPE)) {
            m_running = false;
        }

        for (auto& grass : m_grass) {
            grass.update(dt);
        }

        for (auto& wolf : m_wolf) {
            wolf.update(dt);
            contain_within_bounds(wolf, m_world_bounds);
        }

        for (auto& sheep : m_sheep) {
            sheep->update(dt);
            contain_within_bounds(*sheep, m_world_bounds);
        }

        for (auto& manure : m_manure) {
            manure.update(dt);
        }

        m_wolf.erase(std::remove_if(m_wolf.begin(), m_wolf.end(),
            [](const Wolf& w) { return w.m_state == Wolf::WolfState::DEAD; }),
            m_wolf.end());

        m_manure.erase(std::remove_if(m_manure.begin(), m_manure.end(),
            [](const Manure& m) { return !m.m_isActive; }),
            m_manure.end()); 

        if (m_herder) {
            m_herder->update(dt);
        }

        return m_running;
    }
}
