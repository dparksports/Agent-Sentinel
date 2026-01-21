# Agent-Sentinel

**Agent-Sentinel** is a high-performance, native C++ console application designed to serve as the LLM intelligence layer for a next-generation Windows shell. It leverages [llama.cpp](https://github.com/ggerganov/llama.cpp) to run model inference with maximum efficiency and minimal overhead.

## 🚀 Latest Release

**[Download Agent-Sentinel v1.1.0](https://github.com/dparksports/Agent-Sentinel/releases/tag/v1.1.0)**
*Includes Interactive Chat Memory, Auto-Download support, and full RTX 50-series GPU acceleration.*

## Key Features

- **Interactive Shell Mode**: Continuous chat session with state management.
- **Session Memory**: Robust history re-evaluation ensures the agent remembers context without compromising stability.
- **Auto-Model Manager**: Automatically downloads and verifies required models (e.g., NVIDIA Nemotron 30B) on first run.
- **Direct C++ Implementation**: Bypasses heavy wrappers to provide raw speed and access to bleeding-edge `llama.dll` features.
- **GPU Optimized**: Tuned for NVIDIA RTX 5090 using CUDA 12.9.

## Prerequisites

- **Windows 10/11** x64.
- **Visual Studio 2022/2026** (C++ Desktop Development).
- **CMake** (3.26+).
- **Ninja** build system.
- **NVIDIA CUDA Toolkit 12.x**.

## Build & Run

1.  **Configure**:
    ```powershell
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    ```
2.  **Build**:
    ```powershell
    cmake --build build
    ```
3.  **Run**:
    ```powershell
    .\build\AgentSentinel.exe
    ```

## Usage

Simply run the executable. If the default model is missing, Agent-Sentinel will download it for you automatically (~24.5 GB).

```powershell
.\build\AgentSentinel.exe
```

## Known Issues

- **MSVC Stack Size**: Configured with 256MB stack to handle complex BPE tokenizers.

## License
Private Development.
