#pragma once

#ifndef __IOS_COMPONENT_FUNCTION_H__
#define __IOS_COMPONENT_FUNCTION_H__

#include <deque>
#include <vector>
#include "IOS_MethodStructureDef.h"

#ifdef __cplusplus
extern "C"
{
#endif

    extern std::deque<seIO_JsonInfo> BufQueue_IO_Json;
    extern int IO_JsonQ_Init();
    extern int IO_JsonQ_EnQ(seIO_JsonInfo seInfo);
    extern int IO_JsonQ_DeQ(seIO_JsonInfo *pInfo);
    extern int IO_JsonQ_GetSzie();
    extern int IO_JsonQ_IsEmpty();

    extern const int IO_AlgoParam_TblCnt;
    extern seIO_AlgoParamReg gIO_AlgoParamReg[];

    extern int createHashMap_IO_Param();
    extern int compareHashMap_IO_Param(std::string strKey);
    extern int setIO_ParamAssign(const char *szKey, struct json_object *j_subsystem, seIO_JsonInfo *pMethInfo);

#ifdef __cplusplus
}
#endif

#endif //__IOS_COMPONENT_FUNCTION_H__
