#define VK_CHECK(_value) \
	if(_value != VK_SUCCESS) \
	{VK_ASSERT(false); return false;}

#define VK_CHECK_RET(_value) \
	if(_value != VK_SUCCESS) \
	{VK_ASSERT(false); return _value;}

#define CHECK(_expression) \
	exit(-96);

inline static void VK_ASSERT(bool _check)
{
	if(_check) exit(-69);
}
