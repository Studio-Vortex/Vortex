#pragma once

#include <Sparky.h>
#include <Sparky/Renderer/OrthographicCameraController.h>

class ExampleLayer : public Sparky::Layer
{
public:
	ExampleLayer();
	~ExampleLayer() override = default;

	void OnUpdate(Sparky::TimeStep delta) override;

	void OnGuiRender() override;

	void OnEvent(Sparky::Event& event) override;

private:
	Sparky::ShaderLibrary m_ShaderLibrary;

	Sparky::SharedRef<Sparky::VertexArray> m_TriangleVA;
	Sparky::SharedRef<Sparky::Shader> m_TriangleShader;

	Sparky::SharedRef<Sparky::VertexArray> m_SquareVA;
	Sparky::SharedRef<Sparky::Shader> m_FlatColorShader;

	Sparky::SharedRef<Sparky::Texture2D> m_Texture, m_LinuxLogo;

	Sparky::OrthographicCameraController m_CameraController;

	Math::vec3 m_SquareColor{ 0.2f, 0.3f, 0.8f };
};