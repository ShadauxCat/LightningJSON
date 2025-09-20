#pragma once

namespace SkipProbe
{
	template<typename t_KeyType, typename t_ValueType>
	struct Node;

	template<typename t_KeyType, typename t_ValueType>
	struct LinkedNode;
}

template<typename t_KeyType, typename t_ValueType>
struct SkipProbe::Node
{
	t_KeyType key;
	t_ValueType value;

	void Dispose()
	{
		key.~t_KeyType();
		value.~t_ValueType();
	}
};

template<typename t_KeyType>
struct SkipProbe::Node<t_KeyType, void>
{
	union
	{
		t_KeyType key;
		char value;
	};

	void Dispose()
	{
		key.~t_KeyType();
	}
};

template<typename t_KeyType, typename t_ValueType>
struct SkipProbe::LinkedNode : public SkipProbe::Node<t_KeyType, t_ValueType>
{
	LinkedNode* firstInBucket;
	LinkedNode* nextInBucket;
	LinkedNode* prevInBucket;
	LinkedNode* lastInBucket;
	size_t hash;
};
