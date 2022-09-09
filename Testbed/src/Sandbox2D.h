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

	// Temp
	Sparky::SharedRef<Sparky::VertexArray> m_SquareVA;
	Sparky::SharedRef<Sparky::Shader> m_FlatColorShader;

	Math::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};