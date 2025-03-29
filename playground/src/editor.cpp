// editor.cpp

#include "editor.hpp"
#include "world.hpp"

namespace sim
{
   namespace editor
   {
      void set_ground_active(std::vector<Ground> &ground, const Point &coord, const Point &world_size)
      {
         const int index = coord.y * world_size.x + coord.x;
         if (!ground[index].is_walkable()) {
            ground[index].set_walkable(true);
         }
      }

      void set_ground_inactive(std::vector<Ground> &ground, const Point &coord, const Point &world_size)
      {
         const int index = coord.y * world_size.x + coord.x;
         if (ground[index].is_walkable()) {
            ground[index].set_walkable(false);
         }
      }

      void set_grass_active(std::vector<Grass> &grass, const Point &coord, const Point &world_size)
      {
         const int index = coord.y * world_size.x + coord.x;
         if (!grass[index].is_alive()) {
            const float age = GetRandomValue(0, 100) / 100.0f;
            grass[index].set_age(age);
         }
      }

      void set_grass_inactive(std::vector<Grass> &grass, const Point &coord, const Point &world_size)
      {
         const int index = coord.y * world_size.x + coord.x;
         if (grass[index].is_alive()) {
            grass[index].set_age(0.0f);
         }
      }
   } // !editor

   Editor::Editor(World &world)
      : m_world(world)
   {
   }

   void Editor::init()
   {
   }

   void Editor::shut()
   {
   }

   bool Editor::update(float dt)
   {//When the mouse is placing or removing tiles on the map, the paths of all entities are updated
       if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
           for (auto& sheep : m_world.m_sheep) {
               sheep->recalculatePath();
           }
           for (auto& wolf : m_world.m_wolf) {
               wolf.recalculatePath();
           }
           if (m_world.m_herder) {
               m_world.m_herder->recalculatePath();
           }
       }

      const auto &world_bounds = m_world.m_world_bounds;
      const auto &world_offset = m_world.m_world_offset;
      const auto &world_size = m_world.m_world_size;
      const auto &tile_size = m_world.m_tile_size;

      // note: hover tile info
      m_is_tile_valid = false;
      m_cursor = GetMousePosition();
      if (CheckCollisionPointRec(m_cursor.to_vec2(), world_bounds)) {
         const Point cursor_world_position = m_cursor - world_offset;
         const Point hover_coord = cursor_world_position / tile_size;

         if (hover_coord.x >= 0 && hover_coord.x < world_size.x) {
            if (hover_coord.y >= 0 && hover_coord.y < world_size.y) {
               m_is_tile_valid = true;
               m_tile_coord = hover_coord;
               m_tile_index = hover_coord.y * world_size.x + hover_coord.x;
            }
         }
      }
      //Toggles the path display mode and initializes the path record variable
      if (IsKeyPressed(KEY_H)) {
          m_showPath = !m_showPath;
          m_startSet = false;
          m_path.clear();
      }

      if (m_showPath && m_is_tile_valid) {
          if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
              if (!m_startSet) {
                  m_startPoint = m_tile_coord;
                  m_startSet = true;
              }
              else {
                  m_goalPoint = m_tile_coord;
                  m_path = findPath(m_world, m_startPoint, m_goalPoint);
              }
          }
      }

      // note: edit mode logic
      if (m_is_tile_valid) {
         if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            editor::set_ground_active(m_world.m_ground, m_tile_coord, world_size);
            editor::set_grass_active(m_world.m_grass, m_tile_coord, world_size);
         }

         if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            editor::set_ground_inactive(m_world.m_ground, m_tile_coord, world_size);
            editor::set_grass_inactive(m_world.m_grass, m_tile_coord, world_size);
         }
      }

      return true;
   }

   void Editor::render() const
   {
       m_world.render();
       // If debugging is enabled, draw the path line
       if (m_showPath && !m_path.empty())
       {
           for (size_t i = 0; i < m_path.size() - 1; ++i) {
               Vector2 pos1 = m_world.tile_coord_to_position(m_path[i]);
               Vector2 pos2 = m_world.tile_coord_to_position(m_path[i + 1]);

               pos1.x += m_world.m_tile_size.x / 2.0f;
               pos1.y += m_world.m_tile_size.y / 2.0f;
               pos2.x += m_world.m_tile_size.x / 2.0f;
               pos2.y += m_world.m_tile_size.y / 2.0f;

               DrawLineV(pos1, pos2, MAGENTA);
           }
       }
       // Renders mouse hover tile information, including borders and debug text
       if (m_is_tile_valid)
       {
           DrawRectangleLines(m_world.m_world_offset.x + m_tile_coord.x * m_world.m_tile_size.x,
               m_world.m_world_offset.y + m_tile_coord.y * m_world.m_tile_size.y,
               m_world.m_tile_size.x,
               m_world.m_tile_size.y,
               WHITE);

           const int font_size = 10;
           const int cursor_offset_x = 2;
           const int cursor_offset_y = 20;
           int x = m_cursor.x + cursor_offset_x;
           int y = m_cursor.y + cursor_offset_y;
           const auto& ground = m_world.m_ground[m_tile_index];
           const auto& grass = m_world.m_grass[m_tile_index];

           const char* text = TextFormat("Coord: %d,%d\nIndex: %d\nSolid: %s\nAge: %.2f",
               m_tile_coord.x,
               m_tile_coord.y,
               m_tile_index,
               ground.is_walkable() ? "true" : "false",
               grass.get_age());
           DrawText(text, x, y, font_size, BLACK);
           DrawText(text, x - 1, y - 1, font_size, WHITE);
       }

      const auto &world_bounds = m_world.m_world_bounds;
      const auto &world_offset = m_world.m_world_offset;
      const auto &world_size = m_world.m_world_size;
      const auto &tile_size = m_world.m_tile_size;

      // note: debug grid
      const Color color = ColorAlpha(RAYWHITE, 0.3f);
      for (int y = 0; y <= world_size.y; y++) {
         const int ty = y * tile_size.y;
         DrawLine((int)world_bounds.x,
                  (int)world_bounds.y + ty,
                  (int)(world_bounds.x + world_bounds.width),
                  (int)world_bounds.y + ty,
                  color);
      }
      for (int x = 0; x <= world_size.x; x++) {
         const int tx = x * tile_size.x;
         DrawLine((int)world_bounds.x + tx,
                  (int)world_bounds.y,
                  (int)world_bounds.x + tx,
                  (int)(world_bounds.y + world_bounds.height),
                  color);
      }

      // note: sheep debug info
      for (const auto &sheep : m_world.m_sheep) {
         // note: render collider
          DrawCircleLinesV(sheep->m_position, sheep->m_radius, MAGENTA);
         // note: walking direction
          DrawLineV(sheep->m_position, sheep->m_position + sheep->m_direction * Sheep::WALKING_SPEED, BLACK);
         
      }

      // note: hover tile debug info
      if (m_is_tile_valid) {
         DrawRectangleLines(world_offset.x + m_tile_coord.x * tile_size.x,
                            world_offset.y + m_tile_coord.y * tile_size.y,
                            tile_size.x,
                            tile_size.y,
                            WHITE);

         const int font_size = 10;
         const int cursor_offset_x = 2;
         const int cursor_offset_y = 20;
         const int x = m_cursor.x + cursor_offset_x;
         const int y = m_cursor.y + cursor_offset_y;
         const auto &ground = m_world.m_ground[m_tile_index];
         const auto &grass = m_world.m_grass[m_tile_index];

         const char *text = TextFormat("Coord: %d,%d\nIndex: %d\nSolid: %s\nAge: %.2f",
                                       m_tile_coord.x,
                                       m_tile_coord.y,
                                       m_tile_index,
                                       ground.is_walkable() ? "true" : "false",
                                       grass.get_age());
         DrawText(text, x, y, font_size, BLACK);
         DrawText(text, x - 1, y - 1, font_size, WHITE);
      }
   }
}
