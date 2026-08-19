#pragma once

#include <koral.h>
#include "camera.h"

class IPG_Lab4 final : public kor::Scene
{
public:
    void Initialize() override;
    void Update() override;
    void Render(kor::CommandBuffer& commandBuffer) override;
    void RenderUI() override;
    void OnResize(glm::uvec2 extent) override;

    struct Vertex {
        alignas(16) glm::vec3 position;
        alignas(16) glm::vec3 color;
    };

    using Triangle = glm::uvec3;

    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;

    kor::Resource<kor::DescriptorSet> cameraDescriptorSet;
    Camera camera {};
    kor::Resource<kor::Buffer> cameraBuffer;

    kor::Resource<kor::ComputePipeline> pipeline;

    kor::Resource<kor::DescriptorSet> renderInfoDescriptorSet;
    kor::Resource<kor::Buffer> vertexBuffer;
    kor::Resource<kor::Buffer> indexBuffer;
    kor::Resource<kor::Image> colorImage;
    kor::Resource<kor::Image> depthImage;
    kor::Resource<kor::ImageView> colorImageView;
    kor::Resource<kor::ImageView> depthImageView;

    struct Transform {
        glm::mat4x4 model;
        glm::mat4x4 viewport;
    };

    struct Viewport {
        glm::vec2 offset;
        glm::vec2 extent;
    };

    glm::vec3 meshPosition = { 0.f, 0.f, -3.f };
    glm::vec3 meshScale = { 2.f, 2.f, 2.f };
    glm::vec3 meshRotation = { 0.f, 45.f, 45.f };
};
