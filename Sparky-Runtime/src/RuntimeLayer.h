#pragma once

#include <Sparky.h>

namespace Sparky {

	class RuntimeLayer : public Layer
	{
	public:
		RuntimeLayer();
		~RuntimeLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(TimeStep delta) override;
		void OnGuiRender() override;
		void OnEvent(Event& e) override;

	private:
		bool OpenScene(const std::filesystem::path& filepath);

	private:
		SharedRef<Framebuffer> m_Framebuffer;
		SharedRef<Scene> m_RuntimeScene;
		Math::vec2 m_ViewportSize = Math::vec2();
	};

}
