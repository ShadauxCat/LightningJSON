#pragma once

#include <string.h>
#include <ostream>

namespace LightningJSON
{
	class string_view
	{
	public:
		string_view()
			: m_data(nullptr)
			, m_length(0)
		{}

		string_view(char const* data)
			: m_data(data)
			, m_length(strlen(data))
		{}

		string_view(char const* data, size_t length)
			: m_data(data)
			, m_length(length)
		{}

		string_view(string_view const& other) noexcept
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
	private:
		char const* m_data;
		size_t m_length;
	};
}

namespace std
{
	inline std::ostream& operator<< (std::ostream& out, LightningJSON::string_view const& t)
	{
		out.write(t.data(), t.length());
		return out;
	}
}