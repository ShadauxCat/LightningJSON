#pragma once

#include <stdexcept>

#include "Hash.hpp"
#include "Node.hpp"

namespace SkipProbe
{
	namespace detail_
	{
		template<typename t_KeyType, typename t_ValueType, typename t_Hash = Hash<t_KeyType>, typename t_Compare = std::equal_to<t_KeyType>, typename t_Allocator = std::allocator<LinkedNode<t_KeyType, t_ValueType>>>
		class HashContainerBase;
	}

	template<typename t_KeyType, typename t_ValueType, typename t_Hash = Hash<t_KeyType>, typename t_Compare = std::equal_to<t_KeyType>, typename t_Allocator = std::allocator<LinkedNode<t_KeyType, t_ValueType>>>
	class HashMap;

	template<typename t_KeyType, typename t_Hash = Hash<t_KeyType>, typename t_Compare = std::equal_to<t_KeyType>, typename t_Allocator = std::allocator<LinkedNode<t_KeyType, void>>>
	class HashSet;
}

#include "Iterator.hpp"


template<typename t_KeyType, typename t_ValueType, typename t_Hash, typename t_Compare, typename t_Allocator>
class SkipProbe::detail_::HashContainerBase
{
public:
	using Node = LinkedNode<t_KeyType, t_ValueType>;
	using Iterator = MapIterator<t_KeyType, t_ValueType>;
	using ConstIterator = MapIterator<t_KeyType, t_ValueType> const;

	using key_type = t_KeyType;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using hasher = t_Hash;
	using key_equal = t_Compare;
	using allocator_type = t_Allocator;
	using pointer = typename std::allocator_traits<t_Allocator>::pointer;
	using const_pointer = typename std::allocator_traits<t_Allocator>::const_pointer;
	using iterator = Iterator;
	using const_iterator = ConstIterator;
	using local_iterator = Iterator;
	using const_local_iterator = ConstIterator;

	HashContainerBase(t_Hash hash = t_Hash(), t_Compare compare = t_Compare(), t_Allocator allocator = t_Allocator())
		: m_allocator(allocator)
		, m_count(0)
		, m_bucketCount(8)
		, m_list(m_allocator.allocate(m_bucketCount))
		, m_hash(hash)
		, m_compare(compare)
	{
		memset(m_list, 0, m_bucketCount * sizeof(Node));
	}

	explicit HashContainerBase(t_Allocator allocator)
		: m_allocator(allocator)
		, m_count(0)
		, m_bucketCount(8)
		, m_list(m_allocator.allocate(m_bucketCount))
	{
		memset(m_list, 0, m_bucketCount * sizeof(Node));
	}

	HashContainerBase(std::initializer_list<std::pair<t_KeyType, t_ValueType>> list, t_Hash hash = t_Hash(), t_Compare compare = t_Compare(), t_Allocator allocator = t_Allocator())
		: m_allocator(allocator)
		, m_count(0)
		, m_bucketCount(nearestPowerOf2_(list.size()))
		, m_list(m_allocator.allocate(m_bucketCount))
		, m_hash(hash)
		, m_compare(compare)
	{
		memset(m_list, 0, m_bucketCount * sizeof(Node));
		for (auto& pair : list)
		{
			Insert(pair.first, pair.first);
		}
	}

	HashContainerBase(HashContainerBase const& other)
		: m_allocator(other.m_allocator)
		, m_count(0)
		, m_bucketCount(other.m_bucketCount)
		, m_list(m_allocator.allocate(m_bucketCount))
		, m_hash(other.m_hash)
		, m_compare(other.m_compare)
	{
		memset(m_list, 0, m_bucketCount * sizeof(Node));
		for (auto& node : other)
		{
			Insert(node.key, node.value);
		}
	}

	HashContainerBase(HashContainerBase&& other)
		: m_allocator(std::move(other.m_allocator))
		, m_count(other.m_count)
		, m_bucketCount(other.m_bucketCount)
		, m_list(other.m_list)
		, m_collisions(other.m_collisions)
		, m_hash(std::move(other.m_hash))
		, m_compare(std::move(other.m_compare))
	{
		other.m_list = other.m_allocator.allocate(8);
		other.m_count = 0;
		other.m_bucketCount = 8;
		other.m_collisions = 0;
		memset(other.m_list, 0, m_bucketCount * sizeof(Node));
	}

	~HashContainerBase() noexcept
	{
		destroy_();
	}

protected:
	struct InsertNodeResult
	{
		Node* node;
		bool wasInserted;
	};

#if defined(_WIN64) || defined(_M_X64) || defined(__x86_64__)
	static size_t nearestPowerOf2_(size_t v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v |= v >> 32;
		v++;
		return v;
	}
#else
	static size_t nearestPowerOf2_(size_t v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}
#endif

	void resize_(size_t newBucketSize)
	{
		Node* oldList = m_list;
		size_t oldSize = m_bucketCount;

		m_list = m_allocator.allocate(newBucketSize);
		m_count = 0;
		m_bucketCount = newBucketSize;
		m_collisions = 0;

		memset(m_list, 0, m_bucketCount * sizeof(Node));

		for (Node* node = oldList; node < oldList + oldSize; ++node)
		{
			if (node->firstInBucket != nullptr)
			{
				if constexpr (std::is_same<t_ValueType, void>::value)
				{
					doInsert_(std::move(node->key), node->hash);
				}
				else
				{
					doInsert_(std::move(node->key), std::move(node->value), node->hash);
				}
				node->Dispose();
			}
		}
		m_allocator.deallocate(oldList, oldSize);
	}

	void destroy_() noexcept
	{
		for (Node* node = m_list; node < m_list + m_bucketCount; ++node)
		{
			if (node->firstInBucket != nullptr)
			{
				node->Dispose();
			}
		}
		m_allocator.deallocate(m_list, m_bucketCount);
	}

	template<typename t_KeyReferenceType>
	InsertNodeResult doInsert_(t_KeyReferenceType&& key, size_t hash) noexcept
	{
		size_t location = hash & (m_bucketCount - 1);
		return findNodeForInsert_(std::forward<t_KeyReferenceType>(key), hash, location);
	}

	template<typename t_KeyReferenceType, typename t_ValueReferenceType>
	InsertNodeResult doInsert_(t_KeyReferenceType&& key, t_ValueReferenceType&& value, size_t hash) noexcept
	{
		size_t location = hash & (m_bucketCount - 1);
		return findNodeForInsert_(std::forward<t_KeyReferenceType>(key), std::forward<t_ValueReferenceType>(value), hash, location);
	}

	template<typename t_KeyReferenceType>
	InsertNodeResult doUpsert_(t_KeyReferenceType&& key, size_t hash) noexcept
	{
		size_t location = hash & (m_bucketCount - 1);
		auto result = findNodeForInsert_(std::forward<t_KeyReferenceType>(key), hash, location);
		return result;
	}

	template<typename t_KeyReferenceType, typename t_ValueReferenceType>
	InsertNodeResult doUpsert_(t_KeyReferenceType&& key, t_ValueReferenceType&& value, size_t hash) noexcept
	{
		size_t location = hash & (m_bucketCount - 1);
		auto result = findNodeForInsert_(std::forward<t_KeyReferenceType>(key), std::forward<t_ValueReferenceType>(value), hash, location);
		if (!result.wasInserted)
		{
			result.node->value = std::forward<t_ValueReferenceType>(value);
		}
		return result;
	}

	Node* findNode_(t_KeyType const& key, size_t bucketSearchLocation) const noexcept
	{
		Node* node = &m_list[bucketSearchLocation];
		if (node->firstInBucket == node)
		{
			do
			{
				if (m_compare(key, node->key))
				{
					return node;
				}
				node = node->nextInBucket;
			} while (node != nullptr);
		}
		return m_list + m_bucketCount;
	}

	bool removeNode_(t_KeyType const& key, size_t bucketRemoveLocation) noexcept
	{
		Node* firstNode = &m_list[bucketRemoveLocation];

		Node* removeNode = nullptr;
		for (Node* checkNode = firstNode; checkNode != nullptr; checkNode = checkNode->nextInBucket)
		{
			if (m_compare(checkNode->key, key))
			{
				removeNode = checkNode;
				break;
			}
		}
		if (removeNode == nullptr)
		{
			return false;
		}
		if (removeNode == firstNode)
		{
			// If the removed node is actually the first node for this bucket,
			// then the next node for this bucket needs to become the first node.
			// Which means it needs to be moved into this node's position in the array
			// so we can do lookups on it.
			Node* nextNode = removeNode->nextInBucket;
			if (nextNode != nullptr)
			{
				removeNode->nextInBucket = nextNode->nextInBucket;
				removeNode->key = std::move(nextNode->key);
				removeNode->value = std::move(nextNode->value);
				removeNode->hash = nextNode->hash;
				nextNode->hash = 0;
				nextNode->Dispose();
				nextNode->firstInBucket = nullptr;
				nextNode->nextInBucket = nullptr;
				nextNode->prevInBucket = nullptr;
				nextNode->lastInBucket = nullptr;
				if (nextNode == removeNode->lastInBucket)
				{
					removeNode->lastInBucket = removeNode;
				}
			}
			else
			{
				removeNode->Dispose();
				removeNode->firstInBucket = nullptr;
				removeNode->nextInBucket = nullptr;
				removeNode->prevInBucket = nullptr;
				removeNode->lastInBucket = nullptr;
			}
			--m_count;
			return true;
		}
		// Otherwise we just remove it from the list.
		removeNode_(removeNode);
		--m_count;
		return true;
	}

	void removeNode_(Node* removeNode) noexcept
	{
		Node* firstNode = removeNode->firstInBucket;

		if (removeNode == firstNode->lastInBucket)
		{
			firstNode->lastInBucket = removeNode->prevInBucket;
		}

		if (removeNode->prevInBucket)
		{
			removeNode->prevInBucket->nextInBucket = removeNode->nextInBucket;
		}
		if (removeNode->nextInBucket)
		{
			removeNode->nextInBucket->prevInBucket = removeNode->prevInBucket;
		}
		removeNode->firstInBucket = nullptr;
		removeNode->nextInBucket = nullptr;
		removeNode->prevInBucket = nullptr;
		removeNode->lastInBucket = nullptr;
		removeNode->Dispose();
	}

	Node* findPositionInExistingBucket_(Node* node) noexcept
	{
		// The bucket for this node matches the index.
		// Look for the next place we can insert and add this to the list.
		Node* firstNode = node;
		Node* lastNode = node->lastInBucket;
		while (node->firstInBucket != nullptr)
		{
			node = node->firstInBucket->lastInBucket + 1;
			if (node >= m_list + m_bucketCount)
			{
				node = m_list;
			}
		}
		// Update the previous last node to point at this one
		lastNode->nextInBucket = node;
		// Not trying to track last on every node.
		// Instead, to get to lastInBucket, we do node->firstInBucket->lastInBucket
		lastNode->lastInBucket = nullptr;
		// The new node is now the last in bucket.
		firstNode->lastInBucket = node;
		// Set the firstInBucket value to the first node
		node->firstInBucket = firstNode;
		// There is no nextInBucket for this hash value
		node->nextInBucket = nullptr;
		// Only the first in bucket tracks its last pointer.
		node->lastInBucket = nullptr;
		// Set up the backward tracking
		node->prevInBucket = lastNode;
		return node;
	}

	template<typename t_KeyReferenceType, typename t_ValueReferenceType>
	InsertNodeResult findNodeForInsert_(t_KeyReferenceType&& key, t_ValueReferenceType&& value, size_t hash, size_t bucketInsertLocation) noexcept
	{
		Node* node = &m_list[bucketInsertLocation];

		if (node->firstInBucket == nullptr)
		{
			node->firstInBucket = node;
			node->lastInBucket = node;
			new(&node->key) t_KeyType(std::forward<t_KeyReferenceType>(key));
			new(&node->value) t_ValueType(std::forward<t_ValueReferenceType>(value));
			node->hash = hash;
			++m_count;
			return { node, true };
		}
		++m_collisions;

		// A node already exists in this bucket.
		if (node->firstInBucket == node)
		{
			for (Node* checkNode = node; checkNode != nullptr; checkNode = checkNode->nextInBucket)
			{
				if (m_compare(checkNode->key, key))
				{
					return { checkNode, false };
				}
			}
			node = findPositionInExistingBucket_(node);
			new(&node->key) t_KeyType(std::forward<t_KeyReferenceType>(key));
			new(&node->value) t_ValueType(std::forward<t_ValueReferenceType>(value));
			node->hash = hash;
			++m_count;
			return { node , true };
		}
		// Else, a node from another bucket is sitting in this one.
		// Move it to another spot and use this one.
		Node* firstNode = node->firstInBucket;
		Node* newLocation = findPositionInExistingBucket_(firstNode);
		new(&newLocation->key) t_KeyType(std::move(node->key));
		new(&newLocation->value) t_ValueType(std::move(node->value));
		newLocation->hash = node->hash;
		node->hash = 0;

		// Remove the existing one.
		removeNode_(node);

		node->firstInBucket = node;
		node->lastInBucket = node;
		new(&node->key) t_KeyType(std::forward<t_KeyReferenceType>(key));
		new(&node->value) t_ValueType(std::forward<t_ValueReferenceType>(value));
		node->hash = hash;
		++m_count;
		return { node, true };
	}

	template<typename t_KeyReferenceType>
	InsertNodeResult findNodeForInsert_(t_KeyReferenceType&& key, size_t hash, size_t bucketInsertLocation) noexcept
	{
		Node* node = &m_list[bucketInsertLocation];

		if (node->firstInBucket == nullptr)
		{
			node->firstInBucket = node;
			node->lastInBucket = node;
			new(&node->key) t_KeyType(std::forward<t_KeyReferenceType>(key));
			node->hash = hash;
			++m_count;
			return { node, true };
		}
		++m_collisions;

		// A node already exists in this bucket.
		if (node->firstInBucket == node)
		{
			for (Node* checkNode = node; checkNode != nullptr; checkNode = checkNode->nextInBucket)
			{
				if (m_compare(checkNode->key, key))
				{
					return { checkNode, false };
				}
			}
			node = findPositionInExistingBucket_(node);
			new(&node->key) t_KeyType(std::forward<t_KeyReferenceType>(key));
			node->hash = hash;
			++m_count;
			return { node , true };
		}
		// Else, a node from another bucket is sitting in this one.
		// Move it to another spot and use this one.
		Node* firstNode = node->firstInBucket;
		Node* newLocation = findPositionInExistingBucket_(firstNode);
		new(&newLocation->key) t_KeyType(std::move(node->key));
		newLocation->hash = node->hash;
		node->hash = 0;

		// Remove the existing one.
		removeNode_(node);

		node->firstInBucket = node;
		node->lastInBucket = node;
		new(&node->key) t_KeyType(std::forward<t_KeyReferenceType>(key));
		node->hash = hash;
		++m_count;
		return { node, true };
	}

	t_Allocator m_allocator;
	size_t m_count;
	size_t m_bucketCount;
	Node* m_list;
	size_t m_collisions{ 0 };
	t_Hash m_hash;
	t_Compare m_compare;
};

template<typename t_KeyType, typename t_ValueType, typename t_Hash, typename t_Compare, typename t_Allocator>
class SkipProbe::HashMap : public SkipProbe::detail_::HashContainerBase<t_KeyType, t_ValueType, t_Hash, t_Compare, t_Allocator>
{
public:
	using base = SkipProbe::detail_::HashContainerBase<t_KeyType, t_ValueType, t_Hash, t_Compare, t_Allocator>;

	using typename base::Node;
	using typename base::Iterator;
	using typename base::ConstIterator;

	using typename base::key_type;
	using typename base::size_type;
	using typename base::difference_type;
	using typename base::hasher;
	using typename base::key_equal;
	using typename base::allocator_type;
	using typename base::pointer;
	using typename base::const_pointer;
	using typename base::iterator;
	using typename base::const_iterator;
	using typename base::local_iterator;
	using typename base::const_local_iterator;

protected:
	using base::m_allocator;
	using base::m_count;
	using base::m_bucketCount;
	using base::m_list;
	using base::m_collisions;
	using base::m_hash;
	using base::m_compare;
	using base::destroy_;
	using base::resize_;
	using base::nearestPowerOf2_;
	using base::removeNode_;
	using base::doInsert_;
	using base::doUpsert_;
	using base::findNode_;
	using base::findNodeForInsert_;
	using base::findPositionInExistingBucket_;
public:


	using mapped_type = t_ValueType;
	using value_type = ::SkipProbe::Node<t_KeyType, t_ValueType>;
	using reference = value_type&;
	using const_reference = value_type const&;

	struct InsertResult
	{
		Iterator iterator;
		bool wasInserted;
	};

	HashMap& operator=(HashMap const& other)
	{
		Clear();
		Reserve(other.Size());
		for (auto& node : other)
		{
			Insert(node.key, node.value);
		}
		return *this;
	}

	HashMap& operator=(HashMap&& other)
	{
		destroy_();

		m_allocator = std::move(other.m_allocator);
		m_count = other.m_count;
		m_bucketCount = other.m_bucketCount;
		m_list = other.m_list;
		m_collisions = other.m_collisions;
		m_hash = std::move(other.m_hash);
		m_compare = std::move(other.m_compare);

		other.m_count = 0;
		other.m_bucketCount = 8;
		other.m_list = other.m_allocator.allocate(8);
		other.m_collisions = 0;
		memset(other.m_list, 0, other.m_bucketCount * sizeof(Node));

		return *this;
	}

	Iterator begin() noexcept
	{
		if (m_count == 0)
		{
			return end();
		}

		Node* node = m_list;
		while (node->firstInBucket == nullptr)
		{
			++node;
		}
		return Iterator(node, m_list + m_bucketCount);
	}

	ConstIterator begin() const noexcept
	{
		return cbegin();
	}

	ConstIterator cbegin() const noexcept
	{
		if (m_count == 0)
		{
			return cend();
		}

		Node* node = m_list;
		while (node->firstInBucket == nullptr)
		{
			++node;
		}
		return ConstIterator(node, m_list + m_bucketCount);
	}

	Iterator end() noexcept
	{
		return Iterator(m_list + m_bucketCount, m_list + m_bucketCount);
	}

	ConstIterator end() const noexcept
	{
		return cend();
	}

	ConstIterator cend() const noexcept
	{
		return ConstIterator(m_list + m_bucketCount, m_list + m_bucketCount);
	}

	bool Empty() const noexcept
	{
		return m_count == 0;
	}

	size_type Size() const noexcept
	{
		return m_count;
	}

	void Clear()
	{
		if(m_count > 0)
		{
			destroy_();

			m_bucketCount = 8;
			m_list = m_allocator.allocate(8);
			m_count = 0;
			m_collisions = 0;
			memset(m_list, 0, m_bucketCount * sizeof(Node));
		}
	}

	template<typename t_KeyReferenceType, typename t_ValueReferenceType>
	void Insert(t_KeyReferenceType&& key, t_ValueReferenceType&& value)
	{
		if (m_count >= m_bucketCount * 0.75)
		{
			resize_(m_bucketCount * 2);
		}
		size_t hashResult = m_hash(key);
		doInsert_(std::forward<t_KeyReferenceType>(key), std::forward<t_ValueReferenceType>(value), hashResult);
	}

	template<typename t_KeyReferenceType, typename t_ValueReferenceType>
	void Upsert(t_KeyReferenceType&& key, t_ValueReferenceType&& value)
	{
		if (m_count >= m_bucketCount * 0.75)
		{
			resize_(m_bucketCount * 2);
		}
		size_t hashResult = m_hash(key);
		doUpsert_(std::forward<t_KeyReferenceType>(key), std::forward<t_ValueReferenceType>(value), hashResult);
	}


	template<typename t_KeyReferenceType, typename t_ValueReferenceType>
	InsertResult CheckedInsert(t_KeyReferenceType&& key, t_ValueReferenceType&& value)
	{
		if (m_count >= m_bucketCount * 0.75)
		{
			resize_(m_bucketCount * 2);
		}
		size_t hashResult = m_hash(key);
		auto result = doInsert_(std::forward<t_KeyReferenceType>(key), std::forward<t_ValueReferenceType>(value), hashResult);
		return { Iterator(result.node, m_list + m_bucketCount), result.wasInserted };
	}

	template<typename t_KeyReferenceType, typename t_ValueReferenceType>
	InsertResult CheckedUpsert(t_KeyReferenceType&& key, t_ValueReferenceType&& value)
	{
		if (m_count >= m_bucketCount * 0.75)
		{
			resize_(m_bucketCount * 2);
		}
		size_t hashResult = m_hash(key);
		auto result = doUpsert_(std::forward<t_KeyReferenceType>(key), std::forward<t_ValueReferenceType>(value), hashResult);
		return { Iterator(result.node, m_list + m_bucketCount), result.wasInserted };
	}

	bool Delete(t_KeyType const& key) noexcept
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		return removeNode_(key, location);
	}

	void Swap(HashMap& other) noexcept
	{
		t_Allocator allocator = other.m_allocator;
		size_t count = other.m_count;
		size_t bucketSize = other.m_bucketCount;
		Node* list = other.m_list;
		size_t collisions = other.m_collisions;
		t_Hash hash = other.m_hash;
		t_Compare compare = other.m_compare;

		other.m_allocator = m_allocator;
		other.m_count = m_count;
		other.m_bucketCount = m_bucketCount;
		other.m_list = m_list;
		other.m_collisions = m_collisions;
		other.m_hash = m_hash;
		other.m_compare = m_compare;

		m_allocator = allocator;
		m_count = count;
		m_bucketCount = bucketSize;
		m_list = list;
		m_collisions = collisions;
		m_hash = hash;
		m_compare = compare;
	}

	t_ValueType& Get(t_KeyType const& key)
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		Node* result = findNode_(key, location);
		if (result != m_list + m_bucketCount)
		{
			return result->value;
		}
		throw std::out_of_range("Key out of range");
	}

	t_ValueType const& Get(t_KeyType const& key) const
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		Node* result = findNode_(key, location);
		if (result != m_list + m_bucketCount)
		{
			return result->value;
		}
		throw std::out_of_range("Key out of range");
	}

	t_ValueType& Get(t_KeyType const& key, t_ValueType& defaultValue) noexcept
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		Node* result = findNode_(key, location);
		if (result != m_list + m_bucketCount)
		{
			return result->value;
		}
		return defaultValue;
	}

	t_ValueType const& Get(t_KeyType const& key, t_ValueType const& defaultValue) const noexcept
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		Node* result = findNode_(key, location);
		if (result != m_list + m_bucketCount)
		{
			return result->value;
		}
		return defaultValue;
	}

	t_ValueType& operator[](t_KeyType const& key)
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		Node* result = findNode_(key, location);
		if (result != m_list + m_bucketCount)
		{
			return result->value;
		}
		t_ValueType val;
		InsertResult ir = CheckedInsert(key, val);
		return ir.iterator->value;
	}

	Iterator find(t_KeyType const& key) noexcept
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		Node* node = findNode_(key, location);
		return Iterator(node, m_list + m_bucketCount);
	}

	ConstIterator find(t_KeyType const& key) const noexcept
	{
		return cfind(key);
	}

	ConstIterator cfind(t_KeyType const& key) const noexcept
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		Node* node = findNode_(key, location);
		return ConstIterator(node, m_list + m_bucketCount);
	}

	bool Contains(t_KeyType const& key) const noexcept
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		Node* result = findNode_(key, location);
		return (result != m_list + m_bucketCount);
	}

	size_t BucketCount() const noexcept
	{
		return m_bucketCount;
	}

	size_t BucketSize(size_t n) const noexcept
	{
		if (n >= m_bucketCount)
		{
			return 0;
		}
		Node* node = &m_list[n];
		if (node->firstInBucket != node)
		{
			return 0;
		}
		size_t result = 0;
		while (node != nullptr)
		{
			++result;
			node = node->nextInBucket;
		}
		return result;
	}

	size_t Bucket(t_KeyType const& key) const noexcept
	{
		return m_hash(key) & (m_bucketCount - 1);
	}

	double LoadFactor() const noexcept
	{
		return double(m_count) / double(m_bucketCount);
	}

	double MaxLoadFactor() const noexcept
	{
		return 0.75;
	}

	size_t Reserve(size_t numItems)
	{
		size_t minimumSize = size_t(numItems * 1.3333333333333333);
		size_t newSize = nearestPowerOf2_(minimumSize);
		if (newSize > m_bucketCount)
		{
			resize_(newSize);
		}
		return m_bucketCount;
	}

	size_t TrimToFit(size_t numItems = 0)
	{
		size_t minimumSize = size_t(std::max(m_count, numItems) * 1.3333333333333333);
		size_t newSize = nearestPowerOf2_(minimumSize);
		if (newSize != m_bucketCount)
		{
			resize_(newSize);
		}
		return m_bucketCount;
	}

	bool operator==(HashMap const& other) const
	{
		if (other.Size() != Size())
		{
			return false;
		}
		for (auto& node : other)
		{
			size_t location = m_hash(node.key) & (m_bucketCount - 1);
			Node* thisNode = findNode_(node.key, location);
			if (thisNode == nullptr || thisNode->value != node.value)
			{
				return false;
			}
		}
		return true;
	}

	bool operator!=(HashMap const& other) const
	{
		return !this->operator==(other);
	}
};


template<typename t_KeyType, typename t_Hash, typename t_Compare, typename t_Allocator>
class SkipProbe::HashSet : public SkipProbe::detail_::HashContainerBase<t_KeyType, void, t_Hash, t_Compare, t_Allocator>
{
public:
	using base = SkipProbe::detail_::HashContainerBase<t_KeyType, void, t_Hash, t_Compare, t_Allocator>;

	using typename base::Node;
	using typename base::Iterator;
	using typename base::ConstIterator;

	using typename base::key_type;
	using typename base::size_type;
	using typename base::difference_type;
	using typename base::hasher;
	using typename base::key_equal;
	using typename base::allocator_type;
	using typename base::pointer;
	using typename base::const_pointer;
	using typename base::iterator;
	using typename base::const_iterator;
	using typename base::local_iterator;
	using typename base::const_local_iterator;

protected:
	using base::m_allocator;
	using base::m_count;
	using base::m_bucketCount;
	using base::m_list;
	using base::m_collisions;
	using base::m_hash;
	using base::m_compare;
	using base::destroy_;
	using base::resize_;
	using base::nearestPowerOf2_;
	using base::removeNode_;
	using base::doInsert_;
	using base::doUpsert_;
	using base::findNode_;
	using base::findNodeForInsert_;
	using base::findPositionInExistingBucket_;
public:

	struct InsertResult
	{
		Iterator iterator;
		bool wasInserted;
	};

	HashSet& operator=(HashSet const& other)
	{
		Clear();
		Reserve(other.Size());
		for (auto& node : other)
		{
			Insert(node.key);
		}
		return *this;
	}

	HashSet& operator=(HashSet&& other)
	{
		destroy_();

		m_allocator = std::move(other.m_allocator);
		m_count = other.m_count;
		m_bucketCount = other.m_bucketCount;
		m_list = other.m_list;
		m_collisions = other.m_collisions;
		m_hash = std::move(other.m_hash);
		m_compare = std::move(other.m_compare);

		other.m_count = 0;
		other.m_bucketCount = 8;
		other.m_list = other.m_allocator.allocate(8);
		other.m_collisions = 0;
		memset(other.m_list, 0, other.m_bucketCount * sizeof(Node));

		return *this;
	}

	Iterator begin() noexcept
	{
		if (m_count == 0)
		{
			return end();
		}

		Node* node = m_list;
		while (node->firstInBucket == nullptr)
		{
			++node;
		}
		return Iterator(node, m_list + m_bucketCount);
	}

	ConstIterator begin() const noexcept
	{
		return cbegin();
	}

	ConstIterator cbegin() const noexcept
	{
		if (m_count == 0)
		{
			return cend();
		}

		Node* node = m_list;
		while (node->firstInBucket == nullptr)
		{
			++node;
		}
		return ConstIterator(node, m_list + m_bucketCount);
	}

	Iterator end() noexcept
	{
		return Iterator(m_list + m_bucketCount, m_list + m_bucketCount);
	}

	ConstIterator end() const noexcept
	{
		return cend();
	}

	ConstIterator cend() const noexcept
	{
		return ConstIterator(m_list + m_bucketCount, m_list + m_bucketCount);
	}

	bool Empty() const noexcept
	{
		return m_count == 0;
	}

	size_type Size() const noexcept
	{
		return m_count;
	}

	void Clear()
	{
		if (m_count > 0)
		{
			destroy_();

			m_bucketCount = 8;
			m_list = m_allocator.allocate(8);
			m_count = 0;
			m_collisions = 0;
			memset(m_list, 0, m_bucketCount * sizeof(Node));
		}
	}

	template<typename t_KeyReferenceType>
	void Insert(t_KeyReferenceType&& key)
	{
		if (m_count >= m_bucketCount * 0.75)
		{
			resize_(m_bucketCount * 2);
		}
		size_t hashResult = m_hash(key);
		doInsert_(std::forward<t_KeyReferenceType>(key), hashResult);
	}

	template<typename t_KeyReferenceType>
	void Upsert(t_KeyReferenceType&& key)
	{
		if (m_count >= m_bucketCount * 0.75)
		{
			resize_(m_bucketCount * 2);
		}
		size_t hashResult = m_hash(key);
		doUpsert_(std::forward<t_KeyReferenceType>(key), hashResult);
	}


	template<typename t_KeyReferenceType>
	InsertResult CheckedInsert(t_KeyReferenceType&& key)
	{
		if (m_count >= m_bucketCount * 0.75)
		{
			resize_(m_bucketCount * 2);
		}
		size_t hashResult = m_hash(key);
		auto result = doInsert_(std::forward<t_KeyReferenceType>(key), hashResult);
		return { Iterator(result.node, m_list + m_bucketCount), result.wasInserted };
	}

	template<typename t_KeyReferenceType>
	InsertResult CheckedUpsert(t_KeyReferenceType&& key)
	{
		if (m_count >= m_bucketCount * 0.75)
		{
			resize_(m_bucketCount * 2);
		}
		size_t hashResult = m_hash(key);
		auto result = doUpsert_(std::forward<t_KeyReferenceType>(key), hashResult);
		return { Iterator(result.node, m_list + m_bucketCount), result.wasInserted };
	}

	bool Delete(t_KeyType const& key) noexcept
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		return removeNode_(key, location);
	}

	void Swap(HashSet& other) noexcept
	{
		t_Allocator allocator = other.m_allocator;
		size_t count = other.m_count;
		size_t bucketSize = other.m_bucketCount;
		Node* list = other.m_list;
		size_t collisions = other.m_collisions;
		t_Hash hash = other.m_hash;
		t_Compare compare = other.m_compare;

		other.m_allocator = m_allocator;
		other.m_count = m_count;
		other.m_bucketCount = m_bucketCount;
		other.m_list = m_list;
		other.m_collisions = m_collisions;
		other.m_hash = m_hash;
		other.m_compare = m_compare;

		m_allocator = allocator;
		m_count = count;
		m_bucketCount = bucketSize;
		m_list = list;
		m_collisions = collisions;
		m_hash = hash;
		m_compare = compare;
	}

	Iterator find(t_KeyType const& key) noexcept
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		Node* node = findNode_(key, location);
		return Iterator(node, m_list + m_bucketCount);
	}

	ConstIterator find(t_KeyType const& key) const noexcept
	{
		return cfind(key);
	}

	ConstIterator cfind(t_KeyType const& key) const noexcept
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		Node* node = findNode_(key, location);
		return ConstIterator(node, m_list + m_bucketCount);
	}

	bool Contains(t_KeyType const& key) const noexcept
	{
		size_t location = m_hash(key) & (m_bucketCount - 1);
		Node* result = findNode_(key, location);
		return (result != m_list + m_bucketCount);
	}

	size_t BucketCount() const noexcept
	{
		return m_bucketCount;
	}

	size_t BucketSize(size_t n) const noexcept
	{
		if (n >= m_bucketCount)
		{
			return 0;
		}
		Node* node = &m_list[n];
		if (node->firstInBucket != node)
		{
			return 0;
		}
		size_t result = 0;
		while (node != nullptr)
		{
			++result;
			node = node->nextInBucket;
		}
		return result;
	}

	size_t Bucket(t_KeyType const& key) const noexcept
	{
		return m_hash(key) & (m_bucketCount - 1);
	}

	double LoadFactor() const noexcept
	{
		return double(m_count) / double(m_bucketCount);
	}

	double MaxLoadFactor() const noexcept
	{
		return 0.75;
	}

	size_t Reserve(size_t numItems)
	{
		size_t minimumSize = size_t(numItems * 1.3333333333333333);
		size_t newSize = nearestPowerOf2_(minimumSize);
		if (newSize > m_bucketCount)
		{
			resize_(newSize);
		}
		return m_bucketCount;
	}

	size_t TrimToFit(size_t numItems = 0)
	{
		size_t minimumSize = size_t(std::max(m_count, numItems) * 1.3333333333333333);
		size_t newSize = nearestPowerOf2_(minimumSize);
		if (newSize != m_bucketCount)
		{
			resize_(newSize);
		}
		return m_bucketCount;
	}

	bool operator==(HashSet const& other) const
	{
		if (other.Size() != Size())
		{
			return false;
		}
		for (auto& node : other)
		{
			size_t location = m_hash(node.key) & (m_bucketCount - 1);
			Node* thisNode = findNode_(node.key, location);
			if (thisNode == nullptr)
			{
				return false;
			}
		}
		return true;
	}

	bool operator!=(HashSet const& other) const
	{
		return !this->operator==(other);
	}
};