// appstate.hpp

#pragma once

#include "world.hpp"
#include "editor.hpp"

namespace sim
{
   struct AppState {
      enum class Mode {
         VIEW,
         EDIT,
      };

      AppState();

      bool init(int width, int height);
      void shut();
      bool update(float dt);
      void render() const;

      bool m_running = true;
      Mode m_mode{};
      Texture m_texture{};
      Texture m_wolfTexture{};
      Texture m_herderTexture{};
      World m_world;
      Editor m_editor;
   };
}