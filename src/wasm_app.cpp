#include <emscripten.h>
#include <emscripten/bind.h>

#include <iostream>

using namespace emscripten;

class WasmExample {
 public:
  void run() { std::cout << "Running WASM Example!" << std::endl; }
};

// Exposed WASM class
EMSCRIPTEN_BINDINGS(wasm_example) {
  class_<WasmExample>("WasmExample")
      .constructor<>()
      .function("run", &WasmExample::run);
}

// Exposed function
extern "C" {
EMSCRIPTEN_KEEPALIVE void runWasmFunction() {
  WasmExample example;
  example.run();
}
}
