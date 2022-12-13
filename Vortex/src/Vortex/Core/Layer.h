#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Events/Event.h"
#include "Vortex/Core/TimeStep.h"

namespace Vortex {

	class VORTEX_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(TimeStep delta) {}
		virtual void OnGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }

	protected:
		std::string m_DebugName;
	};

}
