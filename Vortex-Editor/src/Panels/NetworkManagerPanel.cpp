#include "NetworkManagerPanel.h"

namespace Vortex {

	void NetworkManagerPanel::OnEditorAttach()
	{
		m_ServerInfo.SocketOptions.AF = AddressFamily::InternetIPv4;
		m_ServerInfo.SocketOptions.Type = ConnectionType::Udp;
		m_ServerInfo.SocketOptions.Protocol = NetworkProtocol::Udp;
		m_ServerInfo.IpAddr = IpAddress::GetLoopbackAddr();
		m_ServerInfo.PortAddr = Port(27999);
	}

	void NetworkManagerPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!IsOpen)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		Gui::Text("Server Status: ");
		Gui::SameLine();
		Gui::TextColored(m_ServerRunning ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1), m_ServerRunning ? "Running" : "Shutdown");

		{
			UI::BeginPropertyGrid();
			std::string addrStr = m_ServerInfo.IpAddr.ToString();
			if (UI::Property("IP Address", addrStr))
			{
				std::vector<std::string> parts = String::SplitString(std::string_view(addrStr.c_str(), addrStr.length()), '.');
				const bool lastPartEmpty = parts[parts.size() - 1].empty();
				if (parts.size() == 4 && !lastPartEmpty)
				{
					m_ServerInfo.IpAddr.AddressBuf[0] = (uint8_t)std::stoi(parts[0].c_str());
					m_ServerInfo.IpAddr.AddressBuf[1] = (uint8_t)std::stoi(parts[1].c_str());
					m_ServerInfo.IpAddr.AddressBuf[2] = (uint8_t)std::stoi(parts[2].c_str());
					m_ServerInfo.IpAddr.AddressBuf[3] = (uint8_t)std::stoi(parts[3].c_str());
				}
			}
			uint16_t port = m_ServerInfo.PortAddr.Address;
			if (UI::Property("Port", port))
			{
				m_ServerInfo.PortAddr.Address = port;
			}
			UI::EndPropertyGrid();
		}

		{
			Gui::BeginDisabled(m_ServerRunning);
			if (Gui::Button("Start Debug Server"))
			{
				m_ServerRunning = true;

				VX_CONSOLE_LOG_INFO("Debug server starting...");
				m_Server = Server::Create(m_ServerInfo);
			}
			Gui::EndDisabled();
		}
		
		Gui::SameLine();

		{
			Gui::BeginDisabled(!m_ServerRunning);
			if (Gui::Button("Shutdown Debug Server"))
			{
				m_ServerRunning = false;

				VX_CONSOLE_LOG_INFO("Debug server shutting down...");
				m_Server->Shutdown();
				m_Server.Reset();
			}
			Gui::EndDisabled();
		}

		Gui::End();
	}

}
