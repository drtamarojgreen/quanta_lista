#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <string>
#include <vector>
#include "../models/models.h"

std::string extract_string(const std::string& json_string, const std::string& key);
int extract_int(const std::string& json_string, const std::string& key);
std::vector<std::string> extract_string_vector(const std::string& json_string, const std::string& key);

Task from_json(const std::string& json_string);
std::string to_json(const Task& task);
std::string to_json(const Schedule& schedule);
Schedule schedule_from_json(const std::string& json_string);

bool isValidPath(const std::string& path);

#endif // JSON_UTILS_H
