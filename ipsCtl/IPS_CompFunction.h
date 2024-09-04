#pragma once

#ifndef __IPS_COMPONENT_FUNCTION_H__
#define __IPS_COMPONENT_FUNCTION_H__

#include <deque>
#include <vector>
#include "IPS_MethodStructureDef.h"
#include "GigECamDataStructureDef.h"

#ifdef __cplusplus
extern "C" {
#endif



	extern bool was_empty;

    // Single camera
	extern int JsonQ_Init();
	extern int JsonQ_EnQ(seJsonInfo seInfo);
	extern int JsonQ_DeQ(seJsonInfo* pInfo);
	extern void JsonQ_Destory();


    // Dual camera
	extern int JsonQ_Init_Dual(const int iID);
	extern int JsonQ_EnQ_Dual(seJsonInfo seInfo, const int iID);
	extern int JsonQ_DeQ_Dual(seJsonInfo* pInfo, const int iID);
	extern void JsonQ_Destory_Dual(const int iID);

	
	extern const int AlgoParam_TblCnt;
	extern seAlgoParamReg gAlgoParamReg[];

	extern int createHashMap_Param();
	extern int setAlgo_ParamAssign(const char* szKey, struct json_object* j_subsystem, seJsonInfo* pMethInfo);
	extern int setAlgo_ParamAssign_Dual(const char* szKey, struct json_object* j_subsystem, seJsonInfo* pMethInfo, const int iID);    



#ifdef __cplusplus
}
#endif


#endif //__IPS_COMPONENT_FUNCTION_H__

