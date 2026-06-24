#pragma once

#include <array>

#include "obj_model.h"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

struct Obb
{
    glm::vec3 center;
    glm::vec3 half_extents;

    std::array<glm::vec3, 3> axis;
};

Obb CreateObb(const ObjModel& model, const glm::mat4& transform);

bool Intersects(const Obb& a, const Obb& b);