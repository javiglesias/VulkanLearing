#ifndef CVARSYSTEM_H_
#define CVARSYSTEM_H
#include <cstdlib>
#include <cstring>
typedef enum
{
	CVAR_ALL = -1,		// all flags
	CVAR_BOOL = BIT(0),	// variable is a boolean
	CVAR_INTEGER = BIT(1),	// variable is an integer
	CVAR_FLOAT = BIT(2),	// variable is a float
	CVAR_SYSTEM = BIT(3),	// system variable
	CVAR_RENDERER = BIT(4),	// renderer variable
	CVAR_SOUND = BIT(5),	// sound variable
	CVAR_GUI = BIT(6),	// gui variable
	CVAR_GAME = BIT(7),	// game variable
	CVAR_TOOL = BIT(8),	// tool variable
	// original doom3 used to have CVAR_USERINFO ("sent to servers, available to menu") here
	CVAR_SERVERINFO = BIT(10),	// sent from servers, available to menu
	CVAR_NETWORKSYNC = BIT(11),	// cvar is synced from the server to clients
	CVAR_STATIC = BIT(12),	// statically declared, not user created
	CVAR_CHEAT = BIT(13),	// variable is considered a cheat
	CVAR_NOCHEAT = BIT(14),	// variable is not considered a cheat
	CVAR_INIT = BIT(15),	// can only be set from the command-line
	CVAR_ROM = BIT(16),	// display only, cannot be set by user at all
	CVAR_ARCHIVE = BIT(17),	// set to cause it to be saved to a config file
	CVAR_MODIFIED = BIT(18),	// set when the variable is modified
	CVAR_NEW = BIT(19)		// added for RBDoom
} cvarFlags_t;

typedef union cVarType
{
	const char* svalue;
	float fValue;
	int iValue;
	bool bValue;
};

class cVar
{
	const char* name;
	cVarType value;
	cvarFlags_t flags;
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

