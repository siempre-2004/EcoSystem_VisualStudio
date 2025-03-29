// main.cpp

#include "appstate.hpp"
   
int main(int argc, char **argv)
{
   const int window_width = 1920, window_height = 1080;
   const std::string_view window_title = "[5SD806] AI Playground";

   InitWindow(window_width, window_height, window_title.data());
   InitAudioDevice();
   SetTargetFPS(60);
   SetExitKey(0);

   sim::AppState app;
   app.init(window_width, window_height);

   bool running = true;
   while (running) {
      const float dt = GetFrameTime();
      running &= !WindowShouldClose();
      running &= app.update(dt);

      BeginDrawing();
      ClearBackground(SKYBLUE);

      app.render();

      DrawFPS(2, GetScreenHeight() - 20);
      EndDrawing();
   }

   CloseAudioDevice();
   CloseWindow();

   return 0;
}
