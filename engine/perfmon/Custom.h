#ifndef _C_PERFOM_CUSTOM
#define _C_PERFOM_CUSTOM

#include "../video/VKBackend.h"
#include <map>
#include <string>

#define PERF_INIT(_NAME) _initPerf(_NAME);
#define PERF_END(_NAME) _endPerf(_NAME);
inline double start = 0.f;
inline double end = 0.f;
inline VKR::render::VKBackend* permonBackend;
inline std::map<std::string, double>PerfList;
inline FILE* PerfmonLog;
inline FILE* DebugLog;
inline void _initlializePerfmon(VKR::render::VKBackend* _backend)
{
	permonBackend = _backend;
	PerfmonLog = fopen("performance.log", "w+");
	DebugLog = freopen("Debuglog.log", "w+", stdout);
}
inline void _initPerf(const char* _name)
{
	start = permonBackend->GetTime();
	PerfList[std::string(_name)] = start;
}

inline void _endPerf(const char* _name)
{
	end = permonBackend->GetTime();
#ifndef _RELEASE
	auto elap = (end - PerfList[std::string(_name)]);
	printf("PM_%s : %.3f\n", _name, elap);
	if (PerfmonLog)
	{
		fprintf(PerfmonLog, "%s;%.3f\n", _name, elap);
		fflush(PerfmonLog);
	}
#endif
}
inline void _finalizePerfmon()
{
	if (PerfmonLog)
		fclose(PerfmonLog);
}
#endif
