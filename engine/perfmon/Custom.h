#ifndef _C_PERFOM_CUSTOM
#define _C_PERFOM_CUSTOM
#include "../video/VKBackend.h"
#include <map>

#define PERF_INIT(_NAME) _initPerf(_NAME);
#define PERF_END(_NAME) _endPerf(_NAME);
inline double start = 0.f;
inline double end = 0.f;
inline VKR::render::VKBackend* permonBackend;
inline std::map<std::string, double>PerfList;

inline void _initlializePerfmon(VKR::render::VKBackend* _backend)
{
	permonBackend = _backend;
}
inline void _initPerf(const char* _name)
{
	start = permonBackend->GetTime();
	PerfList[std::string(_name)] = start;
}

inline void _endPerf(const char* _name)
{
	end = permonBackend->GetTime();
	//printf("PM_%s : %.3f\n", _name, (end - PerfList[std::string(_name)]));
}
#endif
