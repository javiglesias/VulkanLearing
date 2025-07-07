#pragma once
#include <malloc.h>
#include <utility>
/*
 TO-DO
	alloc de memoria con un tipo concreto
	dealloc comprobando que la memoria es del mismo tipo
*/
void* m_alloc(const char* _file, int _line, size_t _blockSize);
void m_endMem();
void m_delete(void* _ptr);
#define NEW(X) m_alloc(__FILE__, __LINE__, sizeof(X))

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

