#pragma once

#include "Sparky/Core/Core.h"
#include "Sparky/Events/Event.h"
#include "Sparky/Core/TimeStep.h"

namespace Sparky {

	class SPARKY_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(TimeStep ts) {}
		virtual void OnGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }

	protected:
		std::string m_DebugName;
	};

}
