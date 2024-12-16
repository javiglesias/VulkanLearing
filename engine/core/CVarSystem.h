#ifndef __CVARSYSTEM_H__
#define __CVARSYSTEM_H__

class cVar
{
private:
	const char* name;
	const char* sValue;
	float fValue;
	bool bValue;
	int iValue;

public:
	cVar() = delete; // Not use the default ctor;
	cVar(const char* _name, const char* _value)
	{
		this->name = _name;
		this->sValue = _value;
	}
	cVar(const char* _name, float _value)
	{
		this->name = _name;
		this->fValue = _value;
	}
	cVar(const char* _name, int _value)
	{
		this->name = _name;
		this->iValue = _value;
	}
	cVar(const char* _name, bool _value)
	{
		this->name = _name;
		this->bValue = _value;
	}
};

class cVarSystem
{
public:
	
};

#endif

