#pragma once
#include <malloc.h>
/*
 TO-DO
	alloc de memoria con un tipo concreto
	dealloc comprobando que la memoria es del mismo tipo
*/

inline
void* m_alloc(size_t _blockSize)
{
	return malloc(_blockSize);
}