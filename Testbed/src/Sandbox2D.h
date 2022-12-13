#pragma once

#include <Vortex.h>
#include <Vortex/Renderer/OrthographicCameraController.h>

class Sandbox2D : public Vortex::Layer
{
public:
	Sandbox2D();
	~Sandbox2D() override = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(Vortex::TimeStep ts) override;
	void OnGuiRender() override;
	void OnEvent(Vortex::Event &e) override;

private:
	Vortex::OrthographicCameraController m_CameraController;

	Vortex::SharedRef<Vortex::Texture2D> m_GridTexture;

	float m_RotatedQuadRotation = 0.0f;
	float m_RotatedQuadRotationSpeed = 25.0f;
	Math::vec3 m_RotatedQuadPos{ 3.0f, -3.0f, -0.1f };

	Math::vec4 m_SquareColor;

	Math::vec4 m_GridColor;
	Math::vec2 m_GridScale;
};