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
- **NVIDIA CUDA Toolkit 12/13**.

## 🛠️ Build from Scratch (One-Click)

If you have the prerequisites installed, you can build everything (including dependencies) using the automated script:

1.  Open **PowerShell** as Administrator.
2.  Run the build script:
    ```powershell
    .\build.ps1
    ```

### Prerequisites for Build from Scratch
If the script reports missing tools, download them here:
- [Visual Studio 2022 Community](https://visualstudio.microsoft.com/downloads/) (Select "C++ Desktop Development")
- [CMake](https://cmake.org/download/)
- [Ninja](https://github.com/ninja-build/ninja/releases) (Add to PATH)
- [Git](https://git-scm.com/downloads)
- [NVIDIA CUDA Toolkit](https://developer.nvidia.com/cuda-downloads)

## Manual Build Instructions

1.  **Configure**:
    ```powershell
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    ```
2.  **Build**:
    ```powershell
    cmake --build build --config Release
    ```
3.  **Run**:
    ```powershell
    .\build\agentic.exe
    ```

## Usage

Run the agent from the root directory:

```powershell
.\build\agentic.exe
```

### Command Line Arguments
- `model_path`: (Optional) Path to a specific `.gguf` model file. Defaults to Nemotron.
- `initial_prompt`: (Optional) A prompt to start the conversation automatically.

Example:
```powershell
.\build\agentic.exe models/my-model.gguf "Hello, how are you?"
```

## Features
- **GPU Acceleration**: 100% offload to NVIDIA GPUs via CUDA.
- **Smart Memory**: Persistent conversation history using re-evaluation.
- **Auto-Download**: Automatically downloads the optimized Nemotron-3-Nano model on first run.
- **Nemotron Optimized**: Uses official prompt templates for high-quality responses.
- **Clean Interface**: Debug logs are suppressed and written to `log.txt`.

## Known Issues

- **MSVC Stack Size**: Configured with 256MB stack to handle complex BPE tokenizers.

## License
Private Development.
