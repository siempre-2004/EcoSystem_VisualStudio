// world_init.cpp

#include "world.hpp"
//#include "utils/collider_edges.hpp"

namespace sim
{
    void World::init(int width, int height, Texture* texture, Texture* pTexture, Texture *hTexture)
    {
        m_texture = texture;
        m_wolfTexture = pTexture;
        m_herderTexture = hTexture;

        const int columns = (width / m_tile_size.x) - TILE_PADDING_X;
        const int rows = (height / m_tile_size.y) - TILE_PADDING_Y;
        const int start_x = (width - (columns * m_tile_size.x)) / 2;
        const int start_y = (height - (rows * m_tile_size.y)) / 2;

        // note: world settings
        m_world_size = { columns, rows };
        m_world_offset = { start_x, start_y };
        m_world_bounds = {
           .x = float(m_world_offset.x),
           .y = float(m_world_offset.y),
           .width = float(columns * m_tile_size.x),
           .height = float(rows * m_tile_size.y)
        };

        { // note: initialize ground layer
            m_ground.resize(columns * rows);

            int ground_tile_index = 0;
            for (Ground& ground : m_ground) {
                const int x = (ground_tile_index % columns);
                const int y = (ground_tile_index / columns);
                ground_tile_index++;

                const Point tile_coord{ x, y };
                ground.set_tile_coord(tile_coord);
                ground.set_walkable(true);
            }
        }

        { // note: initialize grass layer
            m_grass = std::vector<Grass>();
            m_grass.reserve(m_world_size.x * m_world_size.y);
            m_grass.resize(m_world_size.x * m_world_size.y);

            int grass_tile_index = 0;
            for (Grass& grass : m_grass) {
                const int x = (grass_tile_index % columns);
                const int y = (grass_tile_index / columns);
                grass_tile_index++;

                const Point tile_coord{ x, y };
                grass.set_tile_coord(tile_coord);
                int chance = GetRandomValue(0, 100);
                if (chance < 7) {
                    grass.m_state = Grass::GrassState::GERMINATION;
                    float age = (float)GetRandomValue(1, 100) / 100.0f;
                    grass.set_age(age);
                }
                else {
                    grass.m_state = Grass::GrassState::NONE;
                    grass.set_age(-1.0f);
                }
            }
        }

        { // note: initialize sheep
            const float radius = 15.0f;
            const float target_distance = 70.0f;
            const Rectangle source{ 0, 60, 50, 30 };
            const Vector2 origin = Vector2Scale(Vector2{ source.width, source.height }, 0.5f);
            for (auto& sheep : m_sheep) {
                const int x = GetRandomValue(int(m_world_bounds.x), int(m_world_bounds.x + m_world_bounds.width));
                const int y = GetRandomValue(int(m_world_bounds.y), int(m_world_bounds.y + m_world_bounds.height));
                const float theta = ((float)GetRandomValue(0, 100) * 0.01f) * (180.0f / 3.14159257f);
                const Vector2 position{ (float)x, (float)y };
                const Vector2 direction{ std::cos(theta), std::sin(theta) };

                sheep->set_position(position);
                sheep->set_radius(radius);
                sheep->set_direction(direction);
                sheep->set_sprite_origin(origin);
                sheep->set_sprite_source(source);

            }
        }

        { // note: initialize wolf
            const float radius = 10.0f;
            const Rectangle source{ 0, 0, 50, 30 };
            Vector2 origin = Vector2Scale(Vector2{ source.width, source.height }, 0.5f);

            for (auto& wolf : m_wolf) {
                int x = GetRandomValue(int(m_world_bounds.x), int(m_world_bounds.x + m_world_bounds.width));
                int y = GetRandomValue(int(m_world_bounds.y), int(m_world_bounds.y + m_world_bounds.height));
                Vector2 position{ (float)x, (float)y };

                wolf.set_position(position);
                wolf.set_radius(radius);
                wolf.set_sprite_origin(origin);
                wolf.set_sprite_source(source);
            }
        }

        m_herder = std::make_unique<Herder>(*this, m_herderTexture);
        Vector2 herderPos = { m_world_bounds.x + m_world_bounds.width / 2,
                              m_world_bounds.y + m_world_bounds.height / 2 };
        m_herder->set_position(herderPos);
    }

    void World::shut()
    {
    }
} // !sim
