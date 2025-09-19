#pragma once

/*
 * Copyright (C) 2020 Jaedyn K. Draper
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4250; disable: 4996 )
#endif

#if __cplusplus < 201703L
#	error LightningJSON requires c++17 support
#endif

#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <stddef.h>

#include "Exceptions.hpp"
#include "JSONType.hpp"

#include "third-party/SkipProbe/SkipProbe.hpp"

#ifndef LIGHTNINGJSON_STRICT
#	define LIGHTNINGJSON_STRICT 0
#endif

namespace LightningJSON
{
	class StringData
	{
	public:
		StringData()
			: m_data("")
			, m_length(0)
			, m_commitData(nullptr)
		{
			//
		}

		StringData(char const* const data, size_t length)
			: m_data(data ? data : "")
			, m_length(length)
			, m_commitData(nullptr)
		{
			//
		}

		StringData(char const* const data)
			: m_data(data ? data : "")
			, m_length(data ? strlen(data) : 0)
			, m_commitData(nullptr)
		{
			//
		}

		void CommitStorage()
		{
			if (!m_commitData)
			{
				m_commitData = new char[m_length + 1];
				m_commitData[m_length] = '\0';
				memcpy(m_commitData, m_data, m_length);
				m_data = m_commitData;
			}
		}

		~StringData()
		{
			if (m_commitData)
			{
				delete[] m_commitData;
			}
		}

		StringData(StringData const& other)
			: m_data(other.m_data)
			, m_length(other.m_length)
			, m_commitData(nullptr)
		{
			if (other.m_commitData)
			{
				CommitStorage(); // Get our own copy of it, there's no refcounting
			}
		}

		StringData& operator=(StringData const& other)
		{
			if (m_commitData)
			{
				delete[] m_commitData;
			}

			m_data = other.m_data;
			m_length = other.m_length;
			m_commitData = nullptr;

			if (other.m_commitData)
			{
				CommitStorage(); // Get our own copy of it, there's no refcounting
			}

			return *this;
		}

		bool operator==(StringData const& other) const
		{
			if (m_length != other.m_length)
				return false;
			return !memcmp(m_data, other.m_data, m_length);
		}

		size_t length() const
		{
			return m_length;
		}

		char const* c_str() const
		{
			return m_data;
		}

		char operator[](size_t index) const
		{
			return m_data[index];
		}

		operator std::string() const
		{
			return std::string(m_data, m_length);
		}

		operator std::string_view() const
		{
			return std::string_view(m_data, m_length);
		}

	private:
		char const* m_data;
		size_t m_length;
		char* m_commitData;
	};
}

namespace SkipProbe
{
	template<>
	struct Hash<LightningJSON::StringData>
	{
		typedef LightningJSON::StringData argument_type;

		size_t operator()(argument_type const& str) const
		{
			return SkipProbe::CityHash(str.c_str(), str.length());
		}
	};
}

namespace LightningJSON
{
	class JSONObject
	{
	public:
		static long long ToInt(StringData const& str);
		static unsigned long long ToUInt(StringData const& str);
		static long double ToDouble(StringData const& str);
		static bool ToBool(StringData const& str);
		static std::string EscapeString(StringData const& str);
		static std::string UnescapeString(StringData const& str);

		std::string_view GetKey() const
		{
			return std::string_view(m_key.c_str(), m_key.length());
		}

		long long AsInt() const
		{
#ifdef LIGHTNINGJSON_STRICT
			if (m_holder->m_type != JSONType::Integer)
			{
				throw JSONTypeMismatch(JSONType::Integer, m_holder->m_type);
			}
#endif

			return ToInt(m_holder->m_data);
		}

		unsigned long long AsUnsigned() const
		{
#ifdef LIGHTNINGJSON_STRICT
			if (m_holder->m_type != JSONType::Integer)
			{
				throw JSONTypeMismatch(JSONType::Integer, m_holder->m_type);
			}
#endif

			return ToUInt(m_holder->m_data);
		}

		void Val(signed char& value) const
		{
			value = (signed char)(AsInt());
		}

		void Val(short& value) const
		{
			value = short(AsInt());
		}

		void Val(int& value) const
		{
			value = int(AsInt());
		}

		void Val(long& value) const
		{
			value = long(AsInt());
		}

		void Val(long long& value) const
		{
			value = (long long)(AsInt());
		}

		void Val(unsigned char& value) const
		{
			value = (unsigned char)(AsInt());
		}

		void Val(unsigned short& value) const
		{
			value = (unsigned short)(AsInt());
		}

		void Val(unsigned int& value) const
		{
			value = (unsigned int)(AsInt());
		}

		void Val(unsigned long& value) const
		{
			value = (unsigned long)(AsInt());
		}

		void Val(unsigned long long& value) const
		{
			value = (unsigned long long)(AsInt());
		}

		void Val(bool& value) const
		{
			value = AsBool();
		}

		void Val(double& value) const
		{
			value = (double)AsDouble();
		}

		void Val(float& value) const
		{
			value = float(AsDouble());
		}

		void Val(long double& value) const
		{
			value = AsDouble();
		}

		void Val(std::string& value) const
		{
			value = AsString();
		}

		void Val(char* value, size_t length) const
		{
			std::string data = AsString();
			size_t copyLength = length < data.length() ? length : data.length();
			memcpy(value, data.c_str(), copyLength);
			value[copyLength] = '\0';
		}

		std::string AsString() const
		{
#ifdef LIGHTNINGJSON_STRICT
			if (m_holder->m_type != JSONType::String)
			{
				throw JSONTypeMismatch(JSONType::String, m_holder->m_type);
			}
#endif

			return UnescapeString(m_holder->m_data);
		}

		bool AsBool() const
		{
#ifdef LIGHTNINGJSON_STRICT
			if (m_holder->m_type != JSONType::Boolean)
			{
				throw JSONTypeMismatch(JSONType::Boolean, m_holder->m_type);
			}
#endif

			return ToBool(m_holder->m_data);
		}

		long double AsDouble() const
		{
#ifdef LIGHTNINGJSON_STRICT
			if (m_holder->m_type != JSONType::Double)
			{
				throw JSONTypeMismatch(JSONType::Double, m_holder->m_type);
			}
#endif

			return ToDouble(m_holder->m_data);
		}

		JSONObject& NextSibling();

		JSONObject const& NextSibling() const;

		JSONObject const& operator[](char const* const key) const
		{
			return this->operator[](std::string_view(key, strlen(key)));
		}

		JSONObject const& operator[](std::string const& key) const
		{
			return this->operator[](std::string_view(key.c_str(), key.length()));
		}

		JSONObject const& operator[](std::string_view const& key) const;

		JSONObject const& operator[](size_t index) const;

		JSONObject& operator[](char const* const key)
		{
			return this->operator[](std::string_view(key, strlen(key)));
		}

		JSONObject& operator[](std::string const& key)
		{
			return this->operator[](std::string_view(key.c_str(), key.length()));
		}

		JSONObject& operator[](std::string_view const& key);

		JSONObject& operator[](size_t index);
		JSONObject& operator[](int index) { return operator[](size_t(index)); }

		JSONType Type() const
		{
			return m_holder->m_type;
		}

		bool IsNull() const
		{
			return (m_holder->m_type == JSONType::Null);
		}

		bool IsEmpty() const
		{
			return (m_holder->m_type == JSONType::Empty);
		}

		bool IsInteger() const
		{
			return (m_holder->m_type == JSONType::Integer);
		}

		bool IsString() const
		{
			return (m_holder->m_type == JSONType::String);
		}

		bool IsDouble() const
		{
			return (m_holder->m_type == JSONType::Double);
		}

		bool IsBool() const
		{
			return (m_holder->m_type == JSONType::Boolean);
		}

		bool IsArray() const
		{
			return (m_holder->m_type == JSONType::Array);
		}

		bool IsObject() const
		{
			return (m_holder->m_type == JSONType::Object);
		}

		bool HasKey(char const* const key) const
		{
			return IsObject() && m_holder->m_children.asObject.Contains(StringData(key, strlen(key)));
		}

		bool HasKey(char const* const key, size_t length) const
		{
			return IsObject() && m_holder->m_children.asObject.Contains(StringData(key, length));
		}

		bool HasKey(std::string const& key) const
		{
			return IsObject() && m_holder->m_children.asObject.Contains(StringData(key.data(), key.length()));
		}

		bool HasKey(std::string_view const& key) const
		{
			return IsObject() && m_holder->m_children.asObject.Contains(StringData(key.data(), key.length()));
		}

		size_t Size();

		std::string ToJSONString(bool pretty = false);

		JSONObject& PushBack(JSONObject const& token);
		JSONObject& PushBack(signed char value) { return PushBack((long long)(value)); }
		JSONObject& PushBack(short value) { return PushBack((long long)(value)); }
		JSONObject& PushBack(int value) { return PushBack((long long)(value)); }
		JSONObject& PushBack(long value) { return PushBack((long long)(value)); }
		JSONObject& PushBack(unsigned char value) { return PushBack((unsigned long long)(value)); }
		JSONObject& PushBack(unsigned short value) { return PushBack((unsigned long long)(value)); }
		JSONObject& PushBack(unsigned int value) { return PushBack((unsigned long long)(value)); }
		JSONObject& PushBack(unsigned long value) { return PushBack((unsigned long long)(value)); }
		JSONObject& PushBack(unsigned long long value);
		JSONObject& PushBack(long long value);
		JSONObject& PushBack(float value) { return PushBack((long double)(value)); }
		JSONObject& PushBack(double value) { return PushBack((long double)(value)); }
		JSONObject& PushBack(long double value);
		JSONObject& PushBack(bool value);
		JSONObject& PushBack(const char* const value);
		JSONObject& PushBack(const char* const value, size_t length);
		JSONObject& PushBack(std::string const& value);
		JSONObject& PushBack(std::string_view const& value);

		JSONObject& Insert(std::string const& name, JSONObject const& token) { return Insert(std::string_view(name.c_str(), name.length()), token); }
		JSONObject& Insert(std::string const& name, signed char value) { return Insert(std::string_view(name.c_str(), name.length()), (long long)(value)); }
		JSONObject& Insert(std::string const& name, short value) { return Insert(std::string_view(name.c_str(), name.length()), (long long)(value)); }
		JSONObject& Insert(std::string const& name, int value) { return Insert(std::string_view(name.c_str(), name.length()), (long long)(value)); }
		JSONObject& Insert(std::string const& name, long value) { return Insert(std::string_view(name.c_str(), name.length()), (long long)(value)); }
		JSONObject& Insert(std::string const& name, unsigned char value) { return Insert(std::string_view(name.c_str(), name.length()), (unsigned long long)(value)); }
		JSONObject& Insert(std::string const& name, unsigned short value) { return Insert(std::string_view(name.c_str(), name.length()), (unsigned long long)(value)); }
		JSONObject& Insert(std::string const& name, unsigned int value) { return Insert(std::string_view(name.c_str(), name.length()), (unsigned long long)(value)); }
		JSONObject& Insert(std::string const& name, unsigned long value) { return Insert(std::string_view(name.c_str(), name.length()), (unsigned long long)(value)); }
		JSONObject& Insert(std::string const& name, unsigned long long value) { return Insert(std::string_view(name.c_str(), name.length()), value); }
		JSONObject& Insert(std::string const& name, long long value) { return Insert(std::string_view(name.c_str(), name.length()), value); }
		JSONObject& Insert(std::string const& name, float value) { return Insert(std::string_view(name.c_str(), name.length()), (long double)(value)); }
		JSONObject& Insert(std::string const& name, double value) { return Insert(std::string_view(name.c_str(), name.length()), (long double)(value)); }
		JSONObject& Insert(std::string const& name, long double value) { return Insert(std::string_view(name.c_str(), name.length()), value); }
		JSONObject& Insert(std::string const& name, bool value) { return Insert(std::string_view(name.c_str(), name.length()), value); }
		JSONObject& Insert(std::string const& name, const char* const value) { return Insert(std::string_view(name.c_str(), name.length()), value); }
		JSONObject& Insert(std::string const& name, const char* const value, size_t length) { return Insert(std::string_view(name.c_str(), name.length()), value, length); }
		JSONObject& Insert(std::string const& name, std::string const& value) { return Insert(std::string_view(name.c_str(), name.length()), value); }
		JSONObject& Insert(std::string const& name, std::string_view const& value) { return Insert(std::string_view(name.c_str(), name.length()), value); }

		JSONObject& Insert(std::string_view const& name, JSONObject const& token);
		JSONObject& Insert(std::string_view const& name, signed char value) { return Insert(name, (long long)(value)); }
		JSONObject& Insert(std::string_view const& name, short value) { return Insert(name, (long long)(value)); }
		JSONObject& Insert(std::string_view const& name, int value) { return Insert(name, (long long)(value)); }
		JSONObject& Insert(std::string_view const& name, long value) { return Insert(name, (long long)(value)); }
		JSONObject& Insert(std::string_view const& name, unsigned char value) { return Insert(name, (unsigned long long)(value)); }
		JSONObject& Insert(std::string_view const& name, unsigned short value) { return Insert(name, (unsigned long long)(value)); }
		JSONObject& Insert(std::string_view const& name, unsigned int value) { return Insert(name, (unsigned long long)(value)); }
		JSONObject& Insert(std::string_view const& name, unsigned long value) { return Insert(name, (unsigned long long)(value)); }
		JSONObject& Insert(std::string_view const& name, unsigned long long value);
		JSONObject& Insert(std::string_view const& name, long long value);
		JSONObject& Insert(std::string_view const& name, float value) { return Insert(name, (long double)(value)); }
		JSONObject& Insert(std::string_view const& name, double value) { return Insert(name, (long double)(value)); }
		JSONObject& Insert(std::string_view const& name, long double value);
		JSONObject& Insert(std::string_view const& name, bool value);
		JSONObject& Insert(std::string_view const& name, const char* const value);
		JSONObject& Insert(std::string_view const& name, const char* const value, size_t length);
		JSONObject& Insert(std::string_view const& name, std::string const& value);
		JSONObject& Insert(std::string_view const& name, std::string_view const& value);

		JSONObject& Insert(char const* const name, JSONObject const& token) { return Insert(std::string_view(name, strlen(name)), token); }
		JSONObject& Insert(char const* const name, signed char value) { return Insert(std::string_view(name, strlen(name)), (long long)(value)); }
		JSONObject& Insert(char const* const name, short value) { return Insert(std::string_view(name, strlen(name)), (long long)(value)); }
		JSONObject& Insert(char const* const name, int value) { return Insert(std::string_view(name, strlen(name)), (long long)(value)); }
		JSONObject& Insert(char const* const name, long value) { return Insert(std::string_view(name, strlen(name)), (long long)(value)); }
		JSONObject& Insert(char const* const name, unsigned char value) { return Insert(std::string_view(name, strlen(name)), (unsigned long long)(value)); }
		JSONObject& Insert(char const* const name, unsigned short value) { return Insert(std::string_view(name, strlen(name)), (unsigned long long)(value)); }
		JSONObject& Insert(char const* const name, unsigned int value) { return Insert(std::string_view(name, strlen(name)), (unsigned long long)(value)); }
		JSONObject& Insert(char const* const name, unsigned long value) { return Insert(std::string_view(name, strlen(name)), (unsigned long long)(value)); }
		JSONObject& Insert(char const* const name, unsigned long long value) { return Insert(std::string_view(name, strlen(name)), value); }
		JSONObject& Insert(char const* const name, long long value) { return Insert(std::string_view(name, strlen(name)), value); }
		JSONObject& Insert(char const* const name, float value) { return Insert(std::string_view(name, strlen(name)), (long double)(value)); }
		JSONObject& Insert(char const* const name, double value) { return Insert(std::string_view(name, strlen(name)), (long double)(value)); }
		JSONObject& Insert(char const* const name, long double value) { return Insert(std::string_view(name, strlen(name)), value); }
		JSONObject& Insert(char const* const name, bool value) { return Insert(std::string_view(name, strlen(name)), value); }
		JSONObject& Insert(char const* const name, const char* const value) { return Insert(std::string_view(name, strlen(name)), value); }
		JSONObject& Insert(char const* const name, const char* const value, size_t length) { return Insert(std::string_view(name, strlen(name)), value, length); }
		JSONObject& Insert(char const* const name, std::string const& value) { return Insert(std::string_view(name, strlen(name)), value); }
		JSONObject& Insert(char const* const name, std::string_view const& value) { return Insert(std::string_view(name, strlen(name)), value); }

		static JSONObject Array()
		{
			return JSONObject(JSONType::Array, std::string_view());
		}

		static JSONObject Object()
		{
			return JSONObject(JSONType::Object, std::string_view());
		}

		static JSONObject String(char const* const data)
		{
			return JSONObject(JSONType::String, std::string_view(data));
		}

		static JSONObject String(char const* const data, size_t length)
		{
			return JSONObject(JSONType::String, std::string_view(data, length));
		}

		static JSONObject String(std::string const& data)
		{
			return JSONObject(JSONType::String, std::string_view(data.data(), data.length()));
		}

		static JSONObject String(std::string_view const& data)
		{
			return JSONObject(JSONType::String, std::string_view(data.data(), data.length()));
		}

		static JSONObject Number(signed char value) { return Number((long long)(value)); }
		static JSONObject Number(short value) { return Number((long long)(value)); }
		static JSONObject Number(int value) { return Number((long long)(value)); }
		static JSONObject Number(long value) { return Number((long long)(value)); }
		static JSONObject Number(unsigned char value) { return Number((unsigned long long)(value)); }
		static JSONObject Number(unsigned short value) { return Number((unsigned long long)(value)); }
		static JSONObject Number(unsigned int value) { return Number((unsigned long long)(value)); }
		static JSONObject Number(unsigned long value) { return Number((unsigned long long)(value)); }

		static JSONObject Number(long long data)
		{
			return JSONObject(JSONType::Integer, data);
		}

		static JSONObject Number(unsigned long long data)
		{
			return JSONObject(JSONType::Integer, data);
		}

		static JSONObject Number(long double data)
		{
			return JSONObject(JSONType::Double, data);
		}

		static JSONObject Boolean(bool data)
		{
			return JSONObject(JSONType::Boolean, data);
		}

		static JSONObject Null()
		{
			return JSONObject(JSONType::Null);
		}

		static JSONObject Empty()
		{
			return JSONObject(JSONType::Empty);
		}

		static JSONObject FromString(char const* const jsonStr, size_t const length)
		{
			return FromString(std::string_view(jsonStr, length));
		}

		static JSONObject FromString(std::string const& jsonStr)
		{
			return FromString(std::string_view(jsonStr.data(), jsonStr.length()));
		}

		static JSONObject FromString(std::string_view const& jsonStr)
		{
			if(!jsonStr.data() || jsonStr.length() == 0)
			{
				return GetEmpty();
			}

			char const* data = jsonStr.data();
			SkipWhitespace(data);
			JSONType type = JSONType::Empty;
			switch (data[0])
			{
			case '{': type = JSONType::Object; break;
			case '[': type = JSONType::Array; break;
			case '"': type = JSONType::String; break;
			case 't':
			case 'f': type = JSONType::Boolean; break;
			case 'n': type = JSONType::Null; break;
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
				type = JSONType::Integer; break;
			default:
				throw InvalidJSON();
			}
			return JSONObject(StringData(nullptr, 0), data, type);
		}

		~JSONObject();

		JSONObject();
		JSONObject(JSONObject const& other);
		JSONObject(std::nullptr_t) : JSONObject(JSONType::Null) {}
		JSONObject(signed char value) : JSONObject(JSONType::Integer, (long long)(value)) {}
		JSONObject(short value) : JSONObject(JSONType::Integer, (long long)(value)) {}
		JSONObject(int value) : JSONObject(JSONType::Integer, (long long)(value)) {}
		JSONObject(long value) : JSONObject(JSONType::Integer, (long long)(value)) {}
		JSONObject(long long value) : JSONObject(JSONType::Integer, value) {}
		JSONObject(unsigned char value) : JSONObject(JSONType::Integer, (unsigned long long)(value)) {}
		JSONObject(unsigned short value) : JSONObject(JSONType::Integer, (unsigned long long)(value)) {}
		JSONObject(unsigned int value) : JSONObject(JSONType::Integer, (unsigned long long)(value)) {}
		JSONObject(unsigned long value) : JSONObject(JSONType::Integer, (unsigned long long)(value)) {}
		JSONObject(unsigned long long value) : JSONObject(JSONType::Integer, value) {}
		JSONObject(float value) : JSONObject(JSONType::Double, (long double)(value)) {}
		JSONObject(double value) : JSONObject(JSONType::Double, (long double)(value)) {}
		JSONObject(long double value) : JSONObject(JSONType::Double, value) {}
		JSONObject(bool value) : JSONObject(JSONType::Boolean, value) {}
		JSONObject(char const* value) : JSONObject(JSONType::String, value) {}
		JSONObject(char const* value, size_t length) : JSONObject(JSONType::String, value, length) {}
		JSONObject(std::string const& value) : JSONObject(JSONType::String, value) {}
		JSONObject(std::string_view const& value) : JSONObject(JSONType::String, value) {}

		JSONObject& operator=(JSONObject const& other);
		JSONObject& operator=(std::nullptr_t) { *this = JSONObject(JSONType::Null); return *this; }
		JSONObject& operator=(signed char value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(short value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(int value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(long value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(long long value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(unsigned char value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(unsigned short value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(unsigned int value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(unsigned long value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(unsigned long long value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(float value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(double value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(long double value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(bool value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(char const* value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(std::string const& value) { *this = JSONObject(value); return *this; }
		JSONObject& operator=(std::string_view const& value) { *this = JSONObject(value); return *this; }

		StringData const& KeyData() const
		{
			return m_key;
		}

		class JSONTokenAllocator : public std::allocator<JSONObject>
		{
		public:
			template< class U, class... Args >
			void construct(U* p, Args&& ... args)
			{
				::new((void*)p) U(std::forward<Args>(args)...);
			}

			template<class U>
			struct rebind
			{
				typedef JSONTokenAllocator other;
			};
		};
		friend class JSONTokenAllocator;

		typedef SkipProbe::HashMap<StringData, JSONObject> TokenMap;
		typedef std::vector<JSONObject, JSONTokenAllocator> TokenList;

		class iterator;
		typedef iterator const const_iterator;

		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;
		const_iterator cbegin() const;
		const_iterator cend() const;

		JSONObject ShallowCopy();
		JSONObject DeepCopy();

	protected:
		JSONObject(StringData const& myKey, char const*& data, JSONType expectedType);
		JSONObject(JSONType statedType, char const* data);
		JSONObject(JSONType statedType, char const* data, size_t length);
		JSONObject(JSONType statedType, std::string const& data);
		JSONObject(JSONType statedType, std::string_view const& data);
		JSONObject(JSONType statedType, bool data);
		JSONObject(JSONType statedType, long long data);
		JSONObject(JSONType statedType, unsigned long long data);
		JSONObject(JSONType statedType);
		JSONObject(JSONType statedType, long double data);
		JSONObject(StringData const& myKey, JSONType statedType, std::string const& data);
		JSONObject(StringData const& myKey, JSONType statedType, std::string_view const& data);
		JSONObject(StringData const& myKey, JSONType statedType, bool data);
		JSONObject(StringData const& myKey, JSONType statedType, long long data);
		JSONObject(StringData const& myKey, JSONType statedType, unsigned long long data);
		JSONObject(StringData const& myKey, JSONType statedType);
		JSONObject(StringData const& myKey, JSONType statedType, long double data);
		JSONObject(StringData const& myKey, JSONObject const& other);

	private:
		void BuildJSONString(std::stringstream& outString, bool pretty, int tabDepth);
		void IncRef()
		{
			++m_holder->refCount;
		}
		void DecRef()
		{
			if (--m_holder->refCount == 0)
			{
				Holder::Free(m_holder);
			}
		}

		static void SkipWhitespace(char const*& data);
		static void CollectString(char const*& data);
		void ParseString(char const*& data);
		void ParseNumber(char const*& data);
		void ParseBool(char const*& data);
		void ParseArray(char const*& data);
		void ParseObject(char const*& data);

		struct Holder
		{
			StringData m_data;
			JSONType m_type;
			union Children
			{
				TokenMap asObject;
				TokenList asArray;
				std::nullptr_t asNull;
				// Construction and destruction handled in Holder constructor/destructor.
				Children() : asNull(nullptr) {}
				~Children() {}
			} m_children;
			int refCount;

			bool Unique() { return refCount == 1; }

			Holder(JSONType forType);
			static Holder* Create();
			static void Free(Holder* holder);

			~Holder();
		};

		Holder* m_holder;
		StringData m_key;
		static JSONObject const& GetEmpty()
		{
			static JSONObject staticEmpty(JSONType::Empty);
			return staticEmpty;
		}
	};

	class JSONObject::iterator
	{
	public:
		iterator(TokenList::iterator it, TokenList::iterator begin, TokenList::iterator end)
			: mapIter()
			, arrayBegin(begin)
			, arrayIter(it)
			, arrayEnd(end)
			, type(JSONType::Array)
		{
			//
		}
		iterator(TokenMap::Iterator it)
			: mapIter(it)
			, arrayBegin()
			, arrayIter()
			, arrayEnd()
			, type(JSONType::Object)
		{
			//
		}

		iterator(std::nullptr_t)
			: mapIter()
			, arrayBegin()
			, arrayIter()
			, arrayEnd()
			, type(JSONType::Empty)
		{
			//
		}

		JSONObject& operator*()
		{
			if (type == JSONType::Array)
			{
				return *arrayIter;
			}
			return mapIter->value;
		}

		JSONObject* operator->()
		{
			if (type == JSONType::Array)
			{
				return &*arrayIter;
			}
			return &mapIter->value;
		}


		JSONObject const& operator*() const
		{
			if (type == JSONType::Array)
			{
				return *arrayIter;
			}
			return mapIter->value;
		}

		JSONObject const* operator->() const
		{
			if (type == JSONType::Array)
			{
				return &*arrayIter;
			}
			return &mapIter->value;
		}

		JSONType Type()
		{
			return type;
		}

		size_t Index() const
		{
			if (type == JSONType::Array)
			{
				return arrayIter - arrayBegin;
			}
			return size_t(-1);
		}

		std::string_view Key() const
		{
			if (type == JSONType::Array)
			{
				return "";
			}
			return std::string_view(mapIter->key.c_str(), mapIter->key.length());
		}

		JSONObject& Value()
		{
			if (type == JSONType::Array)
			{
				return *arrayIter;
			}
			return mapIter->value;
		}

		JSONObject const& Value() const
		{
			if (type == JSONType::Array)
			{
				return *arrayIter;
			}
			return mapIter->value;
		}

		iterator& operator++()
		{
			if (type == JSONType::Array)
			{
				++arrayIter;
			}
			else
			{
				++mapIter;
			}
			return *this;
		}

		iterator operator++(int)
		{
			iterator tmp(*this);
			++(*this);
			return tmp;
		}

		bool operator==(iterator const& rhs) const
		{
			if (type == JSONType::Array)
			{
				return arrayIter == rhs.arrayIter;
			}
			return mapIter == rhs.mapIter;
		}

		bool operator!=(iterator const& rhs) const
		{
			return !this->operator==(rhs);
		}

		operator bool() const
		{
			return Valid();
		}

		bool operator!() const
		{
			return !Valid();
		}

		bool Valid() const
		{
			if (type == JSONType::Empty)
			{
				return false;
			}
			else if (type == JSONType::Array)
			{
				return arrayIter != arrayEnd;
			}
			return mapIter.Valid();
		}

		iterator Next()
		{
			return ++(*this);
		}
	private:
		JSONObject::TokenMap::Iterator mapIter;
		TokenList::iterator arrayBegin;
		TokenList::iterator arrayIter;
		TokenList::iterator arrayEnd;
		JSONType type;
	};
}

#include "LightningJSON.inl"

#ifdef _WIN32
#pragma warning( pop )
#endif
