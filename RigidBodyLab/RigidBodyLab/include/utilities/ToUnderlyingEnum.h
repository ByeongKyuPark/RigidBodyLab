#include <type_traits> // for std::underlying_type_t

#define TO_INT(e) static_cast<std::underlying_type_t<decltype(e)>>(e)

/* (usage example)
	mainCamMVMat[TO_INT(ObjID::SPHERE)]
*/