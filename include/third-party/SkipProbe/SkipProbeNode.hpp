#pragma once

namespace SkipProbe
{
	template<typename t_KeyType, typename t_ValueType>
	class Node;

	template<typename t_KeyType, typename t_ValueType>
	class LinkedNode;
}

template<typename t_KeyType, typename t_ValueType>
struct SkipProbe::Node
{
	t_KeyType key;
	t_ValueType value;
};

template<typename t_KeyType, typename t_ValueType>
struct SkipProbe::LinkedNode : public SkipProbe::Node<t_KeyType, t_ValueType>
{
	Node* firstInBucket;
	Node* nextInBucket;
	Node* prevInBucket;
	Node* lastInBucket;
	size_t hash;
};
