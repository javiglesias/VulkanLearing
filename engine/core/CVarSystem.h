#ifndef CVARSYSTEM_H_
#define CVARSYSTEM_H
#include <cstdlib>
#include <cstring>

struct cVar
{
	const char* name;
	const char* sValue;
	float fValue;
	bool bValue;
	int iValue;
};

static cVar* global_sVars[256];
static int current_sVars = 0;
static cVar* global_fVars[256];
static int current_fVars = 0;
static cVar* global_iVars[256];
static int current_iVars = 0;
static cVar* global_bVars[256];
static int current_bVars = 0;
static void create_str_Var(const char* _name, const char* _value)
{
	global_sVars[current_sVars] = (cVar*)malloc(sizeof(cVar));
	global_sVars[current_sVars]->name = _name;
	strcpy(global_sVars[current_sVars]->sValue, _value);
	current_sVars++;
}
static void create_float_Var(const char* _name, float _value)
{
	global_fVars[current_fVars] = (cVar*)malloc(sizeof(cVar));
	global_fVars[current_fVars]->name = _name;
	global_fVars[current_fVars]->fValue = _value;
	current_fVars++;
}
static void create_int_Var(const char* _name, int _value)
{
	global_iVars[current_iVars] = (cVar*)malloc(sizeof(cVar));
	global_iVars[current_iVars]->name = _name;
	global_iVars[current_iVars]->iValue = _value;
	current_iVars++;
}
static void create_bool_Var(const char* _name, bool _value)
{
	global_bVars[current_bVars] = (cVar*)malloc(sizeof(cVar));
	global_bVars[current_bVars]->name = _name;
	global_bVars[current_bVars]->bValue = _value;
	current_bVars++;
}

#endif

