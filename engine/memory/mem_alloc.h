#pragma once
#include <malloc.h>
#include <utility>
/*
 TO-DO
	alloc de memoria con un tipo concreto
	dealloc comprobando que la memoria es del mismo tipo
*/
inline void m_endMem();
inline void* m_alloc(const char* _file, int _line, size_t _blockSize);
inline void m_delete(void* _ptr);

#define NEW(X) static_cast<X*>(m_alloc(__FILE__, __LINE__, sizeof(X)))

struct heap_mem_track
{
	char file_line[128];
	void* mem_addr;
};

inline heap_mem_track heap_track[256];
inline int current_track = 0;

template<typename T>
class s_alloc
{
public:
	using value_type = T;

	s_alloc() = default;

	template<typename U>
	constexpr s_alloc(const s_alloc<U>&){}

	T* allocate(size_t _n)
	{
		return static_cast<T*>(malloc(_n));
	}

	void deallocate(T* _p, size_t)
	{
		::operator delete(_p);
	}

	template<typename U, typename... _Args>
	void construct(U* _p, _Args&&... _args)
	{
		::new (_p) U(std::forward<_Args>(_args)...);
	}

	template<typename U>
	void destroy(U* _p)
	{
		_p->~U();
	}

	friend bool operator==(const s_alloc&, const s_alloc&) { return true; }
	friend bool operator!=(const s_alloc&, const s_alloc&) { return false; }
};


//
// Created by unframed on 09/08/2025.
//

#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include <cstdint>
#include <cstdio>

// Arena allocator
struct region
{
    region* next;
    size_t capacity;
    void* data;
};
struct arena
{
    // region management
    region* begin = nullptr;
    region* end = nullptr;
    uint32_t count = 0; // region count
    // custom mem mangaement
    void* mem_begin, * mem_current;
    size_t capacity;

};

inline arena shaders_arena;
inline arena generic_arena;

static region* alloc_region(size_t _capacity)
{
    region* new_region = static_cast<region*>(malloc(sizeof(region) + _capacity));
    new_region->capacity = _capacity;
    new_region->next = nullptr;
    new_region->data = (char*)(new_region + sizeof(region));
    return new_region;
}

static region* alloc_region_on_mem(arena* _ar, size_t _capacity)
{
    auto mem = _ar->mem_current;
    auto mem_end = ((size_t)_ar->mem_begin + _ar->capacity);
    auto mem_max = ((size_t)_ar->mem_current) + sizeof(region) + _capacity;
    if (mem_end < mem_max)
        __debugbreak();
    _ar->mem_current = (char*)_ar->mem_current + sizeof(region) + _capacity;
    region* new_region = static_cast<region*>(mem);
    new_region->capacity = _capacity;
    new_region->next = nullptr;
    new_region->data = (char*)(new_region + sizeof(region));
    return new_region;
}

// we alloc memory on the arena selected
static void init_arena(arena* _ar_, size_t _capacity)
{
    _ar_->mem_begin = _ar_->mem_current = malloc(_capacity);
    _ar_->capacity = _capacity;
}
static void* alloc_arena(arena* _ar_, size_t _capacity)
{
    if (_ar_->count == 0 || !_ar_->begin)
    {
        _ar_->end = _ar_->begin = alloc_region_on_mem(_ar_, _capacity);
    }
    else
    {
        auto new_region = alloc_region_on_mem(_ar_, _capacity);
        _ar_->end->next = new_region;
        _ar_->end = new_region;
    }
    _ar_->count++;
    return _ar_->end->data;
}

inline void*
m_alloc(const char* _file, int _line, size_t _blockSize)
{
	auto mem = alloc_arena(&generic_arena, _blockSize);
	memset(mem, 0, _blockSize);
	heap_track[current_track].mem_addr = mem;
	++current_track;
	return mem;
}

inline void
m_delete(void* _ptr)
{
	for (int i = 0; i < current_track; i++)
	{
		if (heap_track[i].mem_addr == _ptr)
		{
			delete _ptr;
		}
	}
}

inline void
m_endMem()
{
	for (int i = 0; i < current_track; i++)
	{
		if (heap_track[i].mem_addr != nullptr)
		{
			fprintf(stderr, "Memory Leak at %s -> %p\n", heap_track[i].file_line, heap_track[i].mem_addr);
		}
	}
}

#endif //ALLOCATOR_H

