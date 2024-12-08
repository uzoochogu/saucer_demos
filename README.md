
# Saucer Demos
[`saucer`](https://github.com/saucer/saucer) is a modern C++ webview library for building cross-platform desktop applications with ease. It uses the operating system's native webview to serve the application. 
<table>
<tr>
    <th></th>
    <th>Windows</th>
    <th>Linux</th>
    <th>MacOS</th>
</tr>
<tr>
    <td rowspan="2">Backend</td>
    <td>Win32 & WebView2</td>
    <td>GTK4 & WebKitGtk</td>
    <td>Cocoa & WKWebView</td>
</tr>
<tr align="center">
    <td colspan="3">Qt5 / Qt6 & QWebEngine</td>
</tr>
</table>

This project contains a collection of demos showcasing the capabilities of the [`saucer`](https://github.com/saucer/saucer) framework.

It demonstrates the following:
* Saucer webview with WebAssembly.
* Saucer webview with WebAssembly + WebGPU using Embedded JS.
* Saucer webview with WebAssembly + WebGPU C++ API -  This is useful for Custom WebGPU engines that want to port to a WebView app.

## Demo Concept
- Application layout created using `html`, `css` (and `JS` if necessary).
- WebGPU renderer or other C++ code compiled into WebAssembly modules.
- Embed wasm modules, application layout and content files to C++ include files using `saucer-cli`.
- Final program is compiled to a single executable.

Saucer serves the application on a webview.

## Requirements
- C++23 compatible compiler: (Required for `saucer`). Tested with Clang 18.1.8. See [Troubleshooting](#troubleshooting) for more information.
- [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) installed and available in system PATH. This is required to compile WebAssembly modules.
- [Saucer CLI](https://github.com/saucer/cli) installed - `npm i -g @saucer-dev/cli`. Embeds all web files into C++ include files.
- CMake >= 3.28

## Building the Project

1. Create a build directory and navigate to it:
```bash
mkdir build
cd build
```

2. Configure CMake in a build directory:
```bash
cmake ..

# OR
# To ensure you are using the correct compiler, you can specify it manually
# e.g
cmake -B build -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
```

3. Build the project:
```bash
cmake --build .

# Note: Changes to source files should trigger relevant rebuild steps. 
# If changes are not reflected, make a clean build.
cmake --build . --clean-first
```

## Troubleshooting
If you encounter any issues:
1. Verify Emscripten is in your PATH - `emcc --version`.
2. Check that Saucer CLI is properly installed.
3. Ensure compiler supports relevant C++23 features - Consult here: https://en.cppreference.com/w/cpp/compiler_support/23
