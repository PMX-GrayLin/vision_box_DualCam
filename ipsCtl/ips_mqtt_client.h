#pragma once

#ifndef __IPS_MQTT_CLIENT_H__
#define __IPS_MQTT_CLIENT_H__

/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */
#include "IPS_MethodStructureDef.h"
#include "IPS_CompFunction.h"

#ifdef __cplusplus
extern "C" {
#endif


/* EXPORTED SUBPROGRAM SPECIFICATION ------------------------------------------------- */
extern int ips_mqtt_subscriber();
extern int ips_mqtt_publisher(char* jString);

/* NAMING CONSTANT DECLARATIONS ------------------------------------------------------ */
#define IPS_MQTT_RESPONSE_SIZE  128

#ifdef __cplusplus
}
#endif


#endif //__IPS_MQTT_CLIENT_H__