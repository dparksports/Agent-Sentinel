# LlamaCppRunner

A native C++ console application for running LLM inference using [llama.cpp](https://github.com/ggerganov/llama.cpp) (Master branch). This project bypasses C# wrappers (`LLamaSharp`) to provide direct access to the latest `llama.dll` features, such as MXFP4 support.

## Prerequisites

- **Windows 10/11** x64.
- **Visual Studio 2022/2026** with C++ Desktop Development workload.
- **CMake** (3.26+).
- **Ninja** build system.
- **NVIDIA CUDA Toolkit 12/13** (for GPU acceleration).
- **llama.cpp** source code checked out at `C:/Users/honey/mydog/llama.cpp`.

## Build Instructions

1.  **Open a Terminal** with the Visual Studio environment (e.g., "Developer Command Prompt for VS 2022").
2.  Navigate to the project directory:
    ```cmd
    cd C:\Users\honey\mydog\LlamaCppRunner
    ```
3.  **Configure** the project using CMake and Ninja:
    ```cmd
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    ```
4.  **Build** the executable:
    ```cmd
    cmake --build build
    ```

   *Note: The CMake configuration automatically copies `llama.dll` and necessary `ggml*.dll` dependencies from your `llama.cpp` build folder to the output directory.*

## Usage

Run the executable from the build directory, providing the path to a GGUF model file and an optional prompt.

```cmd
cd build
LlamaCppRunner.exe "C:\Path\To\Model.gguf" "Your prompt here"
```

### Example
```cmd
LlamaCppRunner.exe "C:\Users\honey\.lmstudio\models\lmstudio-community\NVIDIA-Nemotron-3-Nano-30B-A3B-GGUF\NVIDIA-Nemotron-3-Nano-30B-A3B-Q4_K_M.gguf" "Explain efficient code."
```

### Running from Any Directory
You can run the executable from anywhere by using its full path. The required DLLs (`llama.dll`, etc.) are in the same folder, so they will be loaded automatically.

```powershell
& "c:\Users\honey\mydog\LlamaCppRunner\build\LlamaCppRunner.exe" "C:\Path\To\Model.gguf" "Your Prompt"
```

## Known Issues

### MSVC Stack Overflow (Regex)
Some models (e.g., `gpt-oss-20b`) use complex regular expressions in their tokenizer configurations (BPE). The Microsoft Visual C++ (MSVC) standard library implementation of `std::regex` uses deep recursion, which can cause stack overflows even with minimal input.

- **Mitigation**: The `CMakeLists.txt` is configured with an **extremely large stack size (256MB)** to attempt to handle this.
- **Limitation**: For extremely complex regexes, this may still fail. In such cases, the recommendation is to compile using **Clang** on Windows instead of MSVC.

## License
Private repository.
