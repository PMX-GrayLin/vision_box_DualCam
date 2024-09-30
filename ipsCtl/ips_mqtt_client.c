/**
 ******************************************************************************
 * @file    ips_mqtt_client.c
 * @brief   The utility tool about mqtt client for Image Process subsystem.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 Primax Technology Ltd.
 * All rights reserved.
 *
 ******************************************************************************
 */

/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */
#include <assert.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#include <winsock2.h>
#define snprintf sprintf_s
#endif
#include <signal.h>
#include <pthread.h>
#include <mqueue.h>
#include <mosquitto.h>
#include <json.h>
#include "../client_shared.h"
#include "../common.hpp"
#include "ips_mqtt_client.h"
#include "../iosCtl/IOS_MethodStructureDef.h"

/* EXPORTED SUBPROGRAM SPECIFICATION ------------------------------------------------- */
extern ETH_PARA eth;
extern int wifi_update_hostapd(unsigned char *ssid, unsigned char *pwd);
extern void wifi_restart_ap();
extern int eth0_set_ip(bool mode, char *ip, char *gateway);
extern int get_ip_by_ifname(char *ifname, char *ip);
extern void wifi_turn_off();
extern void wifi_turn_on();
extern WIFI_AP_PARA wifi;
enum {
  SLAVE_ID, 
  FUNCTION_CODE, 
  ADDRESS,
  LENGTH,
  CMD_MAX
};
static struct mosquitto *ips_mosq = NULL;  /* internal MQTT for subsystem communication */
static struct mosq_config *ips_cfg;

bool ips_process_messages = true;
extern uint8_t ips_new_job;
extern uint8_t ips_mqtt_response[IPS_MQTT_RESPONSE_SIZE];


/* NAMING CONSTANT DECLARATIONS ------------------------------------------------------ */
#define MAX_SIZE    512				// MQTT max payload


/* MACRO DECLARATIONS ---------------------------------------------------------------- */
#define CHECK(x) \
    do { \
        if (!(x)) { \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x); \
            exit(-1); \
        } \
    } while (0) \


/* GLOBAL VARIABLE DECLARATIONS ------------------------------------------------------- */
uint8_t ips_new_job= NO_IO_CMD;
uint8_t ips_mqtt_response[IPS_MQTT_RESPONSE_SIZE]; /* record payload for responsing to main*/


/* EXPORTED SUBPROGRAM BODIES -------------------------------------------------------- */
/***********************************************************
 *	Function 	  : ips_json_parse
 *	Description : Parse JSON string and get responsing message.
 *	Param 		  : char *payload: json string
 *                uint8_t *setFunc:
 *                uint8_t *response_string: Return responsing message
 *	Return		  : error number
 *************************************************************/
int ips_json_parse(char * payload, uint8_t *setFunc, uint8_t *response_string){ 
  char response[16];
  struct json_object *root, *j_response;
  struct json_object *j_subsystem,*j_cmd;
  
  IPSLOG(3, "[IPS] : json_parse payload=%s\n", payload);
  if(setFunc == NULL) return -1;
  root = (struct json_object*)json_tokener_parse(payload);
  /* parsing "IPS" */
  j_subsystem = (struct json_object*)json_object_object_get(root, "IPS");
  if(j_subsystem != NULL){
    IPSLOG(0, "[IPS] : It's my command : Camera\n");
    /* parsing "cmd" */
    j_cmd = (struct json_object*)json_object_object_get(j_subsystem, "cmd");
    IPSLOG(0, "[IPS] : cmd=%s\n", json_object_get_string(j_cmd));
    strcpy((char*)setFunc, json_object_get_string(j_cmd));
    if(strcmp((char*)json_object_get_string(j_cmd), "CAMERA_SET_MODE") == 0){
      /* do not parse below anymore, we chose using data structure instead of MQTT */
      j_response = (struct json_object*)json_object_object_get(j_subsystem, "response");
      IPSLOG(0, "[IPS] : response=%s\n", json_object_get_string(j_response));
      strcpy(response, json_object_get_string(j_response));

      IPSLOG(0, "type=%s\n", camera_paraStr[camera_grab.type]);
      IPSLOG(0, "triggerMode=%s\n", camera_paraStr[camera_grab.triggerMode]);
      IPSLOG(0, "exposureTime=%d\n", camera_grab.exposureTime);
      IPSLOG(0, "SaveImgOpt=%s\n", camera_paraStr[camera_grab.saveImgOpt]);
      IPSLOG(0, "path=%s\n", camera_grab.saveImgPath);
      ips_new_job = TRIGGER_SET_PROCESS;
      if(response_string != NULL){
        strcpy((char*)response_string, response);
      }     
    }else {
      IPSLOG(0, "[IPS] : This command is not for me, ignore it\n");
    }
  }	
  /* free object */
  json_object_put(root);
  return 0;
}

/***********************************************************
 *	Function 	  : ips_message_callback
 *	Description : It's callback function that deals with the 
 *                message from mqtt service.
 *	Param 		  : mosquitto *ext_mosq:
 *                void *obj:
 *                mosquitto_message *message:
 *	Return		  : NONE
 *************************************************************/
void ips_message_callback(struct mosquitto *ips_mosq, void *obj, const struct mosquitto_message *message)
{
  struct mosq_config *mqcfg;
  int i;
  bool res;
  uint8_t setFunc[16], response_string[16];
  
  IPSLOG(1, "__%s__\n", __func__);
  if(ips_process_messages == false) return;
  
  assert(obj);
  mqcfg = (struct mosq_config *)obj;
  
  if(message->retain && mqcfg->no_retain) return;
  if(message->payloadlen == 0) return;
  if(mqcfg->filter_outs){
    for(i=0; i<mqcfg->filter_out_count; i++){
      mosquitto_topic_matches_sub(mqcfg->filter_outs[i], message->topic, &res);
      if(res){
        IPSLOG(0, "[IPS] : MQTT filter_outs subscribed --> %s/%s\n", message->topic, message->payload);
      }
    }
  }else{
    if(message->payloadlen == (const int)strlen((const char*)message->payload)) {

      IPSLOG(0, "[IPS] : MQTT subscribed topic \"%s\"--> %s\n", message->topic, message->payload);
      
      /* parse json from payload */
      ips_json_parse((char*)message->payload, setFunc, response_string);

      /* save this payload, it can be used when send back to main control */
      strcpy((char*)ips_mqtt_response, (char*)message->payload);
    }else {
      IPSLOG(0, "[IPS] : MQTT ERR subscribed payload length is different, check it ...!!!\n");
      exit(1);
    }	
  }	
}

/***********************************************************
 *	Function 	  : ips_connect_callback
 *	Description : It's callback function that subscribes all
 *                mqtt topics when a connection is successful.
 *	Param 		  : mosquitto *ext_mosq:
 *                void *obj:
 *                int32_t result:
 *	Return		  : NONE
 *************************************************************/
static int mid;
void ips_connect_callback(struct mosquitto *ips_mosq, void *obj, int result)
{
  struct mosq_config *mqcfg;
  int i;
  
  IPSLOG(0, "__%s__\n", __func__);
  assert(obj);
  mqcfg = (struct mosq_config *)obj;
  
  for(i = 0 ; i < mqcfg->topic_count ; i++){
    IPSLOG(1, "set topic %d = %s\n", i , mqcfg->topics[i]);
    mosquitto_subscribe(ips_mosq, &mid, mqcfg->topics[i], mqcfg->qos);
  }	
}

/***********************************************************
 *	Function 	  : ips_subscribe_callback
 *	Description : Provides feedback on MQTT client status 
 *                and subscription details when a subscription 
 *                operation is successful.
 *	Param 		  : mosquitto *ext_mosq:
 *                void *obj:
 *                int32_t mid:
 *                int32_t qos_count:
 *                int32_t *granted_qos:
 *	Return		  : NONE
 *************************************************************/
void ips_subscribe_callback(struct mosquitto *ips_mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
  struct mosq_config *mqcfg;
  int i;
  
  IPSLOG(1, "__%s__\n", __func__);
  assert(obj);
  mqcfg = (struct mosq_config *)obj;
  if(!mqcfg->quiet) IPSLOG(1, "Subscribed (mid: %d, QoS: %d, QoS count=%d)", mid, granted_qos[0], qos_count);
  for(i=1; i<qos_count; i++){
    if(!mqcfg->quiet) printf(", %d", granted_qos[i]);
  }
  if(!mqcfg->quiet) printf("\n");
}

/***********************************************************
 *	Function 	  : ips_log_callback
 *	Description : It's a callback function which gets triggered 
 *                whenever a subscribe event occurs.
 *	Param 		  : mosquitto *ext_mosq:
 *                void *obj:
 *                int32_t level:
 *                char *str:
 *	Return		  : NONE
 *************************************************************/
void ips_log_callback(struct mosquitto *ips_mosq, void *obj, int level, const char *str)
{
	printf("%s\n", str);
}

/***********************************************************
 *	Function 	: ips_init_config
 *	Description : Used to set various parameters for MQTT connection and message handling.
 *	Param 		: mosq_config *ext_cfg: Client configuration structure
 *	Return		: NONE
 *************************************************************/
void ips_init_config(struct mosq_config *ips_cfg)
{
  memset(ips_cfg, 0, sizeof(*ips_cfg));
  ips_cfg->id = (char*)"vision_box_ips";
  ips_cfg->host = "localhost";
  ips_cfg->port = 1883;
  ips_cfg->max_inflight = 20;
  ips_cfg->keepalive = 60;
  ips_cfg->clean_session = true;
  ips_cfg->eol = true;
  ips_cfg->qos = 2;
  ips_cfg->protocol_version = MQTT_PROTOCOL_V31;
  // for subscription
  ips_cfg->topic_count = 1;
  ips_cfg->topics = (char**)realloc(ips_cfg->topics, ips_cfg->topic_count*sizeof(char *));
  ips_cfg->topics[0] = (char*)"Px/VISIONBOX/internal/cmd";
  // for publishing
  ips_cfg->topic = (char*)realloc(ips_cfg->topic, sizeof(char *));
  ips_cfg->topic = (char*)"Px/VISIONBOX/internal/response";
}

/***********************************************************
 *	Function 	  : ips_client_id_generate
 *	Description : Generating the MQTT client identifier (Client ID).
 *	Param 		  : mosq_config *ext_cfg: Client configuration structure
 *                char *id_base: 
 *	Return		  : NONE
 *************************************************************/
int ips_client_id_generate(struct mosq_config *ips_cfg, const char *id_base)
{
  int len;
  char hostname[256];
  
  if(ips_cfg->id_prefix){
    ips_cfg->id = (char*)malloc(strlen(ips_cfg->id_prefix)+10);
    if(!ips_cfg->id){
      if(!ips_cfg->quiet) fprintf(stderr, "Error: Out of memory.\n");
      mosquitto_lib_cleanup();
      return 1;
    }
    snprintf(ips_cfg->id, strlen(ips_cfg->id_prefix)+10, "%s%d", ips_cfg->id_prefix, getpid());
  }else if(!ips_cfg->id){
    hostname[0] = '\0';
    gethostname(hostname, 256);
    hostname[255] = '\0';
    len = strlen(id_base) + strlen("/-") + 6 + strlen(hostname);
    ips_cfg->id = (char*)malloc(len);
    if(!ips_cfg->id){
      if(!ips_cfg->quiet) fprintf(stderr, "Error: Out of memory.\n");
      mosquitto_lib_cleanup();
      return 1;
    }
    snprintf(ips_cfg->id, len, "%s/%d-%s", id_base, getpid(), hostname);
    if(strlen(ips_cfg->id) > MOSQ_MQTT_ID_MAX_LENGTH){
      /* Enforce maximum client id length of 23 characters */
      ips_cfg->id[MOSQ_MQTT_ID_MAX_LENGTH] = '\0';
    }
  }
  return MOSQ_ERR_SUCCESS;
}

/***********************************************************
 *	Function 	  : ips_mqtt_subscriber
 *	Description : Establishes and operates an MQTT client for 
 *                subscribing to and receiving MQTT messages.
 *	Param 		  : NONE 
 *	Return		  : error number
 *************************************************************/
int ips_mqtt_subscriber()
{
  int rc;
  
  /* initial parameters */
  ips_cfg = (mosq_config*)malloc(sizeof(struct mosq_config));
  ips_init_config(ips_cfg);
  
  mosquitto_lib_init();
  
  if(ips_client_id_generate(ips_cfg, "iotsub")){
    return 1;
  }
  ips_mosq = mosquitto_new (ips_cfg->id, ips_cfg->clean_session, ips_cfg);
  if(!ips_mosq){
    switch(errno){
      case ENOMEM:
        if(!ips_cfg->quiet) fprintf(stderr, "Error: Out of memory.\n");
        break;
      case EINVAL:
        if(!ips_cfg->quiet) fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
        break;
    }
    mosquitto_lib_cleanup();
    return 1;
  }
  mosquitto_log_callback_set(ips_mosq, ips_log_callback);
  mosquitto_subscribe_callback_set(ips_mosq, ips_subscribe_callback);
  mosquitto_connect_callback_set(ips_mosq, ips_connect_callback);
  mosquitto_message_callback_set(ips_mosq, ips_message_callback);
  rc = mosquitto_connect (ips_mosq, ips_cfg->host, ips_cfg->port, ips_cfg->keepalive);
  if(rc) return rc;
  
  rc = mosquitto_loop_forever(ips_mosq, -1, 1);
  
  mosquitto_destroy(ips_mosq);
  mosquitto_lib_cleanup();
  
  if(ips_cfg->msg_count>0 && rc == MOSQ_ERR_NO_CONN){
    rc = 0;
  }
  if(rc){
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
  }
  return rc;
}

/***********************************************************
 *	Function 	: ips_mqtt_publisher
 *	Description : Publish a JSON string message to the configured MQTT topic.
 *	Param 		: char *jString: 
 *	Return		: error number
 *************************************************************/
int ips_mqtt_publisher(char *jString)
{
  char text[512];
  int ret;
  
  IPSLOG(1, "%s, jString=%s\n", __func__, jString);
  
  sprintf(text, "%s", jString);
  IPSLOG(0, "[IPS] : MQTT publish topic \"%s\" --> \n", ips_cfg->topic);
  IPSLOG(0, "%s", text);
  
  // Publish the message to the topic
  ret = mosquitto_publish (ips_mosq, &mid, ips_cfg->topic, strlen (text), text, ips_cfg->qos, ips_cfg->retain);
  if (ret) {
    fprintf (stderr, "Can't publish to Mosquitto server\n");
    exit (-1);
  } 

  return ret;
}

