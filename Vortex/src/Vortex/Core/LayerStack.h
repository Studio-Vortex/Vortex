#pragma once

#include "Vortex/Core/Base.h"
#include "Layer.h"

namespace Vortex {

	class VORTEX_API LayerStack
	{
	public:
		LayerStack() = default;
		~LayerStack();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* overlay);

		inline std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		inline std::vector<Layer*>::iterator end() { return m_Layers.end(); }
		inline std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		inline std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

		inline std::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
		inline std::vector<Layer*>::const_iterator end() const { return m_Layers.end(); }
		inline std::vector<Layer*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		inline std::vector<Layer*>::const_reverse_iterator rend() const { return m_Layers.rend(); }

	private:
		std::vector<Layer*> m_Layers;
		uint32_t m_LayerInsertIndex = 0;
	
	};

}
