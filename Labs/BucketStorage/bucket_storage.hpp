#pragma once

#include <cstddef>
#include <iostream>
#include <iterator>
#include <stdexcept>

class Block;

template< typename T >
class BucketStorage
{
  public:
	// Types
	using value_type = T;
	using pointer = value_type *;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = T &;
	using const_reference = const T &;
	class iterator;
	class const_iterator;

	// Constructors
	BucketStorage(const BucketStorage &other);
	BucketStorage(BucketStorage &&other) noexcept;
	explicit BucketStorage(size_type initial_capacity = 64);

	// Destructor
	~BucketStorage();

	// Assignment operators
	BucketStorage &operator=(const BucketStorage &other);
	BucketStorage &operator=(BucketStorage &&other) noexcept;

	// Element access
	iterator begin() noexcept;
	const_iterator begin() const noexcept;
	const_iterator cbegin() noexcept;
	iterator end() noexcept;
	const_iterator end() const noexcept;
	const_iterator cend() noexcept;

	// Capacity
	bool empty() const noexcept;
	size_type size() const noexcept;
	size_type capacity() const noexcept;

	// Modifiers
	void clear();
	void swap(BucketStorage &other) noexcept;
	void shrink_to_fit();

	// Element insert and erase
	iterator insert(const value_type &value);
	iterator insert(value_type &&value);
	iterator erase(iterator it);

	// Additional operations
	iterator get_to_distance(iterator it, difference_type distance);

  private:
	class Block
	{
	  public:
		pointer *data;
		size_type capacity;
		size_type size;
		Block *prev;
		Block *next;

		explicit Block(size_type initial_capacity) : capacity(initial_capacity), size(0), prev(nullptr), next(nullptr)
		{
			data = new pointer[capacity];
		}
		~Block() { delete[] data; }
	};
	size_type block_capacity;
	size_type size_storage;
	size_type block_count;
	Block *head;
	Block *tail;
	Block *defective;

	void new_block();
	void delete_block(Block *block);

	friend class iterator;
	friend class const_iterator;
};

// Nested iterator class
template< typename T >
class BucketStorage< T >::iterator
{
  public:
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using pointer = T *;
	using reference = T &;

	iterator() : storage(nullptr), block(nullptr) {}
	iterator(BucketStorage *storage, Block *blk, size_type ind) : storage(storage), block(blk), index(ind) {}

	reference operator*() const;
	pointer operator->() const;

	iterator &operator++();
	iterator operator++(int);
	iterator &operator--();
	iterator operator--(int);

	bool operator<(const iterator &other) const;
	bool operator>(const iterator &other) const;
	bool operator<=(const iterator &other) const;
	bool operator>=(const iterator &other) const;

	bool operator==(const iterator &other) const;
	bool operator!=(const iterator &other) const;

	bool operator==(const const_iterator &other) const;
	bool operator!=(const const_iterator &other) const;

  private:
	BucketStorage *storage;
	Block *block;
	// взяли с запасом, на всякий
	int64_t index;
	friend class BucketStorage;
};

template< typename T >
class BucketStorage< T >::const_iterator
{
  public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = T;
	using difference_type = std::ptrdiff_t;
	using pointer = const T *;
	using reference = const T &;

	const_iterator() : storage(nullptr), block(nullptr) {}
	const_iterator(const BucketStorage *storage, Block *blk, size_type ind) : storage(storage), block(blk), index(ind)
	{
	}
	const_iterator(const iterator &it) : storage(it.storage), block(it.block), index(it.index) {}

	reference operator*() const;
	pointer operator->() const;

	const_iterator &operator++();
	const_iterator operator++(int);
	const_iterator &operator--();
	const_iterator operator--(int);

	bool operator==(const const_iterator &other) const;
	bool operator!=(const const_iterator &other) const;

  private:
	const BucketStorage *storage;
	Block *block;
	size_type index;

	friend class BucketStorage;
};

template< typename T >
void BucketStorage< T >::new_block()
{
	Block *new_block = new Block(block_capacity);
	if (head == nullptr)
	{
		head = tail;
		tail = new_block;
	}
	else
	{
		tail->next = new_block;
		tail->next->prev = tail;

		tail = new_block;
	}
	block_count++;
	defective = new Block(block_capacity);
}

template< typename T >
void BucketStorage< T >::delete_block(Block *block)
{
	block_count--;
	for (size_type i = 0; i < block->capacity; i++)
	{
		if (block->data[i] != nullptr)
		{
			delete block->data[i];
			size_storage--;
		}
	}
	delete[] block->data;
}

// Implementations of BucketStorage::iterator methods

template< typename T >
typename BucketStorage< T >::iterator::reference BucketStorage< T >::iterator::operator*() const
{
	if (block == nullptr || block == storage->defective || index >= block->capacity || block->data[index] == nullptr)
	{
		throw std::out_of_range("Iterator out of range");
	}
	return *(block->data[index]);
}

template< typename T >
typename BucketStorage< T >::iterator::pointer BucketStorage< T >::iterator::operator->() const
{
	if (block == nullptr || block == storage->defective || index >= block->capacity || block->data[index] == nullptr)
	{
		throw std::out_of_range("Iterator out of range");
	}
	return block->data[index];
}

template< typename T >
typename BucketStorage< T >::iterator &BucketStorage< T >::iterator::operator++()
{
	if (block == nullptr || block == storage->defective)
	{
		throw std::out_of_range("Iterator out of range");
	}
	index++;
	while (index >= block->capacity || block->data[index] == nullptr)
	{
		if (index >= block->capacity)
		{
			block = block->next;
			index = -1;
		}
		else if (block == nullptr || block == storage->defective)
		{
			index = 0;
			block = storage->defective;
			return *this;
		}
		index++;
	}
	return *this;
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::iterator::operator++(int)
{
	iterator temp = *this;
	++(*this);
	return temp;
}

template< typename T >
typename BucketStorage< T >::iterator &BucketStorage< T >::iterator::operator--()
{
	if (block == nullptr)
	{
		throw std::out_of_range("Iterator out of range");
	}
	--index;
	while (index < 0 || block->data[index] == nullptr)
	{
		if (index < 0)
		{
			block = block->prev;
			index = block->capacity - 1;
		}
		else if (block == nullptr)
		{
			index = 0;
			block = storage->defective;
			return *this;
		}
		index--;
	}
	return *this;
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::iterator::operator--(int)
{
	iterator temp = *this;
	--(*this);
	return temp;
}

template< typename T >
bool BucketStorage< T >::iterator::operator==(const iterator &other) const
{
	if (storage != other.storage || block != other.block)
	{
		return false;
	}
	if (block == nullptr || block == storage->defective || other.block == nullptr || other.block == other.storage->defective)
	{
		throw std::out_of_range("Iterator out of range");
	}
	return index == other.index;
}

template< typename T >
bool BucketStorage< T >::iterator::operator!=(const iterator &other) const
{
	return !(*this == other);
}

template< typename T >
bool BucketStorage< T >::iterator::operator==(const const_iterator &other) const
{
	if (storage != other.storage || block != other.block)
	{
		return false;
	}
	if (block == nullptr || block == storage->defective || other.block == nullptr || other.block == other.storage->defective)
	{
		throw std::out_of_range("Iterator out of range");
	}
	return index == other.index;
}

template< typename T >
bool BucketStorage< T >::iterator::operator!=(const const_iterator &other) const
{
	return !(*this == other);
}

template< typename T >
bool BucketStorage< T >::iterator::operator<(const iterator &other) const
{
	return storage == other.storage && block == other.block && index < other.index;
}

template< typename T >
bool BucketStorage< T >::iterator::operator<=(const iterator &other) const
{
	return storage == other.storage && block == other.block && index <= other.index;
}

template< typename T >
bool BucketStorage< T >::iterator::operator>(const iterator &other) const
{
	return !(this < other);
}

template< typename T >
bool BucketStorage< T >::iterator::operator>=(const iterator &other) const
{
	return storage == other.storage && block == other.block && index >= other.index;
}

// Implementations of BucketStorage::const_iterator methods

template< typename T >
typename BucketStorage< T >::const_iterator::reference BucketStorage< T >::const_iterator::operator*() const
{
	if (block == nullptr || block == storage->defective || index >= block->capacity || block->data[index] == nullptr)
	{
		throw std::out_of_range("Iterator out of range");
	}
	return *(block->data[index]);
}

template< typename T >
typename BucketStorage< T >::const_iterator::pointer BucketStorage< T >::const_iterator::operator->() const
{
	if (block == nullptr || block == storage->defective || index >= block->capacity || block->data[index] == nullptr)
	{
		throw std::out_of_range("Iterator out of range");
	}
	return (block->data[index]);
}

template< typename T >
typename BucketStorage< T >::const_iterator &BucketStorage< T >::const_iterator::operator++()
{
	if (block == nullptr || block == storage->defective)
	{
		throw std::out_of_range("Iterator out of range");
	}
	index++;
	while (index >= block->capacity || block->data[index] == nullptr)
	{
		if (index >= block->capacity)
		{
			block = block->next;
			index = -1;
		}
		else if (block == nullptr || block == storage->defective)
		{
			index = 0;
			block = storage->defective;
			return *this;
		}
		index++;
	}
	return *this;
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::const_iterator::operator++(int)
{
	const_iterator temp = *this;
	++(*this);
	return temp;
}

template< typename T >
typename BucketStorage< T >::const_iterator &BucketStorage< T >::const_iterator::operator--()
{
	if (block == nullptr)
	{
		throw std::out_of_range("Iterator out of range");
	}
	--index;
	while (index < 0 || block->data[index] == nullptr)
	{
		if (index < 0)
		{
			block = block->prev;
			index = block->capacity - 1;
		}
		else if (block == nullptr)
		{
			index = 0;
			block = storage->defective;
			return *this;
		}
		index--;
	}
	return *this;
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::const_iterator::operator--(int)
{
	iterator temp = *this;
	--(*this);
	return temp;
}

template< typename T >
bool BucketStorage< T >::const_iterator::operator==(const const_iterator &other) const
{
	if (storage != other.storage || block != other.block)
	{
		return false;
	}
	if (block == nullptr || block == storage->defective || other.block == nullptr || other.block == other.storage->defective)
	{
		throw std::out_of_range("Iterator out of range");
	}
	return index == other.index;
}

template< typename T >
bool BucketStorage< T >::const_iterator::operator!=(const const_iterator &other) const
{
	return !(*this == other);
}

// Implementations of BucketStorage methods

template< typename T >
BucketStorage< T >::~BucketStorage()
{
	clear();
}

template< typename T >
BucketStorage< T >::BucketStorage(size_type initial_capacity) :
	block_capacity(initial_capacity), size_storage(0), block_count(0), head(nullptr), tail(nullptr), defective(nullptr)
{
}

template< typename T >
BucketStorage< T >::BucketStorage(const BucketStorage &other) :
	block_capacity(other.block_capacity), size_storage(other.size_storage), block_count(other.block_count),
	head(other.head), tail(other.tail), defective(other.defective)
{
	for (const auto &value : other)
	{
		insert(value);
	}
}

template< typename T >
BucketStorage< T >::BucketStorage(BucketStorage &&other) noexcept
{
	block_capacity = other.block_capacity;
	size_storage = other.size_storage;
	block_count = other.block_count;
	head = other.head;
	tail = other.tail;
	defective = other.defective;

	// Очистка `other` для предотвращения удаления данных при его деструкции
	other.block_capacity = 0;
	other.size_storage = 0;
	other.block_count = 0;
	other.head = nullptr;
	other.tail = nullptr;
	other.defective = nullptr;
}

template< typename T >
BucketStorage< T > &BucketStorage< T >::operator=(const BucketStorage &other)
{
	if (this == &other)
	{
		return *this;
	}
	BucketStorage temp(other);
	swap(temp);
	return *this;
}

template< typename T >
BucketStorage< T > &BucketStorage< T >::operator=(BucketStorage &&other) noexcept
{
	if (this != &other)
	{
		// Переместите ресурсы из `other`
		clear();
		block_capacity = other.block_capacity;
		size_storage = other.size_storage;
		block_count = other.block_count;
		head = other.head;
		tail = other.tail;
		defective = other.defective;
		defective->prev = tail;

		other.block_capacity = 0;
		other.size_storage = 0;
		other.block_count = 0;
		other.head = nullptr;
		other.tail = nullptr;
		other.defective = nullptr;
	}
	return *this;
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::begin() noexcept
{
	Block *block = head;
	while (block)
	{
		for (size_type i = 0; i < block_capacity; i++)
		{
			if (block->data[i] != nullptr)
			{
				return iterator(this, block, i);
			}
		}
		block = block->next;
	}
	return iterator(this, defective, 0);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::begin() const noexcept
{
	Block *block = head;
	while (block)
	{
		for (size_type i = 0; i < block_capacity; i++)
		{
			if (block->data[i] != nullptr)
			{
				return const_iterator(this, block, i);
			}
		}
		block = block->next;
	}
	return const_iterator(this, defective, 0);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::cbegin() noexcept
{
	Block *block = head;
	while (block)
	{
		for (size_type i = 0; i < block_capacity; i++)
		{
			if (block->data[i] != nullptr)
			{
				return const_iterator(this, block, i);
			}
		}
		block = block->next;
	}
	return const_iterator(this, defective, 0);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::end() noexcept
{
	return iterator(this, defective, 0);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::end() const noexcept
{
	return const_iterator(this, defective, 0);
}

template< typename T >
typename BucketStorage< T >::const_iterator BucketStorage< T >::cend() noexcept
{
	return const_iterator(this, defective, 0);
}

template< typename T >
bool BucketStorage< T >::empty() const noexcept
{
	return !size_storage;
}

template< typename T >
typename BucketStorage< T >::size_type BucketStorage< T >::size() const noexcept
{
	return size_storage;
}

template< typename T >
typename BucketStorage< T >::size_type BucketStorage< T >::capacity() const noexcept
{
	return block_capacity * block_count;
}

template< typename T >
void BucketStorage< T >::clear()
{
	while (head != nullptr)
	{
		Block *next = head->next;
		delete_block(head);
		head = next;
	}
	head = nullptr;
	tail = nullptr;
	size_storage = 0;
	block_count = 0;
}

template< typename T >
void BucketStorage< T >::swap(BucketStorage &other) noexcept
{
	std::swap(block_capacity, other.block_capacity);
	std::swap(size_storage, other.size_storage);
	std::swap(block_count, other.block_count);
	std::swap(head, other.head);
	std::swap(tail, other.tail);
}

template< typename T >
void BucketStorage< T >::shrink_to_fit()
{
	// Implementation of shrink_to_fit()
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::insert(const value_type &value)
{
	if (block_count == 0 || size_storage == block_capacity * block_count)
	{
		new_block();
	}
	size_type index = tail->size++;
	tail->data[index] = new T(value);
	defective->prev = tail;
	size_storage++;
	return iterator(this, tail, index);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::insert(value_type &&value)
{
	if (block_count == 0 || size_storage == block_capacity * block_count)
	{
		new_block();
	}
	size_type index = tail->size++;
	tail->data[index] = new value_type(std::move(value));
	;
	defective->prev = tail;
	size_storage++;
	return iterator(this, tail, index);
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::erase(iterator it)
{
	Block *block = it.block;
	delete block->data[it.index];
	block->data[it.index] = nullptr;
	block->size--;
	size_storage--;
	if (block->size == 0)
	{
		delete_block(block);
	}

	if (it == end())
	{
		return end();
	}
	++it;
	return it;
}

template< typename T >
typename BucketStorage< T >::iterator BucketStorage< T >::get_to_distance(iterator it, difference_type distance)
{
	while (distance > 0)
	{
		++it;
		--distance;
	}
	while (distance < 0)
	{
		--it;
		++distance;
	}
	return it;
}
