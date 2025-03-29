// world_render.cpp

#include "world.hpp"
#include <iostream>

namespace sim
{
    void World::render() const {
        assert(m_texture);

        const Vector2 ZERO{};
        const Vector2 tile_size = m_tile_size.to_vec2();

        { // note: render ground
            constexpr Rectangle source{ 0.0f, 0.0f, 16.0f, 16.0f };

            for (const Ground& ground : m_ground) {
                if (!ground.is_walkable()) {
                    continue;
                }

                const Vector2 position = (m_world_offset + ground.m_tile_coord * m_tile_size).to_vec2();
                const Rectangle destination{ position.x, position.y, tile_size.x, tile_size.y };
                DrawTexturePro(*m_texture, source, destination, ZERO, 0.0f, WHITE);
            }
        }

        { // note: render grass
            constexpr Rectangle sources[] =
            {
                    {16.0f, 0.0f, 16.0f, 16.0f},
                    {32.0f, 0.0f, 16.0f, 16.0f},
                    {48.0f, 0.0f, 16.0f, 16.0f},
                    {64.0f, 0.0f, 16.0f, 16.0f},
                    {80.0f, 0.0f, 16.0f, 16.0f},
                    {96.0f, 0.0f, 16.0f, 16.0f},
            };

            for (const Grass& tile : m_grass) {
                if (!tile.is_alive()) {
                    continue;
                }
                const int index = static_cast<int>(tile.m_state);
                const Vector2 position = (m_world_offset + tile.m_tile_coord * m_tile_size).to_vec2();
                const Rectangle source = sources[index];
                const Rectangle destination{ position.x, position.y, tile_size.x, tile_size.y };
                DrawTexturePro(*m_texture, source, destination, ZERO, 0.0f, WHITE);
            }
        }

        auto drawHealthBar = [&](const Vector2& position, int HP, int maxHP) {
            if(HP <= 0) return;
            float segmentWidth = static_cast<float>(TILE_SIZE) / 10.0f;
            int segments = 10;
            int filledSegments = (HP * segments) / maxHP;
            for (int i = 0; i < segments; i++) {
                Color color = (i < filledSegments) ? RED : DARKGRAY;
                DrawRectangle((int)(position.x + i * segmentWidth), (int)position.y - 20, (int)segmentWidth - 1, 5, color);
            }
            DrawText(TextFormat("HP:%d", HP), (int)position.x, (int)position.y - 35, 14, WHITE);
            };

        // note: render sheep
        for (const auto& sheep : m_sheep) {
            sheep->render(*m_texture);
            drawHealthBar(sheep->m_position, sheep->HP, SHEEP_MAX_HP);
        }

        for (const auto& wolf : m_wolf) {
            wolf.render(*m_wolfTexture);
            drawHealthBar(wolf.m_position, wolf.HP, SHEEP_MAX_HP);
        }

        for (const auto& manure : m_manure) {
            manure.render(*m_texture);
        }
        if (m_debugPathVisible) {
            // Print route
            for (const auto& sheep : m_sheep) {
                if (sheep->m_path.size() > 1) {
                    for (size_t i = 0; i + 1 < sheep->m_path.size(); ++i) {
                        if (!is_valid_coord(position_to_tile_coord(sheep->m_position))) {
                            continue;
                        }
                        Vector2 pos1 = tile_coord_to_position(sheep->m_path[i]);
                        Vector2 pos2 = tile_coord_to_position(sheep->m_path[i + 1]);
                        DrawLineV(pos1, pos2, GREEN);
                    }
                }
            }

            for (const auto& wolf : m_wolf) {
                if (wolf.m_path.size() > 1) {
                    for (size_t i = 0; i + 1 < wolf.m_path.size(); ++i) {
                        Vector2 pos1 = tile_coord_to_position(wolf.m_path[i]);
                        Vector2 pos2 = tile_coord_to_position(wolf.m_path[i + 1]);
                        if (!is_valid_coord(position_to_tile_coord(wolf.m_position))) {
                            continue;
                        }
                        DrawLineV(pos1, pos2, RED);
                    }
                }
            }
            for (const auto& sheep : m_sheep) {
                DrawText(
                    TextFormat("State: %s", SheepStateToString(sheep->m_state)),
                    static_cast<int>(sheep->m_position.x),
                    static_cast<int>(sheep->m_position.y) - 20,
                    10,
                    WHITE
                );
            }
            for (const auto& wolf : m_wolf) {
                DrawText(
                    TextFormat("State: %s", WolfStateToString(wolf.m_state)),
                    static_cast<int>(wolf.m_position.x),
                    static_cast<int>(wolf.m_position.y) - 20,
                    10,
                    WHITE
                );
            }

            if (m_herder) {
                m_herder->render();
            }
        }

        if (m_selectedEntity.type != EntityType::None) 
        {
            Vector2 debugPos = { 0, 0 };
            char debugText[128] = { 0 };
            switch (m_selectedEntity.type) {
            case EntityType::Sheep: {
                Sheep* s = static_cast<Sheep*>(m_selectedEntity.entity);
                debugPos = s->m_position;
                sprintf_s(debugText, sizeof(debugText), "Sheep: State=%d, HP=%d, Hunger=%.1f", s->m_state, s->HP, s->m_hunger);
                break;
            }// Help observing the behavior, judgment, and survival of entities
            case EntityType::Wolf: {
                Wolf* w = static_cast<Wolf*>(m_selectedEntity.entity);
                debugPos = w->m_position;
                sprintf_s(debugText, sizeof(debugText), "Wolf: State=%d, HP=%d, Hunger=%.1f", w->m_state, w->HP, w->m_hunger);
                break;
            }
            case EntityType::Herder: {
                Herder* h = static_cast<Herder*>(m_selectedEntity.entity);
                debugPos = h->get_position();
                sprintf_s(debugText, sizeof(debugText), "Herder: PathLen=%d", (int)h->m_path.size());
                break;
            }
            default:
                break;
            } // Render debug message text & circle entity
            DrawText(debugText, (int)debugPos.x, (int)debugPos.y - 50, 12, YELLOW);
            DrawCircleLines((int)debugPos.x, (int)debugPos.y, 25, YELLOW);
        }
    }
}
