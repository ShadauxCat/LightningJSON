#include <cmath>
#include <stdlib.h>

#include "PoolAllocator.hpp"

#ifndef LIGHTNINGJSON_STRICT
#	define LIGHTNINGJSON_STRICT 0
#endif

namespace LightningJSON
{
	inline JSONObject const& JSONObject::operator[](StringView const& key) const
	{
		if (m_holder->m_type != JSONType::Object)
		{
			return GetEmpty();
		}

		StringData keyData(key.data(), key.length());
		auto it = m_holder->m_children.asObject.find(keyData);
		if (it == m_holder->m_children.asObject.end())
		{
			return GetEmpty();
		}

		return it->value;
	}

	inline JSONObject const& JSONObject::operator[](size_t index) const
	{
		if (m_holder->m_type != JSONType::Array || index >= m_holder->m_children.asArray.size())
		{
			return GetEmpty();
		}

		return m_holder->m_children.asArray[index];
	}

	inline JSONObject& JSONObject::operator[](StringView const& key)
	{
		StringData keyData(key.data(), key.length());
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		auto it = m_holder->m_children.asObject.find(keyData);
		if (it == m_holder->m_children.asObject.end())
		{
			keyData.CommitStorage();
			auto it2 = m_holder->m_children.asObject.CheckedInsert(keyData, JSONObject(keyData, JSONType::Empty)).iterator;
			return it2->value;
		}

		return it->value;
	}

	inline JSONObject& JSONObject::operator[](size_t index)
	{
		if (m_holder->m_type != JSONType::Array || index >= m_holder->m_children.asArray.size())
		{
			throw InvalidJSON();
		}

		return m_holder->m_children.asArray[index];
	}

	inline size_t JSONObject::Size()
	{
		switch (m_holder->m_type)
		{
		case JSONType::Object:
		{
			return m_holder->m_children.asObject.Size();
		}
		case JSONType::Array:
		{
			return size_t(m_holder->m_children.asArray.size());
		}
		case JSONType::Null:
		case JSONType::Empty:
		{
			return 0;
		}
		case JSONType::Boolean:
		case JSONType::Double:
		case JSONType::Integer:
		case JSONType::String:
		default:
		{
			return 1;
		}
		}
	}

	inline std::string JSONObject::ToJSONString(bool pretty)
	{
		std::stringstream outString;
		BuildJSONString(outString, pretty, 1);
		return outString.str();
	}

	inline JSONObject& JSONObject::PushBack(JSONObject const& token)
	{
		if (m_holder->m_type != JSONType::Array)
		{
			throw InvalidJSON();
		}
		m_holder->m_children.asArray.emplace_back(token);
		return m_holder->m_children.asArray.back();
	}

	inline JSONObject& JSONObject::PushBack(unsigned long long value)
	{
		if (m_holder->m_type != JSONType::Array)
		{
			throw InvalidJSON();
		}
		m_holder->m_children.asArray.emplace_back(JSONType::Integer, value);
		return m_holder->m_children.asArray.back();
	}

	inline JSONObject& JSONObject::PushBack(long long value)
	{
		if (m_holder->m_type != JSONType::Array)
		{
			throw InvalidJSON();
		}
		m_holder->m_children.asArray.emplace_back(JSONType::Integer, value);
		return m_holder->m_children.asArray.back();
	}

	inline JSONObject& JSONObject::PushBack(long double value)
	{
		if (m_holder->m_type != JSONType::Array)
		{
			throw InvalidJSON();
		}
		m_holder->m_children.asArray.emplace_back(JSONType::Double, value);
		return m_holder->m_children.asArray.back();
	}

	inline JSONObject& JSONObject::PushBack(bool value)
	{
		if (m_holder->m_type != JSONType::Array)
		{
			throw InvalidJSON();
		}
		m_holder->m_children.asArray.emplace_back(JSONType::Boolean, value);
		return m_holder->m_children.asArray.back();
	}

	inline JSONObject& JSONObject::PushBack(const char* const value)
	{
		if (m_holder->m_type != JSONType::Array)
		{
			throw InvalidJSON();
		}
		m_holder->m_children.asArray.emplace_back(JSONType::String, StringView(value));
		return m_holder->m_children.asArray.back();
	}

	inline JSONObject& JSONObject::PushBack(const char* const value, size_t length)
	{
		if (m_holder->m_type != JSONType::Array)
		{
			throw InvalidJSON();
		}
		m_holder->m_children.asArray.emplace_back(JSONType::String, StringView(value, length));
		return m_holder->m_children.asArray.back();
	}

	inline JSONObject& JSONObject::PushBack(StringView const& value)
	{
		if (m_holder->m_type != JSONType::Array)
		{
			throw InvalidJSON();
		}
		m_holder->m_children.asArray.emplace_back(JSONType::String, value);
		return m_holder->m_children.asArray.back();
	}

	inline JSONObject& JSONObject::PushBack(std::string const& value)
	{
		if (m_holder->m_type != JSONType::Array)
		{
			throw InvalidJSON();
		}
		m_holder->m_children.asArray.emplace_back(JSONType::String, StringView(value.data(), value.size()));
		return m_holder->m_children.asArray.back();
	}

#ifdef _LIGHTNINGJSON_SUPPORTS_STD_STRING_VIEW
	inline JSONObject& JSONObject::PushBack(std::string_view const& value)
	{
		if (m_holder->m_type != JSONType::Array)
		{
			throw InvalidJSON();
		}
		m_holder->m_children.asArray.emplace_back(JSONType::String, StringView(value.data(), value.size()));
		return m_holder->m_children.asArray.back();
	}
#endif

	inline JSONObject& JSONObject::Insert(StringView const& name, JSONObject const& token)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, token)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(StringView const& name, unsigned long long value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::Integer, value)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(StringView const& name, long long value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::Integer, value)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(StringView const& name, long double value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::Double, value)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(StringView const& name, bool value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::Boolean, value)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(StringView const& name, const char* const value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::String, StringView(value))).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(StringView const& name, const char* const value, size_t length)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::String, StringView(value, length))).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(StringView const& name, StringView const& value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::String, value)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(StringView const& name, std::string const& value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::String, value)).iterator;
		return it->value;
	}

#ifdef _LIGHTNINGJSON_SUPPORTS_STD_STRING_VIEW
	inline JSONObject& JSONObject::Insert(StringView const& name, std::string_view const& value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::String, value)).iterator;
		return it->value;
	}
#endif

#ifdef _LIGHTNINGJSON_SUPPORTS_STD_STRING_VIEW
	inline JSONObject& JSONObject::Insert(std::string_view const& name, JSONObject const& token)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, token)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(std::string_view const& name, unsigned long long value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::Integer, value)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(std::string_view const& name, long long value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::Integer, value)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(std::string_view const& name, long double value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::Double, value)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(std::string_view const& name, bool value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::Boolean, value)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(std::string_view const& name, const char* const value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::String, StringView(value))).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(std::string_view const& name, const char* const value, size_t length)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::String, StringView(value, length))).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(std::string_view const& name, StringView const& value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::String, value)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(std::string_view const& name, std::string const& value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::String, value)).iterator;
		return it->value;
	}

	inline JSONObject& JSONObject::Insert(std::string_view const& name, std::string_view const& value)
	{
		if (m_holder->m_type != JSONType::Object)
		{
			throw InvalidJSON();
		}

		StringData nameData(name.data(), name.length());
		nameData.CommitStorage();
		auto it = m_holder->m_children.asObject.CheckedInsert(nameData, JSONObject(nameData, JSONType::String, value)).iterator;
		return it->value;
	}
#endif

	inline JSONObject::JSONObject(JSONType statedType, StringView const& data)
		: m_holder(Holder::Create())
		, m_key(nullptr, 0)
	{
		::new(m_holder) Holder(statedType);
		m_holder->m_data = StringData(data.data(), data.length());
		m_holder->m_data.CommitStorage();
	}

	inline JSONObject::JSONObject(JSONType statedType, bool data)
		: m_holder(Holder::Create())
		, m_key(nullptr, 0)
	{
		::new(m_holder) Holder(statedType);
		if (data)
		{
			m_holder->m_data = StringData("true", 4);
		}
		else
		{
			m_holder->m_data = StringData("false", 5);
		}
	}

	inline JSONObject::JSONObject(JSONType statedType)
		: m_holder(Holder::Create())
		, m_key(nullptr, 0)
	{
		::new(m_holder) Holder(statedType);
	}

	inline JSONObject::JSONObject(JSONType statedType, long long data)
		: m_holder(Holder::Create())
		, m_key(nullptr, 0)
	{
		::new(m_holder) Holder(statedType);
		char buf[128];
		snprintf(buf, 128, "%lld", data);
		m_holder->m_data = StringData(buf, strlen(buf));
		m_holder->m_data.CommitStorage();
	}

	inline JSONObject::JSONObject(JSONType statedType, unsigned long long data)
		: m_holder(Holder::Create())
		, m_key(nullptr, 0)
	{
		::new(m_holder) Holder(statedType);
		char buf[128];
		snprintf(buf, 128, "%llu", data);
		m_holder->m_data = StringData(buf, strlen(buf));
		m_holder->m_data.CommitStorage();
	}

	inline JSONObject::JSONObject(JSONType statedType, long double data)
		: m_holder(Holder::Create())
		, m_key(nullptr, 0)
	{
		::new(m_holder) Holder(statedType);
		char buf[128];
		snprintf(buf, 128, "%.20Lg", data);
		m_holder->m_data = StringData(buf, strlen(buf));
		m_holder->m_data.CommitStorage();
	}

	inline JSONObject::JSONObject(StringData const& myKey, JSONType statedType, StringView const& data)
		: m_holder(Holder::Create())
		, m_key(myKey)
	{
		::new(m_holder) Holder(statedType);
		m_key.CommitStorage();
		m_holder->m_data = StringData(data.data(), data.length());
		m_holder->m_data.CommitStorage();
	}

	inline JSONObject::JSONObject(StringData const& myKey, JSONType statedType, std::string const& data)
		: m_holder(Holder::Create())
		, m_key(myKey)
	{
		::new(m_holder) Holder(statedType);
		m_key.CommitStorage();
		m_holder->m_data = StringData(data.data(), data.length());
		m_holder->m_data.CommitStorage();
	}

#ifdef _LIGHTNINGJSON_SUPPORTS_STD_STRING_VIEW
	inline JSONObject::JSONObject(StringData const& myKey, JSONType statedType, std::string_view const& data)
		: m_holder(Holder::Create())
		, m_key(myKey)
	{
		::new(m_holder) Holder(statedType);
		m_key.CommitStorage();
		m_holder->m_data = StringData(data.data(), data.length());
		m_holder->m_data.CommitStorage();
	}
#endif

	inline JSONObject::JSONObject(StringData const& myKey, JSONType statedType, bool data)
		: m_holder(Holder::Create())
		, m_key(myKey)
	{
		::new(m_holder) Holder(statedType);
		m_key.CommitStorage();
		if (data)
		{
			m_holder->m_data = StringData("true", 4);
		}
		else
		{
			m_holder->m_data = StringData("false", 5);
		}
	}

	inline JSONObject::JSONObject(StringData const& myKey, JSONType statedType)
		: m_holder(Holder::Create())
		, m_key(myKey)
	{
		::new(m_holder) Holder(statedType);
		m_key.CommitStorage();
	}

	inline JSONObject::JSONObject(StringData const& myKey, JSONType statedType, long long data)
		: m_holder(Holder::Create())
		, m_key(myKey)
	{
		::new(m_holder) Holder(statedType);
		m_key.CommitStorage();
		char buf[128];
		snprintf(buf, 128, "%lld", data);
		m_holder->m_data = StringData(buf, strlen(buf));
		m_holder->m_data.CommitStorage();
	}

	inline JSONObject::JSONObject(StringData const& myKey, JSONType statedType, unsigned long long data)
		: m_holder(Holder::Create())
		, m_key(myKey)
	{
		::new(m_holder) Holder(statedType);
		m_key.CommitStorage();
		char buf[128];
		snprintf(buf, 128, "%llu", data);
		m_holder->m_data = StringData(buf, strlen(buf));
		m_holder->m_data.CommitStorage();
	}

	inline JSONObject::JSONObject(StringData const& myKey, JSONType statedType, long double data)
		: m_holder(Holder::Create())
		, m_key(myKey)
	{
		::new(m_holder) Holder(statedType);
		m_key.CommitStorage();
		char buf[128];
		snprintf(buf, 128, "%.20Lg", data);
		m_holder->m_data = StringData(buf, strlen(buf));
		m_holder->m_data.CommitStorage();
	}

	inline void JSONObject::BuildJSONString(std::stringstream& outString, bool pretty, int tabDepth)
	{
		if (m_holder->m_type == JSONType::Object)
		{
			outString << '{';
			if (pretty)
			{
				outString << '\n';
			}
			else
			{
				outString << ' ';
			}
			bool first = true;
			for (auto kvp : m_holder->m_children.asObject)
			{
				if (kvp.value.IsEmpty())
				{
					continue;
				}

				if (!first)
				{
					outString << ',';
					if (pretty)
					{
						outString << '\n';
					}
					else
					{
						outString << ' ';
					}
				}

				if (pretty)
				{
					for (int i = 0; i < tabDepth; ++i)
					{
						outString << '\t';
					}
				}
				outString << '"';
				outString << StringView(kvp.key.c_str(), kvp.key.length());
				outString << "\" : ";
				kvp.value.BuildJSONString(outString, pretty, tabDepth + 1);
				first = false;
			}
			if (pretty)
			{
				outString << '\n';

				for (int i = 0; i < tabDepth - 1; ++i)
				{
					outString << '\t';
				}
			}
			else
			{
				outString << ' ';
			}

			outString << '}';
		}
		else if (m_holder->m_type == JSONType::Array)
		{
			outString << '[';
			if (pretty)
			{
				outString << '\n';
			}
			else
			{
				outString << ' ';
			}
			bool first = true;
			for (auto& it : m_holder->m_children.asArray)
			{
				if (it.IsEmpty())
				{
					continue;
				}
				if (!first)
				{
					outString << ',';
					if (pretty)
					{
						outString << '\n';
					}
					else
					{
						outString << ' ';
					}
				}

				if (pretty)
				{
					for (int i = 0; i < tabDepth; ++i)
					{
						outString << '\t';
					}
				}
				it.BuildJSONString(outString, pretty, tabDepth + 1);
				first = false;
			}
			if (pretty)
			{
				outString << '\n';

				for (int i = 0; i < tabDepth - 1; ++i)
				{
					outString << '\t';
				}
			}
			else
			{
				outString << ' ';
			}

			outString << ']';
		}
		else if (m_holder->m_type == JSONType::Null)
		{
			outString << "null";
		}
		else if (m_holder->m_type == JSONType::String)
		{
			outString << '"';
			outString << EscapeString(m_holder->m_data);
			outString << '"';
		}
		else
		{
			outString << StringView(m_holder->m_data.c_str(), m_holder->m_data.length());
		}
	}

	inline void JSONObject::SkipWhitespace(char const*& data)
	{
		while (*data == ' ' || *data == '\t' || *data == '\n' || *data == '\r')
			++data;
	}

	inline void JSONObject::CollectString(char const*& data)
	{
		for (;;)
		{
			++data;
			switch (*data)
			{
			case '\"':
				++data;
				return;
			case '\\':
				++data;
				break;
			}
		}
	}

	inline void JSONObject::ParseString(char const*& data)
	{
		char const* startPoint = data + 1;

		CollectString(data);

		m_holder->m_data = StringData(startPoint, data - startPoint - 1);
	}

	inline void JSONObject::ParseNumber(char const*& data)
	{
		char const* startPoint = data;

		for(;;)
		{
			switch (*data)
			{
			case '-':
			case '+':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				++data;
				break;
			}
			case '.':
			case 'e':
			case 'E':
			{
				if (m_holder->m_type == JSONType::Integer)
				{
					m_holder->m_type = JSONType::Double;
				}
				++data;
				break;
			}
			default:
			{
				m_holder->m_data = StringData(startPoint, data - startPoint);
				return;
			}
			}
		}
	}

	inline void JSONObject::ParseBool(char const*& data)
	{
		if (
			*data == 't'
#if LIGHTNINGJSON_STRICT
			&& *(data + 1) == 'r'
			&& *(data + 2) == 'u'
			&& *(data + 3) == 'e'
#endif
			)
		{
			m_holder->m_data = StringData(data, 4);
			data += 4;
		}
		else if (
			*data == 'f'
#if LIGHTNINGJSON_STRICT
			&& *(data + 1) == 'a'
			&& *(data + 2) == 'l'
			&& *(data + 3) == 's'
			&& *(data + 4) == 'e'
#endif
			)
		{
			m_holder->m_data = StringData(data, 5);
			data += 5;
		}
#if LIGHTNINGJSON_STRICT
		else
		{
			throw InvalidJSON();
		}
#endif
		return;
	}

	inline void JSONObject::ParseArray(char const*& data)
	{
		++data;

		for (;;)
		{
			SkipWhitespace(data);
			if (*data == ']')
			{
				++data;
				return;
			}
			switch (*data)
			{
			case '{':
			{
				m_holder->m_children.asArray.emplace_back(StringData(nullptr, 0), data, JSONType::Object);
				break;
			}
			case '[':
			{
				m_holder->m_children.asArray.emplace_back(StringData(nullptr, 0), data, JSONType::Array);
				break;
			}
			case '+':
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '.':
			{
				m_holder->m_children.asArray.emplace_back(StringData(nullptr, 0), data, JSONType::Integer);
				break;
			}
			case '\"':
			{
				m_holder->m_children.asArray.emplace_back(StringData(nullptr, 0), data, JSONType::String);
				break;
			}
			case 't':
			case 'f':
			{
				m_holder->m_children.asArray.emplace_back(StringData(nullptr, 0), data, JSONType::Boolean);
				break;
			}
			case 'n':
			{
				m_holder->m_children.asArray.emplace_back(JSONType::Null);
				data += 4;
				break;
			}
#if LIGHTNINGJSON_STRICT
			default:
			{
				throw InvalidJSON();
			}
#endif
			}

			SkipWhitespace(data);

#if LIGHTNINGJSON_STRICT
			if (*data != ',' && *data != ']')
			{
				throw InvalidJSON();
			}
#endif

			if (*data == ']')
			{
				++data;
				break;
			}
			++data;
		}
	}

	inline void JSONObject::ParseObject(char const*& data)
	{
		++data;

		StringData key(nullptr, 0);

		for (;;)
		{
			SkipWhitespace(data);

			if (*data == '}')
			{
				++data;
				return;
			}

#if LIGHTNINGJSON_STRICT
			if (*data != '\"')
			{
				throw InvalidJSON();
			}
#endif

			char const* keyStart = nullptr;
			char const* keyEnd = nullptr;

			keyStart = data + 1;

			CollectString(data);

			keyEnd = data - 1;
			key = StringData(keyStart, keyEnd - keyStart);

			SkipWhitespace(data);

#if LIGHTNINGJSON_STRICT
			if (*data != ':')
			{
				throw InvalidJSON();
			}
#endif

			++data;

			SkipWhitespace(data);

			switch (*data)
			{
			case '{':
			{
				m_holder->m_children.asObject.Insert(key, JSONObject(key, data, JSONType::Object));
				break;
			}
			case '[':
			{
				m_holder->m_children.asObject.Insert(key, JSONObject(key, data, JSONType::Array));
				break;
			}
			case '-':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				m_holder->m_children.asObject.Insert(key, JSONObject(key, data, JSONType::Integer));
				break;
			}
			case '\"':
			{
				m_holder->m_children.asObject.Insert(key, JSONObject(key, data, JSONType::String));
				break;
			}
			case 't':
			case 'f':
			{
				m_holder->m_children.asObject.Insert(key, JSONObject(key, data, JSONType::Boolean));
				break;
			}
			case 'n':
			{
				m_holder->m_children.asObject.Insert(key, JSONObject(JSONType::Null));
#if LIGHTNINGJSON_STRICT
				if (*(data + 1) != 'u' || *(data + 2) != 'l' || *(data + 3) != 'l')
				{
					throw InvalidJSON();
				}
#endif
				data += 4;
				break;
			}
#if LIGHTNINGJSON_STRICT
			default:
			{
				throw InvalidJSON();
			}
#endif
			}

			SkipWhitespace(data);

#if LIGHTNINGJSON_STRICT
			if (*data != ',' && *data != '}')
			{
				throw InvalidJSON();
			}
#endif

			if (*data == '}')
			{
				++data;
				break;
			}
			++data;
		}
	}

	inline JSONObject::JSONObject(StringData const& myKey, char const*& data, JSONType expectedType)
		: m_holder(Holder::Create())
		, m_key(myKey)
	{
		::new(m_holder) Holder(expectedType);
		switch (expectedType)
		{
		case JSONType::Boolean: ParseBool(data); break;
		case JSONType::Integer: ParseNumber(data); break;
		case JSONType::String: ParseString(data); break;
		case JSONType::Array: ParseArray(data); break;
		case JSONType::Object: ParseObject(data); break;
			// "Empty" and "Null" have no content to parse.
			// "Double" will never actually show up here - it will begin its life as "Integer" and grow into "Double" later!
		case JSONType::Empty: case JSONType::Double: case JSONType::Null: default: break;
		}
	}

	inline JSONObject::~JSONObject()
	{
		DecRef();
	}

	inline JSONObject::JSONObject()
		: m_holder(Holder::Create())
		, m_key(nullptr, 0)
	{
		::new(m_holder) Holder(JSONType::Empty);
	}

	inline JSONObject::JSONObject(JSONObject const& other)
		: m_holder(other.m_holder)
		, m_key(other.m_key)
	{
		IncRef();
	}

	inline JSONObject::JSONObject(StringData const& myKey, JSONObject const& other)
		: m_holder(other.m_holder)
		, m_key(myKey)
	{
		IncRef();
	}

	inline JSONObject JSONObject::ShallowCopy()
	{
		JSONObject ret(m_key, m_holder->m_type);
		Holder* newHolder = ret.m_holder;
		//We do not want to copy object children directly. They need cleanup work.
		newHolder->m_data = m_holder->m_data;
		if (newHolder->m_type == JSONType::Array)
		{
			newHolder->m_children.asArray = m_holder->m_children.asArray;
		}
		if (newHolder->m_type == JSONType::Object)
		{
			for (auto kvp = m_holder->m_children.asObject.begin(); kvp; ++kvp)
			{
				newHolder->m_children.asObject.Insert(kvp->key, JSONObject(kvp->key, kvp->value));
			}
		}
		return ret;
	}

	inline JSONObject JSONObject::DeepCopy()
	{
		JSONObject ret(m_key, m_holder->m_type);
		Holder* newHolder = ret.m_holder;
		//We do not want to copy object children directly. They need cleanup work.
		newHolder->m_data = m_holder->m_data;
		if (newHolder->m_type == JSONType::Array)
		{
			for (int i = 0; i < m_holder->m_children.asArray.size(); ++i)
			{
				newHolder->m_children.asArray.emplace_back(m_holder->m_children.asArray[i].DeepCopy());
			}
		}
		if (newHolder->m_type == JSONType::Object)
		{
			for (auto kvp = m_holder->m_children.asObject.begin(); kvp; ++kvp)
			{
				newHolder->m_children.asObject.Insert(kvp->key, kvp->value.DeepCopy());
			}
		}
		return ret;
	}

	inline JSONObject& JSONObject::operator=(JSONObject const& other)
	{
		DecRef();
		m_holder = other.m_holder;
		IncRef();
		return *this;
	}

	inline JSONObject::iterator JSONObject::begin()
	{
		if (m_holder->m_type == JSONType::Array)
		{
			return iterator(m_holder->m_children.asArray.begin(), m_holder->m_children.asArray.begin(), m_holder->m_children.asArray.end());
		}
		else if (m_holder->m_type == JSONType::Object)
		{
			return iterator(m_holder->m_children.asObject.begin());
		}
		return iterator(nullptr);
	}

	inline JSONObject::iterator JSONObject::end()
	{
		if (m_holder->m_type == JSONType::Array)
		{
			return iterator(m_holder->m_children.asArray.end(), m_holder->m_children.asArray.begin(), m_holder->m_children.asArray.end());
		}
		else if (m_holder->m_type == JSONType::Object)
		{
			return iterator(m_holder->m_children.asObject.end());
		}
		return iterator(nullptr);
	}


	inline JSONObject::const_iterator JSONObject::begin() const
	{
		return cbegin();
	}

	inline JSONObject::const_iterator JSONObject::end() const
	{
		return cend();
	}

	inline JSONObject::const_iterator JSONObject::cbegin() const
	{
		if (m_holder->m_type == JSONType::Array)
		{
			return const_iterator(m_holder->m_children.asArray.begin(), m_holder->m_children.asArray.begin(), m_holder->m_children.asArray.end());
		}
		else if (m_holder->m_type == JSONType::Object)
		{
			return const_iterator(m_holder->m_children.asObject.begin());
		}
		return const_iterator(nullptr);
	}

	inline JSONObject::const_iterator JSONObject::cend() const
	{
		if (m_holder->m_type == JSONType::Array)
		{
			return const_iterator(m_holder->m_children.asArray.end(), m_holder->m_children.asArray.begin(), m_holder->m_children.asArray.end());
		}
		else if (m_holder->m_type == JSONType::Object)
		{
			return const_iterator(m_holder->m_children.asObject.end());
		}
		return const_iterator(nullptr);
	}

	inline long long JSONObject::ToInt(StringData const& str)
	{
		long long result = 0;
		size_t index = 0;
		bool negative = false;
		char const* const data = str.c_str();
		size_t const length = str.length();

		if (*data == '-')
		{
			negative = true;
			++index;
		}
		for (; index < length; ++index)
		{
			result *= 10;
			result += ((int)(data[index]) - 48);
		}

		if (negative)
		{
			result *= -1;
		}

		return result;
	}

	inline unsigned long long JSONObject::ToUInt(StringData const& str)
	{
		unsigned long long result = 0;
		size_t index = 0;

		char const* const data = str.c_str();
		size_t const length = str.length();

		for (; index < length; ++index)
		{
			result *= 10;
			result += ((int)(data[index]) - 48);
		}

		return result;
	}

	inline long double JSONObject::ToDouble(StringData const& str)
	{
		long double result = 0;
		size_t index = 0;
		bool negative = false;
		double fractionSize = 1.0;
		bool inFraction = false;
		char const* const data = str.c_str();
		size_t const length = str.length();

		bool exp = false;
		double expVal = 0;
		bool expNegative = false;

		if (*data == '-')
		{
			negative = true;
			++index;
		}
		for (; index < length; ++index)
		{
			char c = data[index];
			if (c == '.')
			{
				inFraction = true;
				continue;
			}
			if (c == 'e' || c == 'E')
			{
				exp = true;
				if (index != length - 1)
				{
					if (data[index + 1] == '-')
					{
						++index;
						expNegative = true;
					}
					else if (data[index + 1] == '+')
					{
						++index;
					}
					continue;
				}
				continue;
			}
			if (exp)
			{
				expVal *= 10;
				expVal += ((int)(data[index]) - 48);
			}
			else
			{
				result *= 10;
				result += ((int)(data[index]) - 48);
				if (inFraction)
				{
					fractionSize *= 10.0;
				}
			}
		}

		if (negative)
		{
			result *= -1;
		}

		result /= fractionSize;

		if (exp)
		{
			double mult = pow(10.0, expVal);
			if (expNegative)
			{
				result /= mult;
			}
			else
			{
				result *= mult;
			}
		}

		return result;
	}

	inline bool JSONObject::ToBool(StringData const& str)
	{
		char const* const data = str.c_str();
		size_t const length = str.length();
		if (
			length == 4 &&
			data[0] == 't' &&
			data[1] == 'r' &&
			data[2] == 'u' &&
			data[3] == 'e'
			)
		{
			return true;
		}
		return false;
	}

	inline std::string JSONObject::EscapeString(StringData const& str)
	{
		std::stringstream builder;
		const char* data = str.c_str();
		const size_t length = str.length();
		for (size_t index = 0; index < length; ++index)
		{
			switch (data[index])
			{
			case '\"': builder << "\\\""; break;
			case '\\': builder << "\\\\"; break;
			case '\b': builder << "\\b"; break;
			case '\f': builder << "\\f"; break;
			case '\n': builder << "\\n"; break;
			case '\r': builder << "\\r"; break;
			case '\t': builder << "\\t"; break;
			default: builder << data[index]; break;
			}
		}
		return builder.str();
	}

	inline std::string JSONObject::UnescapeString(StringData const& str)
	{
		std::stringstream builder;
		bool inEscape = false;
		const char* data = str.c_str();
		const size_t length = str.length();
		for (size_t index = 0; index < length; ++index)
		{
			if (inEscape)
			{
				switch (data[index])
				{
				case '\"': builder << '\"'; break;
				case '\\': builder << '\\'; break;
				case '/': builder << '/'; break;
				case 'b': builder << '\b'; break;
				case 'f': builder << '\f'; break;
				case 'n': builder << '\n'; break;
				case 'r': builder << '\r'; break;
				case 't': builder << '\t'; break;
				case 'u':
				{
					++index;
					char digits[5];
					memcpy(digits, &data[index], 5);
					digits[4] = '\0';
					union UnicodeChar
					{
						int32_t asInt;
						char asChar[4];
					} ch;
					ch.asInt = strtol(digits, nullptr, 16);

					if (ch.asInt < 0x80) {
						builder << ch.asChar[0];
					}
					else if (ch.asInt < 0x800) {
						builder << char((ch.asInt >> 6) | 0xC0);
						builder << char((ch.asInt & 0x3F) | 0x80);
					}
					else if (ch.asInt < 0x10000) {
						builder << char((ch.asInt >> 12) | 0xE0);
						builder << char(((ch.asInt >> 6) & 0x3F) | 0x80);
						builder << char((ch.asInt & 0x3F) | 0x80);
					}
					else if (ch.asInt < 0x110000) {
						builder << char((ch.asInt >> 18) | 0xF0);
						builder << char(((ch.asInt >> 12) & 0x3F) | 0x80);
						builder << char(((ch.asInt >> 6) & 0x3F) | 0x80);
						builder << char((ch.asInt & 0x3F) | 0x80);
					}

					index += 3;
					break;
				}
				default: builder << '\\' << data[index]; break;
				}
				inEscape = false;
			}
			else
			{
				if (data[index] == '\\')
				{
					inEscape = true;
				}
				else
				{
					builder << data[index];
				}
			}
		}
		return builder.str();
	}

	inline JSONObject::Holder* JSONObject::Holder::Create()
	{
		typedef PoolAllocator<sizeof(JSONObject::Holder)> holderAlloc;

		return (JSONObject::Holder*)holderAlloc::alloc();
	}

	inline void JSONObject::Holder::Free(JSONObject::Holder* holder)
	{
		typedef PoolAllocator<sizeof(JSONObject::Holder)> holderAlloc;

		holder->~Holder();
		holderAlloc::free(holder);
	}

	inline JSONObject::Holder::Holder(JSONType forType)
		: m_data(nullptr, 0)
		, m_type(forType)
		, refCount(1)
	{
		switch(forType)
		{
		case JSONType::Array:
			new(&m_children.asArray) TokenList();
			m_children.asArray.reserve(16);
			break;
		case JSONType::Object:
			new(&m_children.asObject) TokenMap();
			m_children.asObject.Reserve(16);
			break;
		}
	}

	inline JSONObject::Holder::~Holder()
	{
		switch (m_type)
		{
		case JSONType::Array:
			m_children.asArray.~TokenList();
			break;
		case JSONType::Object:
			m_children.asObject.~TokenMap();
			break;
		}
	}
}
