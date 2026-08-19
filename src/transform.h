#pragma once

#include <cmath>

#include <glm/glm.hpp>

namespace t3D {
    inline glm::mat4 Translate(glm::vec3 p) {
        return {
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            p.x, p.y, p.z, 1.f
        };
    }

    inline glm::mat4 Scale(glm::vec3 s) {
        return {
            s.x, 0.f, 0.f, 0.f,
            0.f, s.y, 0.f, 0.f,
            0.f, 0.f, s.z, 0.f,
            0.f, 0.f, 0.f, 1.f
        };
    }

    inline glm::mat4 _RotateOX(float alpha) {
        float c = cos(alpha);
        float s = sin(alpha);

        return {
            1.f, 0.f, 0.f, 0.f,
            0.f, c  , s  , 0.f,
            0.f, -s , c  , 0.f,
            0.f, 0.f, 0.f, 1.f
        };
    }

    inline glm::mat4 _RotateOY(float alpha) {
        float c = cos(alpha);
        float s = sin(alpha);

        return {
            c  , 0.f, s  , 0.f,
            0.f, 1.f, 0.f, 0.f,
            -s , 0.f, c  , 0.f,
            0.f, 0.f, 0.f, 1.f
        };
    }

    inline glm::mat4 _RotateOZ(float alpha) {
        float c = cos(alpha);
        float s = sin(alpha);

        return {
            c  , s  , 0.f, 0.f,
            -s , c  , 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
            0.f, 0.f, 0.f, 1.f
        };
    }

    inline glm::mat4 Rotate(glm::vec3 eulerAngles) {
        auto e = glm::radians(eulerAngles);
        return _RotateOX(e.x) * _RotateOY(e.y) * _RotateOZ(e.z);
    }

    inline glm::mat4 map(glm::vec2 src_min, glm::vec2 src_max, glm::vec2 dst_min, glm::vec2 dst_max) {
        return Translate({dst_min, 0.f}) * Scale({(dst_max - dst_min) / (src_max - src_min), 1.f}) * Translate({-src_min, 0.f});
    }

    inline glm::mat4 Viewport(
        glm::vec2 viewport_offset, glm::vec2 viewport_extent,
        glm::vec2 logic_offset = { 0.f, 0.f }, glm::vec2 logic_extent = { 16.f, 9.f }
    ) {
        glm::vec2 bottom_left = { viewport_offset.x, viewport_offset.y + viewport_extent.y };
        glm::vec2 top_right = { viewport_offset.x + viewport_extent.x, viewport_offset.y };

        return map(logic_offset, logic_offset + logic_extent, bottom_left, top_right);
    }
}