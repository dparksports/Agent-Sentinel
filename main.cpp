#include "llama.h"
#include <iostream>
#include <vector>
#include <string>

int main(int argc, char ** argv) {
    if (argc < 2) {
        std::cerr << "Usage: LlamaCppRunner <model_path> [prompt]" << std::endl;
        return 1;
    }

    std::string model_path = argv[1];
    std::string prompt = (argc > 2) ? argv[2] : "Hello, I am a C++ application running llama.cpp!";

    // Initialize backend
    ggml_backend_load_all();

    // Load Model
    auto model_params = llama_model_default_params();
    model_params.n_gpu_layers = 99; // Try to offload all to GPU
    
    llama_model* model = llama_model_load_from_file(model_path.c_str(), model_params);
    if (!model) {
        std::cerr << "Failed to load model from " << model_path << std::endl;
        return 1;
    }

    // Tokenize Prompt
    const llama_vocab* vocab = llama_model_get_vocab(model);
    int n_prompt = -llama_tokenize(vocab, prompt.c_str(), prompt.size(), NULL, 0, true, true);
    std::vector<llama_token> prompt_tokens(n_prompt);
    if (llama_tokenize(vocab, prompt.c_str(), prompt.size(), prompt_tokens.data(), prompt_tokens.size(), true, true) < 0) {
        std::cerr << "Failed to tokenize prompt" << std::endl;
        return 1;
    }

    // Create Context
    auto ctx_params = llama_context_default_params();
    ctx_params.n_ctx = n_prompt + 128; // prompt + gen size
    ctx_params.n_batch = n_prompt;
    
    llama_context* ctx = llama_init_from_model(model, ctx_params);
    if (!ctx) {
        std::cerr << "Failed to create context" << std::endl;
        return 1;
    }

    // Initialize Sampler
    auto sparams = llama_sampler_chain_default_params();
    llama_sampler* smpl = llama_sampler_chain_init(sparams);
    llama_sampler_chain_add(smpl, llama_sampler_init_greedy());

    // Print Prompt
    std::cout << "Prompt: " << prompt << "\nOutput: " << prompt;

    // Decode Prompt
    llama_batch batch = llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());
    if (llama_decode(ctx, batch) != 0) {
        std::cerr << "Failed to decode prompt" << std::endl;
        return 1;
    }

    // Generation Loop
    int n_predict = 32;
    for (int i = 0; i < n_predict; i++) {
        llama_token new_token_id = llama_sampler_sample(smpl, ctx, -1);
        
        if (llama_vocab_is_eog(vocab, new_token_id)) {
            break;
        }

        char buf[128];
        int n = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
        std::string piece(buf, n);
        std::cout << piece << std::flush;

        batch = llama_batch_get_one(&new_token_id, 1);
        if (llama_decode(ctx, batch) != 0) {
            break;
        }
    }

    std::cout << "\n\nDone." << std::endl;

    // Cleanup
    llama_sampler_free(smpl);
    llama_free(ctx);
    llama_model_free(model);

    return 0;
}
