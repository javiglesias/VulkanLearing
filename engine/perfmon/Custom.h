#ifndef _C_PERFOM_CUSTOM
#define _C_PERFOM_CUSTOM
#include "../video/VKBackend.h"

#define PERF_INIT(_NAME) _initPerf(_NAME);
#define PERF_END(_NAME) _endPerf(_NAME);
inline float start = 0.f;
inline float end = 0.f;
inline VKR::render::VKBackend* permonBackend;

inline void _initlializePerfmon(VKR::render::VKBackend* _backend)
{
	permonBackend = _backend;
}
inline void _initPerf()
{
	start = permonBackend->GetTime();
}

inline void _endPerf(const char* _name)
{
	end = permonBackend->GetTime();
	printf("PM_%s : %.3f\n", _name, (end-start));
}
#endif