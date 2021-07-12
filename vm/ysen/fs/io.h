#pragma once
#include <ysen/core/Optional.h>
#include <ysen/core/String.h>
#include <ysen/core/StringView.h>

namespace ysen::fs {

	/*
	enum class FileOpenMode
	{
		ReadOnly,
		WriteOnly,
		ReadWrite,
	};

	enum class FileCreatePolicy
	{
		CreateIfNotExist,
		ErrorIfNotExist,
	};
	
	class File
	{
	public:
		core::SharedPtr<File> open(core::String filename, FileOpenMode mode = FileOpenMode::ReadOnly, FileCreatePolicy = FileCreatePolicy::CreateIfNotExist);

		bool is_open() const { return m_handle != nullptr; }

	private:
		core::String m_filename{};
		FileOpenMode m_file_open_mode{};
		FileCreatePolicy m_file_create_policy{};
		void *m_handle{nullptr};
	};
	*/

	extern core::Optional<core::String> read_file(const core::StringView &filename);
	extern bool write_file(const core::StringView &filename, const core::StringView& content);
}
