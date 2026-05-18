#pragma once

#include <vector>
#include <filesystem>

#include <glm/vec3.hpp>

std::vector<glm::vec3> LoadCurvePath(const std::filesystem::path& curve_filepath);
