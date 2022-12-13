#pragma once

#include "Sparky/Core/Base.h"

#include "Sparky/Core/Layer.h"
#include "Sparky/Events/Event.h"

namespace Sparky {

	class SPARKY_API GuiLayer : public Layer
	{
	public:
		GuiLayer();
		~GuiLayer();

		void OnAttach() override;
		void OnDetach() override;

		void OnEvent(Event& event) override;

		void BeginFrame();
		void EndFrame();

		void BlockEvents(bool block) { m_BlockEvents = block; }

		uint32_t GetActiveLayerID() const;

		void SetDarkThemeColors();
		void SetLightGrayThemeColors();

	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};


}

namespace ImGui {

	void TextCentered(const char* text, float y = 0.0f, ...);

}
