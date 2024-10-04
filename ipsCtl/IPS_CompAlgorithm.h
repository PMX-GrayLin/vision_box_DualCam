#pragma once

#ifndef __IPS_COMPONENT_ALGORITHM_H__
#define __IPS_COMPONENT_ALGORITHM_H__

/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */
#include <deque>
#include <vector>
#include "IPS_MethodStructureDef.h"
#include "GigECamDataStructureDef.h"

/* EXPORTED SUBPROGRAM SPECIFICATION ------------------------------------------------- */
#ifdef __cplusplus
extern "C" {
#endif

	extern void* pter_hdl_GigE;
	extern void* pter_hdl_GigE_Dual[2];
	extern void* pter_hdl_IPL;

	extern int ipsComp_IPL_Init();
	extern int ipsComp_IPL_Release();

	extern int ipsComp_Camera_Init();
	extern int ipsComp_Camera_Release();

	extern int ipsComp_Camera_Init_Dual(const int iID); 
	extern int ipsComp_Camera_Release_Dual(const int iID); 


    // Single camera
	int TasksQ_Init();
	int TasksQ_EnQ(CAlgoMethodParametr seMthdInfo);
	int TasksQ_DeQ(CAlgoMethodParametr* pMthdInfo);
	int TasksQ_GetSzie();
	int TasksQ_IsEmpty();
	void TasksQ_Destory();


	int ExModeQ_Init();
	int ExModeQ_EnQ(seExpansionMode iEnbExMode);
	int ExModeQ_DeQ(seExpansionMode* pEnbExMode);
	int ExModeQ_GetSzie();
	void ExModeQ_Destory();    



    // Dual camera
	int TasksQ_Init_Dual(const int iID);
	int TasksQ_EnQ_Dual(CAlgoMethodParametr seMthdInfo, const int iID);
	int TasksQ_DeQ_Dual(CAlgoMethodParametr* pMthdInfo,const int iID);
	int TasksQ_GetSzie_Dual(const int iID);
	int TasksQ_IsEmpty_Dual(const int iID);
	void TasksQ_Destory_Dual(const int iID);


	int ExModeQ_Init_Dual(const int iID);
	int ExModeQ_EnQ_Dual(seExpansionMode iEnbExMode, const int iID);
	int ExModeQ_DeQ_Dual(seExpansionMode* pEnbExMode, const int iID);
	int ExModeQ_GetSzie_Dual(const int iID);
	void ExModeQ_Destory_Dual(const int iID);
	
	extern seAlgoMethodReg gAlgoMethodReg[];
	
	extern int createHashMap_Method();
	extern int setAlgo_MethodAssign(const char* szKey, const char* szJsonArg);
    extern int setAlgo_MethodAssign_Dual(const char* szKey, const char* szJsonArg, const int iID); 
	

		


#ifdef __cplusplus
}
#endif


#endif //__IPS_COMPONENT_ALGORITHM_H__
