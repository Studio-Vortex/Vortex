#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/Math/Math.h"
#include "Vortex/Core/LibraryBase.h"
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


	class VORTEX_API ShaderLibrary : public LibraryMapBase<SharedReference<Shader>, std::string>
	{
	public:
		ShaderLibrary() = default;
		~ShaderLibrary() override = default;

		void Add(const std::string& name, const SharedReference<Shader>& shader);
		void Add(const SharedReference<Shader>& shader) override;
		uint8_t Remove(const std::string& name) override;

		SharedReference<Shader> Load(const std::string& filepath);
		SharedReference<Shader> Load(const std::string& name, const std::string& filepath);

		SharedReference<Shader>& Get(const std::string& name) override;
		const SharedReference<Shader>& Get(const std::string& name) const override;
	};
}