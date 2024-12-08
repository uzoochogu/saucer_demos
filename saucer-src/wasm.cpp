#include <saucer/smartview.hpp>

#include "generated/wasm/all.hpp"

int main() {
  auto app = saucer::application::acquire({
      .id = "wasm",
  });

  saucer::smartview smartview{{
      .application = app,
  }};

  smartview.set_title("wasm app");
  smartview.embed(saucer::embedded::all());
  smartview.set_size(500, 600);

  smartview.serve("src/index.html");
  smartview.set_dev_tools(true);

  smartview.show();

  app->run();

  return 0;
}
