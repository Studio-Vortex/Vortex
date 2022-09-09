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
	Math::vec4 m_GridColor;
	int m_GridScale;

	struct ProfileResult
	{
		const char* Name;
		float Time;
	};

	std::vector<ProfileResult> m_ProfileResults;

	Math::vec4 m_SquareColor;
};