#pragma once

#include "Sparky/Core.h"

#include "Sparky/Layer.h"
#include "Sparky/Events/Event.h"

namespace Sparky {

	class SPARKY_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate() override;
		void OnEvent(Event& event) override;
	private:
		float m_Time = 0.0f;
	};

}
