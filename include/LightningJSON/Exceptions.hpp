#pragma once

#include <exception>
#include "JSONType.hpp"

namespace LightningJSON
{
	class JSONException;
	class JSONTypeMismatch;
	class InvalidJSON;
}

class LightningJSON::JSONException : public std::exception
{
public:
	virtual ~JSONException() noexcept
	{

	}
};

class LightningJSON::JSONTypeMismatch : public JSONException
{
public:
	virtual ~JSONTypeMismatch() noexcept
	{

	}

	JSONTypeMismatch(JSONType expectedType, JSONType foundType)
		: m_expectedType(expectedType)
		, m_foundType(foundType)
	{

	}

	virtual char const* what() const noexcept override
	{
		switch (m_expectedType)
		{
		case JSONType::Empty:
			switch (m_foundType)
			{
			case JSONType::Null:    return "JSON type mismatch: Expected empty JSON token, but a token was found with type 'null'.";
			case JSONType::Integer: return "JSON type mismatch: Expected empty JSON token, but a token was found with type 'integer'.";
			case JSONType::String:  return "JSON type mismatch: Expected empty JSON token, but a token was found with type 'string'.";
			case JSONType::Boolean: return "JSON type mismatch: Expected empty JSON token, but a token was found with type 'boolean'.";
			case JSONType::Array:   return "JSON type mismatch: Expected empty JSON token, but a token was found with type 'array'.";
			case JSONType::Object:  return "JSON type mismatch: Expected empty JSON token, but a token was found with type 'object'.";
			case JSONType::Double:  return "JSON type mismatch: Expected empty JSON token, but a token was found with type 'double'.";
			case JSONType::Empty:   abort(); // This case should never happen.
			}
		case JSONType::Integer:
			switch (m_foundType)
			{
			case JSONType::Empty:   return "JSON type mismatch: Expected 'integer', but the provided JSON token was empty.";
			case JSONType::String:  return "JSON type mismatch: Expected 'integer', but the type found was 'string'.";
			case JSONType::Boolean: return "JSON type mismatch: Expected 'integer', but the type found was 'boolean'.";
			case JSONType::Array:   return "JSON type mismatch: Expected 'integer', but the type found was 'array'.";
			case JSONType::Object:  return "JSON type mismatch: Expected 'integer', but the type found was 'object'.";
			case JSONType::Double:  return "JSON type mismatch: Expected 'integer', but the type found was 'double'.";
			case JSONType::Null:    return "JSON type mismatch: Expected 'integer', but the type found was 'null'.";
			case JSONType::Integer: abort(); // This case should never happen.
			}
		case JSONType::String:
			switch (m_foundType)
			{
			case JSONType::Empty:   return "JSON type mismatch: Expected 'string', but the provided JSON token was empty.";
			case JSONType::Integer: return "JSON type mismatch: Expected 'string', but the type found was 'integer'.";
			case JSONType::Boolean: return "JSON type mismatch: Expected 'string', but the type found was 'boolean'.";
			case JSONType::Array:   return "JSON type mismatch: Expected 'string', but the type found was 'array'.";
			case JSONType::Object:  return "JSON type mismatch: Expected 'string', but the type found was 'object'.";
			case JSONType::Double:  return "JSON type mismatch: Expected 'string', but the type found was 'double'.";
			case JSONType::Null:    return "JSON type mismatch: Expected 'string', but the type found was 'null'.";
			case JSONType::String:  abort(); // This case should never happen.
			}
		case JSONType::Boolean:
			switch (m_foundType)
			{
			case JSONType::Empty:   return "JSON type mismatch: Expected 'boolean', but the provided JSON token was empty.";
			case JSONType::Integer: return "JSON type mismatch: Expected 'boolean', but the type found was 'integer'.";
			case JSONType::String:  return "JSON type mismatch: Expected 'boolean', but the type found was 'string'.";
			case JSONType::Array:   return "JSON type mismatch: Expected 'boolean', but the type found was 'array'.";
			case JSONType::Object:  return "JSON type mismatch: Expected 'boolean', but the type found was 'object'.";
			case JSONType::Double:  return "JSON type mismatch: Expected 'boolean', but the type found was 'double'.";
			case JSONType::Null:    return "JSON type mismatch: Expected 'boolean', but the type found was 'null'.";
			case JSONType::Boolean: abort(); // This case should never happen.
			}
		case JSONType::Array:
			switch (m_foundType)
			{
			case JSONType::Empty:   return "JSON type mismatch: Expected 'array', but the provided JSON token was empty.";
			case JSONType::Integer: return "JSON type mismatch: Expected 'array', but the type found was 'integer'.";
			case JSONType::String:  return "JSON type mismatch: Expected 'array', but the type found was 'string'.";
			case JSONType::Boolean: return "JSON type mismatch: Expected 'array', but the type found was 'boolean'.";
			case JSONType::Object:  return "JSON type mismatch: Expected 'array', but the type found was 'object'.";
			case JSONType::Double:  return "JSON type mismatch: Expected 'array', but the type found was 'double'.";
			case JSONType::Null:    return "JSON type mismatch: Expected 'array', but the type found was 'null'.";
			case JSONType::Array:   abort(); // This case should never happen.
			}
		case JSONType::Object:
			switch (m_foundType)
			{
			case JSONType::Empty:   return "JSON type mismatch: Expected 'object', but the provided JSON token was empty.";
			case JSONType::Integer: return "JSON type mismatch: Expected 'object', but the type found was 'integer'.";
			case JSONType::String:  return "JSON type mismatch: Expected 'object', but the type found was 'string'.";
			case JSONType::Boolean: return "JSON type mismatch: Expected 'object', but the type found was 'boolean'.";
			case JSONType::Array:   return "JSON type mismatch: Expected 'object', but the type found was 'array'.";
			case JSONType::Double:  return "JSON type mismatch: Expected 'object', but the type found was 'double'.";
			case JSONType::Null:    return "JSON type mismatch: Expected 'object', but the type found was 'null'.";
			case JSONType::Object:  abort(); // This case should never happen.
			}
		case JSONType::Double:
			switch (m_foundType)
			{
			case JSONType::Empty:   return "JSON type mismatch: Expected 'double', but the provided JSON token was empty.";
			case JSONType::Integer: return "JSON type mismatch: Expected 'double', but the type found was 'integer'.";
			case JSONType::String:  return "JSON type mismatch: Expected 'double', but the type found was 'string'.";
			case JSONType::Boolean: return "JSON type mismatch: Expected 'double', but the type found was 'boolean'.";
			case JSONType::Array:   return "JSON type mismatch: Expected 'double', but the type found was 'array'.";
			case JSONType::Object:  return "JSON type mismatch: Expected 'double', but the type found was 'object'.";
			case JSONType::Null:    return "JSON type mismatch: Expected 'double', but the type found was 'null'.";
			case JSONType::Double:  abort(); // This case should never happen.
			}
		case JSONType::Null:
			switch (m_foundType)
			{
			case JSONType::Empty:   return "JSON type mismatch: Expected 'null', but the provided JSON token was empty.";
			case JSONType::Integer: return "JSON type mismatch: Expected 'null', but the type found was 'integer'.";
			case JSONType::String:  return "JSON type mismatch: Expected 'null', but the type found was 'string'.";
			case JSONType::Boolean: return "JSON type mismatch: Expected 'null', but the type found was 'boolean'.";
			case JSONType::Array:   return "JSON type mismatch: Expected 'null', but the type found was 'array'.";
			case JSONType::Object:  return "JSON type mismatch: Expected 'null', but the type found was 'object'.";
			case JSONType::Double:  return "JSON type mismatch: Expected 'null', but the type found was 'double'.";
			case JSONType::Null:    abort(); // This case should never happen.
			}
		}
		return "Something went wrong.";
	}

private:
	JSONType m_expectedType;
	JSONType m_foundType;
};

class LightningJSON::InvalidJSON : public JSONException
{
public:
	virtual ~InvalidJSON() noexcept
	{

	}
	virtual char const* what() const noexcept override
	{
		return "Could not parse JSON: Provided JSON data was invalid.";
	}
};