#include "io.h"

ysen::core::Optional<ysen::core::String> ysen::fs::read_file(const core::StringView& filename)
{
	auto *handle = fopen(filename.c_str(), "rb");
	if (!handle) {
		return {};
	}

	fseek(handle, 0, SEEK_END);
	auto length = ftell(handle);
	fseek(handle, 0, SEEK_SET);

	core::String string{};
	string.resize(length + 1, 0);
	fread(string.buffer(), 1, length, handle);
	fclose(handle);
	return string;
}

bool ysen::fs::write_file(const core::StringView& filename, const core::StringView& content)
{
	auto *handle = fopen(filename.c_str(), "wb+");
	if (!handle) {
		return false;
	}

	fwrite(content.c_str(), 1, content.length(), handle);
	fclose(handle);
	return true;
}
