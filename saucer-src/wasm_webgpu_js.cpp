#include <saucer/smartview.hpp>

#include "generated/wasm-webgpu-js/all.hpp"

int main() {
  auto app = saucer::application::acquire({
      .id = "wasm-webgpu js",
  });

  saucer::smartview smartview{{
      .application = app,
  }};

  smartview.set_title("wasm webgpu app (JS version)");
  smartview.embed(saucer::embedded::all());
  smartview.set_size(1200, 1600);

  smartview.serve("src/index.html");
  smartview.set_dev_tools(true);

  smartview.show();

  app->run();

  return 0;
}
