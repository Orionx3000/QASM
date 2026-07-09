# QASM Engine (Quantized Agentic State Machine)

## Holistic Intent
The QASM engine is a "Glass Box" architecture for an auditable, semantic-native AI operating system. It moves away from standard chatbot paradigms by integrating a multi-agent debate (Proposer-Reviewer consensus) at the core of its execution loop. The holistic intent is to create a deterministic, progressive, and auditable reasoning system. Before any autonomous action is executed, it must be proposed by the `deep_brain`, reviewed by the `fast_brain` against a rigid cognitive scaffold, and either approved or rewritten based on the critique. This eliminates non-stop looping generation and guarantees the AI behaves deliberately.

QASM runs directly on local hardware via `llama.cpp` integration, creating an AI OS where thoughts, actions, and memory are deeply integrated natively into the host system.

## Core Components

### 1. Multi-Agent Consensus Loop (`main.cpp`)
The core autonomous loop is powered by two instantiations of the model:
- **Proposer (`deep_brain`)**: Proposes shell scripts, HTTP calls, or web search actions based on the current context and user prompt.
- **Reviewer (`fast_brain`)**: Analyzes the proposed script against a `scaffold.json` rule set. It ensures actions aren't repetitive and move the task forward progressively. If it finds violations, it rejects the script and outputs a critique, forcing the Proposer to rewrite.
Only `APPROVED` scripts are executed by the local virtual machine.

### 2. Semantic Cores (`SemanticCores.h` & `SemanticCores.cpp`)
This component wraps `llama.cpp` using the C API for high-performance native inference. It handles multiple "brain" contexts within the same memory space, enabling parallelized or asynchronous generation between the Proposer, Reviewer, and the Voice/Speech generator. It supports dynamic model loading and context buffer management.

### 3. Cognitive Scaffold (`scaffold.json` & `SystemMemory.h`)
The scaffold acts as the strict "Constitution" or standard operating procedure for the engine. It defines the allowed action formats (e.g. `BalaScript`, JSON schemas), constraints against repetition, and the required output format.
`SystemMemory` tracks long-term identifiers, boot counts, and persistent state across reboots.

### 4. Native Interface (`NativeUI.h` & `NativeUI.cpp`)
QASM features a responsive, native UI powered by Microsoft Edge WebView2. 
- It binds C++ backend logic directly to frontend JavaScript.
- The UI serves as a real-time monitor for the "Glass Box", streaming the internal debate, critiques, and executed scripts live to the user.

### 5. Agentic Action VM (`QasmParser.cpp` & `BalaScriptEngine`)
The execution engine that bridges AI thought to OS-level action. It parses semantic action blocks (like `FETCH.URL`, `WRITE.FILE`, `BASH.EXEC`) and executes them using native C++ implementations or shell spawning.

### 6. HTTP Subsystem (`HttpClient.h`)
A robust native Windows HTTP client (`WinHTTP`) that allows the AI to autonomously browse the web, scrape content, and interact with REST APIs without relying on external Python scripts or dependencies. It features secure URI parsing and protocol handling.

## Model Setup
Because the LLM weights are too large for GitHub, they are excluded from this repository. 
To run QASM, you need to download the appropriate `.gguf` model files and place them in the `models/` directory.

1. Create a directory named `models` in the root of the project.
2. Download your preferred GGUF models (e.g., Llama-3-8B-Instruct.gguf).
3. Update `storage/model_config.json` to point to the correct paths:
```json
{
  "deep_brain": {
    "path": "models/your-deep-model.gguf"
  },
  "fast_brain": {
    "path": "models/your-fast-model.gguf"
  }
}
```

## Build Requirements
- CMake 3.14+
- MSVC (Visual Studio 2022)
- Windows SDK
- `llama.cpp` dependencies (CUDA Toolkit for GPU acceleration)

## Future Roadmap
- Implementation of the `Tinaten Protocol` for semantic synthesis.
- Hardware-level perception arrays (Vision, Audio).
- Deep memory vectorization and fractal addressing (`Base-138 Math System`).
