#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/promise.h>
#include <webgpu/webgpu.h>

using namespace emscripten;

const char *shaderCode = R"(
struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
}

@vertex
fn vertexMain(@builtin(vertex_index) vertexIndex: u32) -> VertexOutput {
    var pos = array<vec2f, 3>(
        vec2f( 0.0,  0.5),
        vec2f(-0.5, -0.5),
        vec2f( 0.5, -0.5)
    );
    var colors = array<vec3f, 3>(
        vec3f(1.0, 0.0, 0.0),
        vec3f(0.0, 1.0, 0.0),
        vec3f(0.0, 0.0, 1.0)
    );
    
    var output: VertexOutput;
    output.position = vec4f(pos[vertexIndex], 0.0, 1.0);
    output.color = vec4f(colors[vertexIndex], 1.0);
    return output;
}

@fragment
fn fragmentMain(input: VertexOutput) -> @location(0) vec4f {
    return input.color;
}
)";

EM_JS(void, initWebGPUAndRender, (), {
  return Asyncify.handleSleep(function(wakeUp) {
    (async function() {
      const adapter = await navigator.gpu.requestAdapter();
      const device = await adapter.requestDevice();

      // Get WebGPU context from canvas
      const canvas = document.getElementById('canvas');
      const context = canvas.getContext('webgpu');

      const format = navigator.gpu.getPreferredCanvasFormat();
      context.configure({
        device : device,
        format : format,
        alphaMode : 'premultiplied',
      });

      const shaderModule =
          device.createShaderModule({code : UTF8ToString(Module.shaderCode)});

      const pipeline = device.createRenderPipeline({
        layout : 'auto',
        vertex : {module : shaderModule, entryPoint : 'vertexMain'},
        fragment : {
          module : shaderModule,
          entryPoint : 'fragmentMain',
          targets : [ {format : format} ]
        },
        primitive : {topology : 'triangle-list'}
      });

      // Render
      const commandEncoder = device.createCommandEncoder();
      const textureView = context.getCurrentTexture().createView();
      const renderPass = commandEncoder.beginRenderPass({
        colorAttachments : [ {
          view : textureView,
          clearValue : {r : 0.0, g : 0.0, b : 0.0, a : 1.0},
          loadOp : 'clear',
          storeOp : 'store'
        } ]
      });

      renderPass.setPipeline(pipeline);
      renderPass.draw(3, 1, 0, 0);
      renderPass.end();

      device.queue.submit([commandEncoder.finish()]);

      Module.device = device;
      wakeUp();
    })();
  });
});

class WebGPURenderer {
 public:
  void initialize() {
    EM_ASM({ Module.shaderCode = $0; }, shaderCode);
    initWebGPUAndRender();
  }
};

EMSCRIPTEN_BINDINGS(webgpu_module) {
  class_<WebGPURenderer>("WebGPURenderer")
      .constructor<>()
      .function("initialize", &WebGPURenderer::initialize);
}
