#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace matops
{
    glm::mat4 MatrixIdentity()
    {
        return glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    glm::mat4 MatrixTranslate(float tx, float ty, float tz)
    {
        return glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            tx, ty, tz, 1.0f
        );
    }

    glm::mat4 MatrixRotateX(float angle)
    {
        float c = cos(angle);
        float s = sin(angle);

        return glm::mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, c, s, 0.0f,
            0.0f, -s, c, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    glm::mat4 MatrixRotateY(float angle)
    {
        float c = cos(angle);
        float s = sin(angle);

        return glm::mat4(
            c, 0.0f, -s, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            s, 0.0f, c, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    glm::mat4 MatrixRotateZ(float angle)
    {
        float c = cos(angle);
        float s = sin(angle);

        return glm::mat4(
            c, s, 0.0f, 0.0f,
            -s, c, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    glm::mat4 MatrixScale(float sx, float sy, float sz)
    {
        return glm::mat4(
            sx, 0.0f, 0.0f, 0.0f,
            0.0f, sy, 0.0f, 0.0f,
            0.0f, 0.0f, sz, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    glm::mat4 MatrixPerspective(float field_of_view, float aspect_ratio, float n, float f)
    {
        float t = fabs(n) * tanf(field_of_view / 2.0f);
        float b = -t;
        float r = t * aspect_ratio;
        float l = -r;

        glm::mat4 P = glm::mat4(
            n, 0.0f, 0.0f, 0.0f,
            0.0f, n, 0.0f, 0.0f,
            0.0f, 0.0f, n + f, 1.0f,
            0.0f, 0.0f, -f * n, 0.0f
        );

        glm::mat4 M = glm::mat4(
            2.0f / (r - l), 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f / (t - b), 0.0f, 0.0f,
            0.0f, 0.0f, 2.0f / (f - n), 0.0f,
            -(r + l) / (r - l),
            -(t + b) / (t - b),
            -(f + n) / (f - n),
            1.0f
        );

        return -M * P;
    }

    glm::mat4 MatrixCameraView(glm::vec4 position, glm::vec4 view_vector, glm::vec4 up_vector)
    {
        glm::vec4 w = -view_vector;
        glm::vec4 u = glm::vec4(glm::cross(glm::vec3(up_vector), glm::vec3(w)), 0.0f);

        w = glm::normalize(w);
        u = glm::normalize(u);

        glm::vec4 v = glm::vec4(glm::cross(glm::vec3(w), glm::vec3(u)), 0.0f);

        glm::vec4 origin_o(0.0f, 0.0f, 0.0f, 1.0f);

        return glm::mat4(
            u.x, v.x, w.x, 0.0f,
            u.y, v.y, w.y, 0.0f,
            u.z, v.z, w.z, 0.0f,
            -glm::dot(u, position - origin_o),
            -glm::dot(v, position - origin_o),
            -glm::dot(w, position - origin_o),
            1.0f
        );
    }
}