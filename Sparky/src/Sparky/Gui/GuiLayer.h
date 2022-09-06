#pragma once

#include "Sparky/Core.h"

#include "Sparky/Layer.h"
#include "Sparky/Events/Event.h"

namespace Sparky {

	class SPARKY_API GuiLayer : public Layer
	{
	public:
		GuiLayer();
		~GuiLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnGuiRender() override;

		void BeginFrame();
		void EndFrame();

	private:
		float m_Time = 0.0f;
	};

}
