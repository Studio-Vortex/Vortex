#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/Layer.h"
#include "Vortex/Events/Event.h"

namespace Vortex {

	class VORTEX_API GuiLayer : public Layer
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
		void MergeIconFontWithLast();

	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};


}
