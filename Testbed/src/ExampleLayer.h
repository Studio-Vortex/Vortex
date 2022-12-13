#pragma once

#include <Vortex.h>
#include <Vortex/Renderer/OrthographicCameraController.h>

class ExampleLayer : public Vortex::Layer
{
public:
	ExampleLayer();
	~ExampleLayer() override = default;

	void OnUpdate(Vortex::TimeStep delta) override;

	void OnGuiRender() override;

	void OnEvent(Vortex::Event& event) override;

private:
	Vortex::ShaderLibrary m_ShaderLibrary;

	Vortex::SharedRef<Vortex::VertexArray> m_TriangleVA;
	Vortex::SharedRef<Vortex::Shader> m_TriangleShader;

	Vortex::SharedRef<Vortex::VertexArray> m_SquareVA;
	Vortex::SharedRef<Vortex::Shader> m_FlatColorShader;

	Vortex::SharedRef<Vortex::Texture2D> m_Texture, m_LinuxLogo;

	Vortex::OrthographicCameraController m_CameraController;

	Math::vec3 m_SquareColor{ 0.2f, 0.3f, 0.8f };
};