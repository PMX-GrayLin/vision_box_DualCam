#pragma once

#ifndef __EXT_MQTT_CLIENT_H__
#define __EXT_MQTT_CLIENT_H__

/* EXPORTED SUBPROGRAM SPECIFICATION ------------------------------------------------- */
#ifdef __cplusplus
extern "C"
{
#endif
    extern int32_t ext_mqtt_subscriber_Dual();
    extern int32_t ext_mqtt_publisher_Dual(char *jString, const bool bCameID); //dual camera

    extern void ext_mqtt_release();

#ifdef __cplusplus
}
#endif

#endif //__EXT_MQTT_CLIENT_H__
