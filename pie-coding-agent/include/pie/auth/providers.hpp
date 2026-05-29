#pragma once

#include <string>
#include <vector>

namespace pie::auth {

struct ProviderDescriptor {
    std::string id;
    std::string name;
    std::string env_var;
    std::string default_endpoint;
    bool supports_oauth = false;
};

inline const std::vector<ProviderDescriptor>& all_providers() {
    static const std::vector<ProviderDescriptor> providers = {
        {"anthropic", "Anthropic", "ANTHROPIC_API_KEY", "https://api.anthropic.com", true},
        {"openai", "OpenAI", "OPENAI_API_KEY", "https://api.openai.com", true},
        {"openai-codex", "OpenAI Codex", "OPENAI_API_KEY", "https://api.openai.com", true},
        {"azure-openai", "Azure OpenAI", "AZURE_OPENAI_API_KEY", "", false},
        {"deepseek", "DeepSeek", "DEEPSEEK_API_KEY", "https://api.deepseek.com", false},
        {"google-gemini", "Google Gemini", "GOOGLE_API_KEY", "https://generativelanguage.googleapis.com", false},
        {"google-vertex", "Google Vertex", "GOOGLE_APPLICATION_CREDENTIALS", "https://us-central1-aiplatform.googleapis.com", false},
        {"amazon-bedrock", "Amazon Bedrock", "AWS_ACCESS_KEY_ID", "https://bedrock-runtime.us-east-1.amazonaws.com", false},
        {"mistral", "Mistral", "MISTRAL_API_KEY", "https://api.mistral.ai", false},
        {"groq", "Groq", "GROQ_API_KEY", "https://api.groq.com", false},
        {"cerebras", "Cerebras", "CEREBRAS_API_KEY", "https://api.cerebras.ai", false},
        {"cloudflare-ai-gateway", "Cloudflare AI Gateway", "CLOUDFLARE_API_KEY", "", false},
        {"cloudflare-workers-ai", "Cloudflare Workers AI", "CLOUDFLARE_API_KEY", "", false},
        {"xai", "xAI", "XAI_API_KEY", "https://api.x.ai", false},
        {"openrouter", "OpenRouter", "OPENROUTER_API_KEY", "https://openrouter.ai/api", false},
        {"vercel-ai-gateway", "Vercel AI Gateway", "VERCEL_API_KEY", "", false},
        {"zai", "ZAI", "ZAI_API_KEY", "https://api.zai.com", false},
        {"opencode-zen", "OpenCode Zen", "OPENCODE_ZEN_API_KEY", "", false},
        {"opencode-go", "OpenCode Go", "OPENCODE_GO_API_KEY", "", false},
        {"huggingface", "Hugging Face", "HF_TOKEN", "https://api-inference.huggingface.co", false},
        {"fireworks", "Fireworks", "FIREWORKS_API_KEY", "https://api.fireworks.ai", false},
        {"together", "Together AI", "TOGETHER_API_KEY", "https://api.together.xyz", false},
        {"kimi", "Kimi For Coding", "KIMI_API_KEY", "https://api.moonshot.cn", false},
        {"minimax", "MiniMax", "MINIMAX_API_KEY", "https://api.minimax.chat", false},
        {"xiaomi-mimo", "Xiaomi MiMo", "XIAOMI_API_KEY", "https://api.xiaomi.com", false},
        {"xiaomi-mimo-cn", "Xiaomi MiMo (CN)", "XIAOMI_API_KEY", "https://api.xiaomi.cn", false},
        {"xiaomi-mimo-hk", "Xiaomi MiMo (HK)", "XIAOMI_API_KEY", "https://api.xiaomi.hk", false},
        {"xiaomi-mimo-sg", "Xiaomi MiMo (SG)", "XIAOMI_API_KEY", "https://api.xiaomi.sg", false},
        {"github-copilot", "GitHub Copilot", "GITHUB_TOKEN", "https://api.github.com/copilot", true},
    };
    return providers;
}

inline const ProviderDescriptor* find_provider(const std::string& id) {
    for (const auto& p : all_providers()) {
        if (p.id == id) return &p;
    }
    return nullptr;
}

}  // namespace pie::auth
