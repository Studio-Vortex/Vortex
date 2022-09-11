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
	void OnEvent(Sparky::Event &e) override;

private:
	Sparky::OrthographicCameraController m_CameraController;

	Sparky::SharedRef<Sparky::Texture2D> m_GridTexture;
	Sparky::SharedRef<Sparky::Texture2D> m_SpriteSheet;
	Sparky::SharedRef<Sparky::SubTexture2D> m_Stairs, m_Barrel, m_Tree;
	const Math::vec2 m_SpriteSize = Math::vec2(128.0f);

	float m_RotatedQuadRotation = 0.0f;
	float m_RotatedQuadRotationSpeed = 25.0f;
	Math::vec3 m_RotatedQuadPos{ 3.0f, -3.0f, -0.1f };

	Math::vec4 m_SquareColor;

	Math::vec4 m_GridColor;
	float m_GridScale;
};