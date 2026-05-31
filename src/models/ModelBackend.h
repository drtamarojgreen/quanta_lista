#ifndef MODEL_BACKEND_H
#define MODEL_BACKEND_H

#include <string>
#include <map>

class ModelBackend {
public:
    ModelBackend();
    std::string run_model(const std::string& input);
    bool is_available() const;

private:
    std::map<std::string, std::string> config;
    void load_config();
};

#endif // MODEL_BACKEND_H
