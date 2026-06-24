#include "collisions.h"

Obb CreateObb(const ObjModel& model, const glm::mat4& transform)
{
    Obb result;

    glm::vec3 local_center = (model.min_bounds + model.max_bounds) * 0.5f;

    glm::vec3 local_extents = (model.max_bounds - model.min_bounds) * 0.5f;


    glm::vec4 world_center = transform * glm::vec4(local_center, 1.0f);

    result.center = glm::vec3(world_center);

    result.half_extents = local_extents *
        glm::vec3(
            glm::length(glm::vec3(transform[0])),
            glm::length(glm::vec3(transform[1])),
            glm::length(glm::vec3(transform[2]))
        );

    result.axis[0] = glm::normalize(glm::vec3(transform[0]));
    result.axis[1] = glm::normalize(glm::vec3(transform[1]));
    result.axis[2] = glm::normalize(glm::vec3(transform[2]));

    return result;
}

bool Intersects(const Obb& a, const Obb& b)
{
    constexpr float EPSILON = 1e-6f;

    glm::mat3 R;
    glm::mat3 AbsR;

    // Rotation matrix from B's coordinate frame into A's frame
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            R[i][j] = glm::dot(a.axis[i], b.axis[j]);
            AbsR[i][j] = fabs(R[i][j]) + EPSILON;
        }
    }

    // Translation vector from A to B expressed in A's coordinate frame
    glm::vec3 t = b.center - a.center;

    t = glm::vec3(
        glm::dot(t, a.axis[0]),
        glm::dot(t, a.axis[1]),
        glm::dot(t, a.axis[2])
    );


    float ra;
    float rb;


    // Test axes of A
    for (int i = 0; i < 3; ++i)
    {
        ra = a.half_extents[i];

        rb =
            b.half_extents[0] * AbsR[i][0] +
            b.half_extents[1] * AbsR[i][1] +
            b.half_extents[2] * AbsR[i][2];

        if (fabs(t[i]) > ra + rb)
            return false;
    }


    // Test axes of B
    for (int i = 0; i < 3; ++i)
    {
        ra =
            a.half_extents[0] * AbsR[0][i] +
            a.half_extents[1] * AbsR[1][i] +
            a.half_extents[2] * AbsR[2][i];

        rb = b.half_extents[i];

        float distance =
            fabs(
                t[0] * R[0][i] +
                t[1] * R[1][i] +
                t[2] * R[2][i]
            );

        if (distance > ra + rb)
            return false;
    }


    // Test cross products of A's axes and B's axes
    // L = A0 x B0
    ra =
        a.half_extents[1] * AbsR[2][0] +
        a.half_extents[2] * AbsR[1][0];

    rb =
        b.half_extents[1] * AbsR[0][2] +
        b.half_extents[2] * AbsR[0][1];

    if (fabs(t[2] * R[1][0] - t[1] * R[2][0]) > ra + rb)
        return false;


    // L = A0 x B1
    ra =
        a.half_extents[1] * AbsR[2][1] +
        a.half_extents[2] * AbsR[1][1];

    rb =
        b.half_extents[0] * AbsR[0][2] +
        b.half_extents[2] * AbsR[0][0];

    if (fabs(t[2] * R[1][1] - t[1] * R[2][1]) > ra + rb)
        return false;


    // L = A0 x B2
    ra =
        a.half_extents[1] * AbsR[2][2] +
        a.half_extents[2] * AbsR[1][2];

    rb =
        b.half_extents[0] * AbsR[0][1] +
        b.half_extents[1] * AbsR[0][0];

    if (fabs(t[2] * R[1][2] - t[1] * R[2][2]) > ra + rb)
        return false;


    // L = A1 x B0
    ra =
        a.half_extents[0] * AbsR[2][0] +
        a.half_extents[2] * AbsR[0][0];

    rb =
        b.half_extents[1] * AbsR[1][2] +
        b.half_extents[2] * AbsR[1][1];

    if (fabs(t[0] * R[2][0] - t[2] * R[0][0]) > ra + rb)
        return false;


    // L = A1 x B1
    ra =
        a.half_extents[0] * AbsR[2][1] +
        a.half_extents[2] * AbsR[0][1];

    rb =
        b.half_extents[0] * AbsR[1][2] +
        b.half_extents[2] * AbsR[1][0];

    if (fabs(t[0] * R[2][1] - t[2] * R[0][1]) > ra + rb)
        return false;


    // L = A1 x B2
    ra =
        a.half_extents[0] * AbsR[2][2] +
        a.half_extents[2] * AbsR[0][2];

    rb =
        b.half_extents[0] * AbsR[1][1] +
        b.half_extents[1] * AbsR[1][0];

    if (fabs(t[0] * R[2][2] - t[2] * R[0][2]) > ra + rb)
        return false;


    // L = A2 x B0
    ra =
        a.half_extents[0] * AbsR[1][0] +
        a.half_extents[1] * AbsR[0][0];

    rb =
        b.half_extents[1] * AbsR[2][2] +
        b.half_extents[2] * AbsR[2][1];

    if (fabs(t[1] * R[0][0] - t[0] * R[1][0]) > ra + rb)
        return false;


    // L = A2 x B1
    ra =
        a.half_extents[0] * AbsR[1][1] +
        a.half_extents[1] * AbsR[0][1];

    rb =
        b.half_extents[0] * AbsR[2][2] +
        b.half_extents[2] * AbsR[2][0];

    if (fabs(t[1] * R[0][1] - t[0] * R[1][1]) > ra + rb)
        return false;


    // L = A2 x B2
    ra =
        a.half_extents[0] * AbsR[1][2] +
        a.half_extents[1] * AbsR[0][2];

    rb =
        b.half_extents[0] * AbsR[2][1] +
        b.half_extents[1] * AbsR[2][0];

    if (fabs(t[1] * R[0][2] - t[0] * R[1][2]) > ra + rb)
        return false;


    // No separating axis found
    return true;
}