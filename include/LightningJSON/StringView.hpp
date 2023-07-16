#pragma once

#include <string.h>
#include <ostream>

namespace LightningJSON
{
	class StringView
	{
	public:
		StringView()
			: m_data(nullptr)
			, m_length(0)
		{}

		StringView(char const* data)
			: m_data(data)
			, m_length(strlen(data))
		{}

		StringView(char const* data, size_t length)
			: m_data(data)
			, m_length(length)
		{}

		StringView(StringView const& other) noexcept
			: m_data(other.m_data)
			, m_length(other.m_length)
		{}

		char const* data() const
		{
			return m_data;
		}

		size_t length() const
		{
			return m_length;
		}

		size_t size() const
		{
			return m_length;
		}

		operator std::string() const
		{
			return std::string(m_data, m_length);
		}

#ifdef _LIGHTNINGJSON_SUPPORTS_STD_STRING_VIEW
		operator std::string_view() const
		{
			return std::string_view(m_data, m_length);
		}
#endif
	private:
		char const* m_data;
		size_t m_length;
	};
}

namespace std
{
	inline std::ostream& operator<< (std::ostream& out, LightningJSON::StringView const& t)
	{
		out.write(t.data(), t.length());
		return out;
	}
}