#pragma once

#include <cmath>

#include <glm/glm.hpp>

struct Camera {
    float aspect = 16.f / 9.f;
    float fov    = 80.f;
    float near   = 0.01f;
    float far    = 100.f;

    glm::vec3 position = { 0.f, 0.f, 0.f };
    glm::vec3 forward = { 0.f, 0.f, -1.f };
    glm::vec3 right = { 1.f, 0.f, 0.f };
    glm::vec3 up = { 0.f, 1.f, 0.f };

    glm::mat4 projection() const {
        const float t = std::tan(glm::radians(fov) / 2.f);

        return {
            1.f / (t * aspect), 0.f, 0.f, 0.f,
            0.f, 1.f / t, 0.f, 0.f,
            0.f, 0.f, -(far + near) / (far - near), -1.f,
            0.f, 0.f, -2.f * far * near / (far - near), 0.f
        };
    }

    glm::mat4 view() const {
        const glm::vec3 x = glm::normalize(right);
        const glm::vec3 y = glm::normalize(up);
        const glm::vec3 z = -glm::normalize(forward);

        return {
            x.x, y.x, z.x, 0.f,
            x.y, y.y, z.y, 0.f,
            x.z, y.z, z.z, 0.f,
            -glm::dot(x, position), -glm::dot(y, position), -glm::dot(z, position), 1.f
        };
    }

    void MoveForward(float distance) { 
        auto projectedForward = glm::normalize(glm::vec3 { forward.x, 0.f, forward.z });
        position += projectedForward * distance; 
    }
    void MoveUp(float distance)      { position += worldUp() * distance; }
    void MoveRight(float distance) {
        auto projectedRight = glm::normalize(glm::vec3 { right.x, 0.f, right.z });
        position += projectedRight * distance; 
    }

    void RotateOx(float amount) {
        const glm::vec3 rotated = glm::normalize(rotateAround(forward, glm::normalize(right), amount));

        if (std::abs(glm::dot(rotated, worldUp())) > 0.999f)
            return;

        forward = rotated;
        up = glm::normalize(glm::cross(right, forward));
    }

    void RotateOy(float amount) {
        forward = glm::normalize(rotateAround(forward, worldUp(), amount));
        right   = glm::normalize(rotateAround(right,   worldUp(), amount));
        up      = glm::normalize(glm::cross(right, forward));
    }

private:
    static glm::vec3 worldUp() { return { 0.f, -1.f, 0.f }; }

    static glm::vec3 rotateAround(const glm::vec3& v, const glm::vec3& axis, float angle) {
        const float c = std::cos(angle);
        const float s = std::sin(angle);
        return v * c + glm::cross(axis, v) * s + axis * glm::dot(axis, v) * (1.f - c);
    }
};
