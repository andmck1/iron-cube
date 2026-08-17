# Iron Cube

A Vulkan-based rendering engine built with modern C++ practices.

This application was built as part of my Masters research project into the 
comparative benefit of mesh shaders compared to traditional techniques on the
Compute Games Technology MSc at the University of Abertay.

## Prerequisites

- Windows 11 or Wayland-supported Linux
- CMake 4.3.3+
- Ninja build system 1.13.2
- GCC 13+ (or Clang 15+)
- Vulkan SDK (for runtime) 1.4.350

## Building

This has been built for Linux but has been tested and runs on Windows.

```bash
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Running

Two builds have been provided, one for Linux and one for Windows. These
have been zipped so must be unzipped before running.

To run the application on Linux:

```bash
./build/linux/bin/iron-cube
```

And on Windows:

```pwsh
.\build\windows\bin\iron-cube.exe
```

To see available CLI commands pass `--help`:

```bash
./build/bin/iron-cube --help
```

## Contributors
- Alastair Hamilton
- - Email: alastair.amhamilton@icloud.com
- - Student contact: s2503450@abertay.ac.uk

## AI Disclaimer
AI was used actively in this repo only for the renderer using 
[Pi](https://pi.dev) and [LMStudio](https://lmstudio.ai) with 
[Qwen 3.6](https://qwen.ai/blog?id=qwen3.6) and this is what the 
[`AGENT.md`](./AGENT.md) file was for. This was done to explore and understand 
how agentic AI can be applied to graphics programming and software development 
and the only other applications of using AI was intended for research and 
debugging. There are also some other minor applications, such as the build
system and the blender scripts for LOD generation. Any application of AI
has been appropriately labelled in the code.

## Honourable mentions
- The [Vulkan docs](https://docs.vulkan.org/tutorial/latest/00_Introduction.html)
