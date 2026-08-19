#include "IPG_Lab4.h"
#include "transform.h"

void IPG_Lab4::Initialize()
{
    const auto shader = kor::Shader::Builder()
        .setLang<kor::Shader::Lang::eSlang>()
        .setEntryPoint("rasterizer", "draw")
        .getOrBuild();

    pipeline = kor::ComputePipeline::Builder()
        .setComputeShader(shader)
        .build();


    cameraBuffer = kor::Buffer::Builder<glm::mat4>()
        .setInstanceCount(static_cast<glm::i64>(2))
        .setIsPerFrame(true)
        .addUsage(kor::Buffer::Usage::eUniform)
        .addUsage(kor::Buffer::Usage::eStorage)
        .setType(kor::Buffer::Type::eDynamic)
        .build();

    const auto windowExtent = kor::Context::Window().getExtent();
    camera.aspect = static_cast<float>(windowExtent.x) / static_cast<float>(windowExtent.y);

    cameraBuffer->WriteAt(0, camera.view());
    cameraBuffer->WriteAt(1, camera.projection());

    float hl = 0.5f;

    vertices = {
        { { -hl, -hl, -hl }, { 0.f, 0.f, 0.f } },
        { { -hl, -hl, +hl }, { 0.f, 0.f, 1.f } },
        { { -hl, +hl, -hl }, { 0.f, 1.f, 0.f } },
        { { -hl, +hl, +hl }, { 0.f, 1.f, 1.f } },
        { { +hl, -hl, -hl }, { 1.f, 0.f, 0.f } },
        { { +hl, -hl, +hl }, { 1.f, 0.f, 1.f } },
        { { +hl, +hl, -hl }, { 1.f, 1.f, 0.f } },
        { { +hl, +hl, +hl }, { 1.f, 1.f, 1.f } },
    };

    triangles = {
        { 0, 1, 2 }, { 1, 3, 2 },
        { 4, 6, 5 }, { 5, 6, 7 },
        { 0, 4, 1 }, { 1, 4, 5 },
        { 2, 3, 7 }, { 2, 7, 6 },
        { 0, 2, 4 }, { 4, 2, 6 },
        { 1, 5, 3 }, { 3, 5, 7 },
    };

    vertexBuffer = kor::Buffer::Builder<Vertex>()
        .setInstanceCount(static_cast<glm::i64>(vertices.size()))
        .addUsage(kor::Buffer::Usage::eStorage)
        .setType(kor::Buffer::Type::eDynamic)
        .build();

    indexBuffer = kor::Buffer::Builder<Triangle>()
        .setInstanceCount(static_cast<glm::i64>(triangles.size()))
        .addUsage(kor::Buffer::Usage::eStorage)
        .setType(kor::Buffer::Type::eDynamic)
        .build();

    vertexBuffer->Write(std::span<const Vertex> { vertices });
    indexBuffer->Write(std::span<const Triangle> { triangles });

    const auto extent = windowExtent;
    colorImage = kor::Image::Builder()
        .setIsPerFrame(true)
        .setFormat(kor::Image::Format::eRGBA8_UNORM)
        .setExtent(extent)
        .addUsage(kor::Image::Usage::eStorage)
        .addUsage(kor::Image::Usage::eTransferSrc)
        .addUsage(kor::Image::Usage::eTransferDst)
        .build();

    depthImage = kor::Image::Builder()
        .setIsPerFrame(true)
        .setFormat(kor::Image::Format::eR32_UINT)
        .setExtent(extent)
        .addUsage(kor::Image::Usage::eStorage)
        .addUsage(kor::Image::Usage::eTransferDst)
        .build();

    colorImageView = kor::ImageView::Builder(colorImage).build();
    depthImageView = kor::ImageView::Builder(depthImage).build();

    cameraDescriptorSet = kor::DescriptorSet::Builder(pipeline, 0)
        .write(0, kor::Descriptor(cameraBuffer))
        .build();

    renderInfoDescriptorSet = kor::DescriptorSet::Builder(pipeline, 1)
        .write(0, kor::Descriptor(vertexBuffer))
        .write(1, kor::Descriptor(indexBuffer))
        .write(2, kor::Descriptor(colorImageView))
        .write(3, kor::Descriptor(depthImageView))
        .build();
}

float wrap(float x, float lo, float hi) { return lo + glm::mod(x - lo, hi - lo); }

void IPG_Lab4::Update()
{
    meshRotation.x += 20.f * kor::Time::FrameTime();
    meshRotation.x = wrap(meshRotation.x, -180.f, 180.f);
    if (!kor::Input::isMouseButtonHeld(kor::MouseButton::eRight)) return;

    bool changed = false;

    glm::vec3 movement = glm::vec3(0.f);

    if (kor::Input::isKeyHeld(kor::Key::eW)) movement.z += 1.f;
    if (kor::Input::isKeyHeld(kor::Key::eS)) movement.z -= 1.f;
    if (kor::Input::isKeyHeld(kor::Key::eA)) movement.x -= 1.f;
    if (kor::Input::isKeyHeld(kor::Key::eD)) movement.x += 1.f;
    if (kor::Input::isKeyHeld(kor::Key::eE)) movement.y -= 1.f;
    if (kor::Input::isKeyHeld(kor::Key::eQ)) movement.y += 1.f;

    if (movement != glm::vec3(0.f)) {
        movement = glm::normalize(movement) * kor::Time::FrameTime() * 2.f;

        camera.MoveRight(movement.x);
        camera.MoveUp(movement.y);
        camera.MoveForward(movement.z);

        changed = true;
    }

    auto mouseDelta = kor::Input::getMousePositionDelta();
    if (mouseDelta.x != 0.f) camera.RotateOy(mouseDelta.x * kor::Time::FrameTime());
    if (mouseDelta.y != 0.f) camera.RotateOx(-mouseDelta.y * kor::Time::FrameTime());
    if (glm::length(mouseDelta) != 0.f) changed = true;

    if (changed) cameraBuffer->WriteAt(0, camera.view());
}

void IPG_Lab4::Render(kor::CommandBuffer& commandBuffer)
{
    const auto extent = colorImage->getExtent();
    const auto groupCount = glm::uvec2 { (extent.x + 7) / 8, (extent.y + 7) / 8 };
    const auto triangleCount = static_cast<glm::u32>(triangles.size());

    auto model = t3D::Translate(meshPosition) * t3D::Rotate(meshRotation) * t3D::Scale(meshScale);
    auto viewport = t3D::Viewport({ 0.f, 0.f }, extent, { -1.f, -1.f }, { 2.f, 2.f });

    commandBuffer
        .ClearColorImage(colorImage, { 0.f, 0.f, 0.f, 1.f })
        .ClearColorImage(depthImage, { 0.f, 0.f, 0.f, 0.f })
        .BindComputePipeline(pipeline)
        .BindDescriptorSet(0, cameraDescriptorSet)
        .BindDescriptorSet(1, renderInfoDescriptorSet)
        .PushConstants(Transform { model, viewport })
        .Dispatch(groupCount.x, groupCount.y, triangleCount)
        .Blit(colorImage);
}

void IPG_Lab4::RenderUI()
{
    ImGui::Begin("FPS");
    ImGui::Text("%.2f", 1.f / kor::Time::FrameTime());
    ImGui::End();

    ImGui::Begin("Transform");
    ImGui::DragFloat3("position", &meshPosition.x, .1f, -5.f, 5.f, "%.1f");
    ImGui::DragFloat3("rotation", &meshRotation.x, 1.f, -180.f, 180.f, "%.1f", ImGuiSliderFlags_WrapAround);
    ImGui::DragFloat3("scale", &meshScale.x, .1f, 0.1f, 5.f, "%.1f");
    ImGui::End();


    bool cameraChanged = false;
    ImGui::Begin("Camera");
    cameraChanged |= ImGui::DragFloat("fov", &camera.fov, 1.f, 20.f, 130.f, "%.0f");
    cameraChanged |= ImGui::DragFloat("near", &camera.near, 0.1f, 0.1f, 2.f, "%.1f");
    cameraChanged |= ImGui::DragFloat("far", &camera.far, 1.f, 10.f, 100.f, "%1.f");
    ImGui::End();

    if (cameraChanged) {
        cameraBuffer->WriteAt(1, camera.projection());
    }
}

void IPG_Lab4::OnResize(glm::uvec2 extent)
{
    colorImage->Resize({ extent, 1 });
    depthImage->Resize({ extent, 1 });

    colorImageView = kor::ImageView::Builder(colorImage).build();
    depthImageView = kor::ImageView::Builder(depthImage).build();

    renderInfoDescriptorSet->Write(2, kor::Descriptor(colorImageView), 0);
    renderInfoDescriptorSet->Write(3, kor::Descriptor(depthImageView), 0);

    camera.aspect = static_cast<float>(extent.x) / static_cast<float>(extent.y);
    cameraBuffer->WriteAt(1, camera.projection());
}
