
#include "orbitersdk.h"
#include <EnjoLib/ModuleMessagingExtBase.hpp>


#ifndef ExportStruct_H
#define ExportStruct_H

#pragma pack(push)
#pragma pack(8)
struct ExportStruct : public EnjoLib::ModuleMessagingExtBase {
	ExportStruct():EnjoLib::ModuleMessagingExtBase(13, sizeof(ExportStruct)) {};	
	char safeName[32];
};
#pragma pack(pop)

#endif // ExportStruct_H