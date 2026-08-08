#pragma once

#include <koral.h>

class IPG_Lab4 final : public kor::Scene
{
public:
    void Initialize() override;
    void Update() override;
    void Render(kor::CommandBuffer& commandBuffer) override;
    void RenderUI() override;
};
