#include "mem_alloc.h"

#include <cstdio>
#include <cstring>

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
		if (heap_track[i].mem_addr == _ptr)
		{
			delete _ptr;
		}
	}
}

void m_endMem()
{
	for (int i = 0; i < current_track; i++)
	{
		if (heap_track[i].mem_addr != nullptr)
		{
			fprintf(stderr, "Memory Leak at %s -> %p", heap_track[i].file_line, heap_track[i].mem_addr);
		}
	}
}
