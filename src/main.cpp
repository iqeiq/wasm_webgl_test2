#include <iostream>
#include <SDL/SDL.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include "app.hpp"

static App app;

void one_iter() {
  app.updateFrame();
  SDL_GL_SwapBuffers();
}

int main(int argc, char ** argv) {
  app.initGL(512, 512);

#ifdef __EMSCRIPTEN__
  // void emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);
  emscripten_set_main_loop(one_iter, 60, 1);
#else
  while (1) {
    one_iter();
    // Delay to keep frame rate constant (using SDL)
    SDL_Delay(time_to_next_frame());
  }
#endif

}
