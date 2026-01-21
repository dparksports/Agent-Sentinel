#include "llama.h"
#include <cstdio>
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

  // Redirect stderr to log.txt to avoid spamming the console with llama.cpp
  // logs We do this after ensure_model_exists so download errors are visible.
  if (freopen("log.txt", "w", stderr) == NULL) {
    std::cout << "Warning: Failed to redirect stderr to log.txt" << std::endl;
  }

  // Initialize backend
  ggml_backend_load_all();

  // Load Model
  auto model_params = llama_model_default_params();
  model_params.n_gpu_layers = 99; // Try to offload all to GPU

  llama_model *model =
      llama_model_load_from_file(model_path.c_str(), model_params);
  if (!model) {
    // Restore stderr to print fatal error if possible, or just exit.
    // Since we redirected, this goes to log.txt.
    // We should probably print to stdout as well for the user.
    std::cout << "Failed to load model from " << model_path
              << ". Check log.txt for details." << std::endl;
    std::cerr << "Failed to load model from " << model_path << std::endl;
    return 1;
  }

  const llama_vocab *vocab = llama_model_get_vocab(model);

  // Interactive Loop
  bool first_run = true;
  std::string conversation_history = "";

  while (true) {
    std::string user_input;
    std::string current_response = "";

    if (first_run && !initial_prompt.empty()) {
      user_input = initial_prompt;
      first_run = false;
    } else {
      std::cout << "\nUser: ";
      std::getline(std::cin, user_input);
    }

    if (user_input == "exit" || user_input == "quit" || user_input.empty()) {
      break;
    }

    // Update history
    conversation_history += "User: " + user_input + "\nAssistant: ";

    // Initialize Context (Recreate per turn for stateless safety)
    auto ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 4096;
    ctx_params.n_batch = 2048;

    llama_context *ctx = llama_init_from_model(model, ctx_params);
    if (!ctx) {
      std::cerr << "Failed to create context" << std::endl;
      break;
    }

    // Initialize Sampler
    auto sparams = llama_sampler_chain_default_params();
    llama_sampler *smpl = llama_sampler_chain_init(sparams);
    llama_sampler_chain_add(smpl, llama_sampler_init_greedy());

    // Tokenize Full History
    int n_tokens = -llama_tokenize(
        vocab, conversation_history.c_str(),
        static_cast<int>(conversation_history.size()), NULL, 0, true, true);
    std::vector<llama_token> history_tokens(n_tokens);
    if (llama_tokenize(vocab, conversation_history.c_str(),
                       static_cast<int>(conversation_history.size()),
                       history_tokens.data(),
                       static_cast<int>(history_tokens.size()), true,
                       true) < 0) {
      std::cerr << "Failed to tokenize history" << std::endl;
      llama_sampler_free(smpl);
      llama_free(ctx);
      continue;
    }

    // Decode Full History (No output, just priming state)
    // We process in batches of n_batch
    for (size_t i = 0; i < history_tokens.size(); i += ctx_params.n_batch) {
      int32_t n_eval =
          (int32_t)((history_tokens.size() - i < ctx_params.n_batch)
                        ? (history_tokens.size() - i)
                        : ctx_params.n_batch);

      // llama_batch_get_one expects a pointer to a single token, but we want to
      // process a sequence. Let's manually construct the batch for the sequence
      // segment.
      llama_batch seq_batch = llama_batch_init(n_eval, 0, 1);
      for (int32_t j = 0; j < n_eval; j++) {
        seq_batch.token[j] = history_tokens[i + j];
        seq_batch.pos[j] = (llama_pos)(i + j);
        seq_batch.n_seq_id[j] = 1;
        seq_batch.seq_id[j][0] = 0;
        seq_batch.logits[j] = false; // logic: we only need logits for the very
                                     // last token of the history
      }
      seq_batch.n_tokens = n_eval;

      // Enable logits for the last token of the entire history so we can sample
      // next
      if (i + n_eval == history_tokens.size()) {
        seq_batch.logits[n_eval - 1] = true;
      }

      if (llama_decode(ctx, seq_batch) != 0) {
        std::cerr << "Failed to decode batch" << std::endl;
        // cleanup
        llama_batch_free(seq_batch); // Ensure batch is freed even on error
        llama_sampler_free(smpl);
        llama_free(ctx);
        goto next_turn; // Jump to the next iteration of the while loop
      }
      llama_batch_free(seq_batch);
    }

    // Print Output Header
    std::cout << "Assistant: ";

    // Generation Loop
    int n_predict = 512; // Limit generation length

    for (int i = 0; i < n_predict; i++) {
      llama_token new_token_id = llama_sampler_sample(smpl, ctx, -1);

      if (llama_vocab_is_eog(vocab, new_token_id)) {
        std::cerr << "Debug: EOG token encountered." << std::endl;
        break;
      }

      char buf[128];
      int n =
          llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
      std::string piece(buf, n);

      std::cout << piece << std::flush;
      current_response += piece;

      // Stop sequence detection
      // Check if response starts with "User:" or contains "\nUser:"
      if (current_response.find("User:") == 0 ||
          current_response.find("\nUser:") != std::string::npos) {
        std::cerr << "Debug: Stop sequence detected." << std::endl;

        size_t stop_pos = current_response.find("User:") == 0
                              ? 0
                              : current_response.find("\nUser:");
        current_response = current_response.substr(0, stop_pos);
        break;
      }

      // Prepare next batch
      llama_batch batch = llama_batch_get_one(&new_token_id, 1);
      if (llama_decode(ctx, batch) != 0) {
        std::cerr << "Failed to decode generated token" << std::endl;
        break;
      }
    }
    std::cout << std::endl;

    // Append full response to history for next turn
    conversation_history += current_response + "\n";

    // Cleanup per turn
    llama_sampler_free(smpl);
    llama_free(ctx);
  next_turn:; // Label for goto
  }

  // Cleanup
  llama_model_free(model);

  return 0;
}
