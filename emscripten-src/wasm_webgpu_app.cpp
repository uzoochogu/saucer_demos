#include <emscripten.h>
#include <emscripten/bind.h>
#include <webgpu/webgpu.h>

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

class WebGPURenderer {
 private:
  WGPUDevice device;
  WGPUQueue queue;
  WGPUSwapChain swapChain;
  WGPURenderPipeline pipeline;

  void InitializeAdapter(void *userData, WGPURequestAdapterStatus status,
                         WGPUAdapter adapter, const char *message) {
    if (status == WGPURequestAdapterStatus_Success) {
      WGPUDeviceDescriptor deviceDesc = {};
      deviceDesc.label = "My Device";
      deviceDesc.requiredFeatureCount = 0;
      deviceDesc.requiredLimits = nullptr;
      deviceDesc.defaultQueue.label = "Default Queue";

      wgpuAdapterRequestDevice(
          adapter, &deviceDesc,
          [](WGPURequestDeviceStatus status, WGPUDevice device,
             const char *message, void *userdata) {
            auto *self = static_cast<WebGPURenderer *>(userdata);
            if (status == WGPURequestDeviceStatus_Success) {
              self->device = device;
              self->queue = wgpuDeviceGetQueue(device);

              // Set up surface from canvas
              WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc = {};
              canvasDesc.chain.sType =
                  WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
              canvasDesc.selector = "#canvas";

              WGPUSurfaceDescriptor surfaceDesc = {};
              surfaceDesc.nextInChain =
                  reinterpret_cast<WGPUChainedStruct *>(&canvasDesc);

              WGPUSurface surface =
                  wgpuInstanceCreateSurface(nullptr, &surfaceDesc);

              WGPUSwapChainDescriptor swapChainDesc = {};
              swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
              swapChainDesc.format = WGPUTextureFormat_BGRA8Unorm;
              swapChainDesc.width = 800;
              swapChainDesc.height = 600;
              swapChainDesc.presentMode = WGPUPresentMode_Fifo;

              self->swapChain =
                  wgpuDeviceCreateSwapChain(device, surface, &swapChainDesc);

              self->SetupPipeline();
              self->Render();
            }
          },
          this);
    }
  }

  void SetupPipeline() {
    // Shader module
    WGPUShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    wgslDesc.code = shaderCode;

    WGPUShaderModuleDescriptor shaderDesc = {};
    shaderDesc.nextInChain = reinterpret_cast<WGPUChainedStruct *>(&wgslDesc);
    WGPUShaderModule shaderModule =
        wgpuDeviceCreateShaderModule(device, &shaderDesc);

    WGPURenderPipelineDescriptor pipelineDesc = {};

    // Vertex state
    WGPUVertexState vertexState = {};
    vertexState.module = shaderModule;
    vertexState.entryPoint = "vertexMain";
    vertexState.constantCount = 0;
    vertexState.constants = nullptr;
    pipelineDesc.vertex = vertexState;

    // Fragment state
    WGPUBlendState blend = {};
    blend.color.operation = WGPUBlendOperation_Add;
    blend.color.srcFactor = WGPUBlendFactor_One;
    blend.color.dstFactor = WGPUBlendFactor_Zero;
    blend.alpha = blend.color;

    WGPUColorTargetState colorTarget = {};
    colorTarget.format = WGPUTextureFormat_BGRA8Unorm;
    colorTarget.blend = &blend;
    colorTarget.writeMask = WGPUColorWriteMask_All;

    WGPUFragmentState fragmentState = {};
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fragmentMain";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;

    // Other pipeline settings
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
    pipelineDesc.primitive.cullMode = WGPUCullMode_None;

    WGPUMultisampleState multisample = {};
    multisample.count = 1;
    multisample.mask = ~0u;
    multisample.alphaToCoverageEnabled = false;
    pipelineDesc.multisample = multisample;

    pipeline = wgpuDeviceCreateRenderPipeline(device, &pipelineDesc);
  }

  void Render() {
    WGPUTextureView view = wgpuSwapChainGetCurrentTextureView(swapChain);

    WGPUCommandEncoderDescriptor encoderDesc = {};
    WGPUCommandEncoder encoder =
        wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

    WGPURenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = view;
    colorAttachment.loadOp = WGPULoadOp_Clear;
    colorAttachment.storeOp = WGPUStoreOp_Store;
    colorAttachment.clearValue = {0.0f, 0.0f, 0.0f, 1.0f};
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

    WGPURenderPassDescriptor renderPassDesc = {};
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.depthStencilAttachment = nullptr;
    renderPassDesc.timestampWrites = nullptr;

    WGPURenderPassEncoder pass =
        wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    wgpuRenderPassEncoderSetPipeline(pass, pipeline);
    wgpuRenderPassEncoderDraw(pass, 3, 1, 0, 0);
    wgpuRenderPassEncoderEnd(pass);

    WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, nullptr);
    wgpuQueueSubmit(queue, 1, &commands);
  }

 public:
  void Initialize() {
    WGPURequestAdapterOptions options = {};
    wgpuInstanceRequestAdapter(
        nullptr, &options,
        [](WGPURequestAdapterStatus status, WGPUAdapter adapter,
           const char *message, void *userdata) {
          auto *self = static_cast<WebGPURenderer *>(userdata);
          self->InitializeAdapter(userdata, status, adapter, message);
        },
        this);
  }
};

EMSCRIPTEN_BINDINGS(webgpu_module) {
  emscripten::class_<WebGPURenderer>("WebGPURenderer")
      .constructor<>()
      .function("initialize", &WebGPURenderer::Initialize);
}

// Not used in html
extern "C" {
EMSCRIPTEN_KEEPALIVE
void runWasmFunction() {
  WebGPURenderer renderer;
  renderer.Initialize();
}
}
