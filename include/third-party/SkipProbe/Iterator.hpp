#pragma once

#include <iterator>
#include "Node.hpp"

namespace SkipProbe
{
	template<typename t_KeyType, typename t_ValueType>
	class MapIterator;
}

template<typename t_KeyType, typename t_ValueType>
class SkipProbe::MapIterator
{
	using Node = ::SkipProbe::Node<t_KeyType, t_ValueType>;
	using LinkedNode = ::SkipProbe::LinkedNode<t_KeyType, t_ValueType>;
public:
	using iterator_category = std::forward_iterator_tag;
	using value_type        = t_ValueType;
	using difference_type   = std::ptrdiff_t;
	using pointer           = t_ValueType *;
	using reference         = t_ValueType &;

	MapIterator()
		: m_currentItem(nullptr)
		, m_end(nullptr)
	{
		//
	}

	MapIterator(LinkedNode* item, LinkedNode* end)
		: m_currentItem(item)
		, m_end(end)
	{
		//
	}

	Node& operator*()
	{
		return *m_currentItem;
	}

	Node* operator->()
	{
		return m_currentItem;
	}


	Node const& operator*() const
	{
		return *m_currentItem;
	}

	Node const* operator->() const
	{
		return m_currentItem;
	}

	MapIterator<t_KeyType, t_ValueType>& operator++()
	{
		do
		{
			++m_currentItem;
		} while (m_currentItem != m_end && m_currentItem->firstInBucket == nullptr);
		return *this;
	}

	MapIterator<t_KeyType, t_ValueType> operator++(int)
	{
		MapIterator<t_KeyType, t_ValueType> tmp(*this);
		++(*this);
		return tmp;
	}

	MapIterator<t_KeyType, t_ValueType> const& operator++() const
	{
		do
		{
			++m_currentItem;
		} while (m_currentItem != m_end && m_currentItem->firstInBucket == nullptr);
		return *this;
	}

	MapIterator<t_KeyType, t_ValueType> const operator++(int) const
	{
		MapIterator<t_KeyType, t_ValueType> tmp(*this);
		++(*this);
		return tmp;
	}

	MapIterator<t_KeyType, t_ValueType> operator+(int steps)
	{
		LinkedNode* item = m_currentItem;
		for (int i = 0; i < steps; ++i)
		{
			do
			{
				++item;
			} while (item != m_end && item->firstInBucket == nullptr);
		}
		return MapIterator<t_KeyType, t_ValueType>(item, m_end);
	}

	MapIterator<t_KeyType, t_ValueType> const operator+(int steps) const
	{
		LinkedNode* item = m_currentItem;
		for (int i = 0; i < steps; ++i)
		{
			do
			{
				++item;
			} while (item != m_end && item->firstInBucket == nullptr);
		}
		return MapIterator<t_KeyType, t_ValueType>(item, m_end);
	}

	bool operator==(MapIterator<t_KeyType, t_ValueType> const& rhs) const
	{
		return m_currentItem == rhs.m_currentItem;
	}

	bool operator!=(MapIterator<t_KeyType, t_ValueType> const& rhs) const
	{
		return !this->operator==(rhs);
	}

	operator bool() const
	{
		return m_currentItem != m_end;
	}

	bool operator!() const
	{
		return m_currentItem != m_end;
	}

	bool Valid() const
	{
		return m_currentItem != m_end;
	}

	operator bool()
	{
		return m_currentItem != m_end;
	}

protected:
	template<typename t_OtherKeyType, typename t_OtherValueType, typename t_OtherHash, typename t_OtherCompare, typename t_OtherAllocator>
	friend class ::SkipProbe::HashMap;
	mutable LinkedNode* m_currentItem;
	LinkedNode* m_end;
};