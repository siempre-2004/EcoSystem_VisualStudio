// editor.hpp

#pragma once

#include "common.hpp"
#include "pathfinding.h"

namespace sim
{
   struct World;

   struct Editor {
      Editor(World &world);

      void init();
      void shut();
      bool update(float dt);
      void render() const;

      World &m_world;
      Point m_cursor;
      bool m_is_tile_valid{};
      Point m_tile_coord;
      int m_tile_index{};

      bool m_showPath = false;
      bool m_startSet = false;
      Point m_startPoint;
      Point m_goalPoint;
      std::vector<sim::Point> m_path;
   };
}
