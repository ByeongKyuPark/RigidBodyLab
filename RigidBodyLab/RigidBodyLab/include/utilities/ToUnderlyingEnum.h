#include <type_traits> // for std::underlying_type_t

//Used the fact that the underlying type of an enumeration is int by default.
#define TO_INT(e) static_cast<std::underlying_type_t<decltype(e)>>(e)

/* (usage example)
	m_mainCamMVMat[TO_INT(ObjID::SPHERE)]
*/