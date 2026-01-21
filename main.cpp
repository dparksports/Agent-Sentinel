#include "llama.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

const std::string MODEL_NAME = "NVIDIA-Nemotron-3-Nano-30B-A3B-Q4_K_M.gguf";
const std::string MODEL_URL =
    "https://huggingface.co/lmstudio-community/"
    "NVIDIA-Nemotron-3-Nano-30B-A3B-GGUF/resolve/main/"
    "NVIDIA-Nemotron-3-Nano-30B-A3B-Q4_K_M.gguf?download=true";

void ensure_model_exists(const std::string &model_path) {
  if (fs::exists(model_path)) {
    return;
  }

  std::cout << "Model file not found at: " << model_path << std::endl;
  // Check if it is the default model we can download
  if (model_path.find(MODEL_NAME) != std::string::npos) {
    std::cout << "Downloading default model (" << MODEL_NAME
              << ")... This may take a while." << std::endl;

    // Ensure models directory exists
    fs::path path(model_path);
    if (path.has_parent_path()) {
      fs::create_directories(path.parent_path());
    }

    std::string cmd = "curl -L -o \"" + model_path + "\" \"" + MODEL_URL + "\"";
    int result = std::system(cmd.c_str());
    if (result != 0) {
      std::cerr << "Failed to download model." << std::endl;
      exit(1);
    }
    std::cout << "Download complete." << std::endl;
  } else {
    std::cerr << "Please provide a valid model path." << std::endl;
    exit(1);
  }
}

int main(int argc, char **argv) {
  std::string model_path;
  std::string initial_prompt;

  if (argc < 2) {
    // Default to local model path for interactive mode
    model_path = "models/" + MODEL_NAME;
  } else {
    model_path = argv[1];
    if (argc > 2) {
      initial_prompt = argv[2];
    }
  }

  ensure_model_exists(model_path);

  // Initialize backend
  ggml_backend_load_all();

  // Load Model
  auto model_params = llama_model_default_params();
  model_params.n_gpu_layers = 99; // Try to offload all to GPU

  llama_model *model =
      llama_model_load_from_file(model_path.c_str(), model_params);
  if (!model) {
    std::cerr << "Failed to load model from " << model_path << std::endl;
    return 1;
  }

  // Initialize Context
  auto ctx_params = llama_context_default_params();
  ctx_params.n_ctx = 4096; // Fixed context size for interactive session
  ctx_params.n_batch = 2048;

  llama_context *ctx = llama_init_from_model(model, ctx_params);
  if (!ctx) {
    std::cerr << "Failed to create context" << std::endl;
    return 1;
  }

  // Initialize Sampler
  auto sparams = llama_sampler_chain_default_params();
  llama_sampler *smpl = llama_sampler_chain_init(sparams);
  llama_sampler_chain_add(smpl, llama_sampler_init_greedy());

  const llama_vocab *vocab = llama_model_get_vocab(model);

  // Interactive Loop
  bool first_run = true;
  while (true) {
    std::string prompt;

    if (first_run && !initial_prompt.empty()) {
      prompt = initial_prompt;
      first_run = false;
    } else {
      std::cout << "\nUser: ";
      std::getline(std::cin, prompt);
    }

    if (prompt == "exit" || prompt == "quit" || prompt.empty()) {
      break;
    }

    // Clear cache for fresh query (stateless) using new memory API
    // llama_memory_seq_rm(mem, seq_id, p0, p1): seq_id=-1 (all), p0=-1 (all),
    // p1=-1
    llama_memory_seq_rm(llama_get_memory(ctx), -1, -1, -1);

    // Tokenize Prompt
    int n_prompt = -llama_tokenize(vocab, prompt.c_str(), prompt.size(), NULL,
                                   0, true, true);
    std::vector<llama_token> prompt_tokens(n_prompt);
    if (llama_tokenize(vocab, prompt.c_str(), prompt.size(),
                       prompt_tokens.data(), prompt_tokens.size(), true,
                       true) < 0) {
      std::cerr << "Failed to tokenize prompt" << std::endl;
      continue;
    }

    // Print Output Header
    std::cout << "Assistant: ";

    // Decode Prompt
    llama_batch batch =
        llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());
    if (llama_decode(ctx, batch) != 0) {
      std::cerr << "Failed to decode prompt" << std::endl;
      continue;
    }

    // Generation Loop
    int n_predict = 128; // Limit generation length
    for (int i = 0; i < n_predict; i++) {
      llama_token new_token_id = llama_sampler_sample(smpl, ctx, -1);

      if (llama_vocab_is_eog(vocab, new_token_id)) {
        break;
      }

      char buf[128];
      int n =
          llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
      std::string piece(buf, n);
      std::cout << piece << std::flush;

      batch = llama_batch_get_one(&new_token_id, 1);
      if (llama_decode(ctx, batch) != 0) {
        break;
      }
    }
    std::cout << std::endl;
  }

  // Cleanup
  llama_sampler_free(smpl);
  llama_free(ctx);
  llama_model_free(model);

  return 0;
}
