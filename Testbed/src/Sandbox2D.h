#pragma once

#include <Sparky.h>

class Sandbox2D : public Sparky::Layer
{
public:
	Sandbox2D();
	~Sandbox2D() override = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(Sparky::TimeStep ts) override;
	void OnGuiRender() override;
	void OnEvent(Sparky::Event& e) override;

private:
	Sparky::OrthographicCameraController m_CameraController;

	Sparky::SharedRef<Sparky::Texture2D> m_GridTexture;
	float m_RotatedQuadRotation = 0.0f;
	Math::vec3 m_RotatedQuadPos{};
	Math::vec4 m_SquareColor;
	Math::vec4 m_GridColor;
	int m_GridScale;
};