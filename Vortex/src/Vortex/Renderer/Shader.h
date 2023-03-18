#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math/Math.h"
#include "Vortex/Core/ReferenceCounting/RefCounted.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include <unordered_map>
#include <string>

namespace Vortex {

	class VORTEX_API Shader : public RefCounted
	{
	public:
		Shader() = default;
		virtual ~Shader() = default;

		virtual void Enable() const = 0;
		virtual void Disable() const = 0;
		
		virtual void SetBool(const std::string& name, bool value) const = 0;
		virtual void SetInt(const std::string& name, int value) const = 0;
		virtual void SetInt2(const std::string& name, const Math::ivec2& vector) const = 0;
		virtual void SetInt3(const std::string& name, const Math::ivec3& vector) const = 0;
		virtual void SetInt4(const std::string& name, const Math::ivec4& vector) const = 0;
		virtual void SetIntArray(const std::string& name, int* data, uint32_t count) const = 0;
		virtual void SetFloat(const std::string& name, float value) const = 0;
		virtual void SetMat3(const std::string& name, const Math::mat3& matrix) const = 0;
		virtual void SetMat4(const std::string& name, const Math::mat4& matrix) const = 0;
		virtual void SetFloat2(const std::string& name, const Math::vec2& vector) const = 0;
		virtual void SetFloat3(const std::string& name, const Math::vec3& vector) const = 0;
		virtual void SetFloat4(const std::string& name, const Math::vec4& vector) const = 0;

		virtual void Reload() = 0;

		virtual const std::string& GetName() const = 0;

		static SharedReference<Shader> Create(const std::string& filepath);
		static SharedReference<Shader> Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc, const std::string& geometrySrc = std::string());
	};


	class VORTEX_API ShaderLibrary
	{
	public:
		ShaderLibrary() = default;
		~ShaderLibrary() = default;

		void Add(const std::string& name, const SharedReference<Shader>& shader);
		void Add(const SharedReference<Shader>& shader);
		SharedReference<Shader> Load(const std::string& filepath);
		SharedReference<Shader> Load(const std::string& name, const std::string& filepath);

		SharedReference<Shader>& Get(const std::string& name);
		const SharedReference<Shader>& Get(const std::string& name) const;

		bool Exists(const std::string& name) const;

		size_t Size() const;

		inline std::unordered_map<std::string, SharedReference<Shader>>::iterator begin() { return m_Shaders.begin(); }
		inline std::unordered_map<std::string, SharedReference<Shader>>::iterator end() { return m_Shaders.end(); }

		inline std::unordered_map<std::string, SharedReference<Shader>>::const_iterator begin() const { return m_Shaders.begin(); }
		inline std::unordered_map<std::string, SharedReference<Shader>>::const_iterator end() const { return m_Shaders.end(); }

	private:
		std::unordered_map<std::string, SharedReference<Shader>> m_Shaders;
	};
}