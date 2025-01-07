#pragma once
#include <malloc.h>
/*
 TO-DO
	alloc de memoria con un tipo concreto
	dealloc comprobando que la memoria es del mismo tipo
*/
void* m_alloc(const char* _file, int _line, size_t _blockSize);

#define NEW(X) m_alloc(__FILE__, __LINE__, sizeof(X))

struct heap_mem_track
{
	char file_line[128];
	void* mem_addr;
};

heap_mem_track heap_track[256];
int current_track = 0;
inline
void* m_alloc(const char* _file, int _line, size_t _blockSize)
{
	sprintf(heap_track[current_track].file_line, "%s : %d", _file, _line);
	auto p = malloc(_blockSize);
	heap_track[current_track].mem_addr = p;
	memset(p, 0x69, _blockSize);
	++current_track;
	return p;
}

void m_delete(void* _ptr)
{
	for (int i = 0; i < current_track; i++)
	{
		if(heap_track[i].mem_addr == _ptr)
		{
			delete _ptr;
		}
	}
}

void m_endMem()
{
	for (int i = 0; i < current_track; i++)
	{
		if(heap_track[i].mem_addr != nullptr)
		{
			fprintf(stderr, "Memory Leak at %s -> %p", heap_track[i].file_line, heap_track[i].mem_addr);
		}
	}
}
