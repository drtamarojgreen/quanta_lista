#include "ModelBackend.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <array>
#include <memory>
#include <algorithm>

ModelBackend::ModelBackend() {
    load_config();
}

void ModelBackend::load_config() {
    std::ifstream file(".quanta");
    if (!file.is_open()) return;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            // Trim
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            config[key] = value;
        }
    }
}

bool ModelBackend::is_available() const {
    return config.count("model.llama_cli_path") && config.count("engine.model_path");
}

std::string ModelBackend::run_model(const std::string& input) {
    if (!is_available()) {
        return "Error: Model backend not configured.";
    }

    std::string llama_path = config.at("model.llama_cli_path");
    std::string model_path = config.at("engine.model_path");

    std::stringstream cmd;
    cmd << llama_path << " -m " << model_path << " -p \"" << input << "\" -n 128 --temp 0 --no-display-prompt --log-disable --simple-io --single-turn --no-show-timings 2>/dev/null";

    std::array<char, 256> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.str().c_str(), "r"), pclose);

    if (!pipe) {
        return "Error: Failed to launch llama-cli.";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    if (result.empty()) {
        return "Error: No output from model.";
    }

    // Cleanup output
    std::string prompt_marker = "> " + input;
    size_t start_pos = result.find(prompt_marker);
    if (start_pos != std::string::npos) {
        result = result.substr(start_pos + prompt_marker.length());
    }

    size_t exit_pos = result.find("Exiting...");
    if (exit_pos != std::string::npos) {
        result = result.substr(0, exit_pos);
    }

    // Trim
    result.erase(0, result.find_first_not_of(" \t\r\n"));
    result.erase(result.find_last_not_of(" \t\r\n") + 1);

    return result;
}
