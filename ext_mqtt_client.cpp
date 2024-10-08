/**
 ******************************************************************************
 * @file    int_mqtt_client.c
 * @brief   The utility tool about mqtt client for External system.
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
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <mqueue.h>
#include <mosquitto.h>
// #include <json.h>
#include "client_shared.h"
#include "ipsCtl/IPS_CompFunction.h"
#include "ipsCtl/IPS_CompAlgorithm.h"
#include "iosCtl/IOS_CompFunction.h"

#include <string>
#include <chrono>
#include <ctime>

#include "ext_mqtt_client.hpp"
#include "common.hpp"
#include "mainCtl.hpp"
#include "global.hpp"
#include "json.h"

/* GLOBAL VARIABLE DECLARATIONS ------------------------------------------------------- */
extern WIFI_AP_PARA wifi;
enum
{
    SLAVE_ID,
    FUNCTION_CODE,
    ADDRESS,
    LENGTH,
    CMD_MAX
};
static const char *publish_topic[] = { "PX/VBS/Resp/Cam1" , "PX/VBS/Resp/Cam2" };
static const char *subscribe_topic[] =  { "PX/VBS/Cmd/Cam1", "PX/VBS/Cmd/Cam2" };
static struct mosquitto *ext_mosq = nullptr; /* external MQTT for backend communication */
static struct mosq_config *ext_cfg;
extern unsigned char ios_cameraid;
extern char trig_DinMode;
extern char trig_DinMode_Dual;

pthread_mutex_t ext_mqtt_publisher_Dual_Mutex = PTHREAD_MUTEX_INITIALIZER;

bool ext_process_messages = true;

/* NAMING CONSTANT DECLARATIONS ------------------------------------------------------ */
#define MAX_SIZE 4096 // MQTT max payload

/* MACRO DECLARATIONS ---------------------------------------------------------------- */
#define CHECK(x)                                            \
    do                                                      \
    {                                                       \
        if (!(x))                                           \
        {                                                   \
            fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
            perror(#x);                                     \
            exit(-1);                                       \
        }                                                   \
    } while (0)


/* EXPORTED SUBPROGRAM BODIES -------------------------------------------------------- */
/***********************************************************
 *	Function 	: ext_gettime_ms
 *	Description : Get current time(ms)
 *	Param 		: NONE
 *	Return		: msec
 *************************************************************/
static uint32_t ext_gettime_ms(void)
{
    struct timeval tv;
#if !defined(_MSC_VER)
    gettimeofday(&tv, nullptr);
    return (uint32_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
    return GetTickCount();
#endif
}

/***********************************************************
 *	Function 	: json_parse_IOS
 *	Description : Parse JSON string for IOS and update the 
 *                corresponding system status.
 *	Param 		: 
 *	Return		: error number
 *************************************************************/
int json_parse_IOS(struct json_object *root, uint8_t *setFunc, const int iID)
{

    seIO_JsonInfo seInfo;
    memset(&seInfo, 0x00, sizeof(seIO_JsonInfo));

    struct json_object *j_subsystem;
    int ret = -1;
    
    /* parsing "CMS" */
    j_subsystem = (struct json_object *)json_object_object_get(root, "cmd");
    if(j_subsystem) {
        std::string strCAM = json_object_get_string(j_subsystem);
        ret = setIO_ParamAssign(strCAM.c_str(), (struct json_object *)root, &seInfo);
        if(ret >= 0) {
            IO_JsonQ_EnQ(seInfo);
        } else {
            MAINLOG(0, "[MAIN] %s()%d: ret=[%d]\n", __FUNCTION__, __LINE__, ret);
        }
    } else {
        MAINLOG(0, "[MAIN] %s()%d: j_subsystem=%s fail.\n", __FUNCTION__, __LINE__, j_subsystem);
        ret = -1;
    }
    
    return ret;
}

/***********************************************************
 *	Function 	: ext_json_parse_Dual
 *	Description : Parse JSON string and update the 
 *                corresponding system status.
 *	Param 		: char *payload: json string
 *                uint8_t *setFunc:
 *                int iID:
 *	Return		: error number
 *************************************************************/
int ext_json_parse_Dual(uint8_t *payload, uint8_t *setFunc, const int iID)
{
    struct json_object *root;
    struct json_object *j_subsystem;

    seJsonInfo seInfo;
    memset(&seInfo, 0x00, sizeof(seJsonInfo));

    MAINLOG(3, "[MAIN] : json_parse payload=[%s]\n", payload);
    if (setFunc == nullptr)
        return -1;
    root = (struct json_object *)json_tokener_parse((const char *)payload);

    //////////////////////////////////////////////////////////////////////////

    struct json_object *tmp_obj_array = nullptr;
    
    const char *mqtt_cmd_header[] = {"CMD_START", "ROI_", "CMD_END"};
    const char *mqtt_type_header[] = {"Normal", "ARRAY"};

    int iSetlection = 0; // 0:Noraml, 1:Array < AutoRun Mode >
    tmp_obj_array = json_object_object_get(root, mqtt_type_header[1]);

    // Selection the MQTT type. Identity is 0:Noraml, 1:Array < AutoRun Mode >
    if (!tmp_obj_array)
    {
        iSetlection = 0; // Normal
    }
    else
    {
        iSetlection = 1; // Array < AuotRun Mode >
    }

    if (0 == iSetlection) // mqtt_type_header[0] : "Normal"
    {
        MAINLOG(0, "iSelection[ %02d ] is %s. ", iSetlection, mqtt_type_header[iSetlection]);

        /* parsing "CMS" */
        j_subsystem = (struct json_object *)json_object_object_get(root, "cmd");

        strcpy((char *)setFunc, json_object_get_string(j_subsystem));

        if (j_subsystem != nullptr)
        {
            int ret = 0;
            //================= IOS function.start ===================
            if((ret = json_parse_IOS(root, setFunc, iID)) == 0) {

                MAINLOG(0, "Running %s object of Json ret=[%d]\n", "IOS", ret);
            }     
            //================= IOS function.End ===================                        

            //================= IPS function.start ===================
            else {

                MAINLOG(0, "Running %s object of Json\n", "IPS");

                // ===>>> START >>>
                
                std::string strCAM = json_object_get_string(j_subsystem);
                
                setAlgo_ParamAssign_Dual(strCAM.c_str(), (struct json_object *)root, &seInfo, iID);
                
                memset(setFunc, '\0', strlen((const char *)setFunc));
                memcpy(setFunc, seInfo.szCmd, strlen(seInfo.szCmd));
                fprintf(stderr, "%s()%d: c_str=[%s] 1111111111111111111\n", __FUNCTION__, __LINE__, strCAM.c_str());
                JsonQ_EnQ_Dual(seInfo, iID);
                
                memset(&seInfo, 0x00, sizeof(seJsonInfo));

                // <<< END <<<===
            }
            //================= IPS function.End ===================

            std::string str(reinterpret_cast<char *>(setFunc));
            innerQ_Main_EnQ(str);
        }
    }
    //================= AutoMode Array of Json.start ===================
    else // mqtt_type_header[1] : "ARRAY"
    {

        MAINLOG(0, "iSelection[ %02d ] is %s.", iSetlection, mqtt_type_header[iSetlection]);

        // get the length of the array.
        int array_mqttlength = json_object_array_length(tmp_obj_array);

        MAINLOG(0, "@ ===> %s size = %d\n", "ARRAY", array_mqttlength);

        // get each item from the array.
        for (int i = 0; i < array_mqttlength; i++)
        {
            std::string strKey;
            if (i == 0 || i == (array_mqttlength - 1))
            { // CMD_START, CMD_END
                if (i == 0)
                {
                    strKey = mqtt_cmd_header[0]; // CMD_START
                }
                else
                {
                    strKey = mqtt_cmd_header[2]; // CMD_END
                }
            }
            else
            { // ROI_x [x:1 ~ n]
                string strTmp = mqtt_cmd_header[1];
                strKey = strTmp + std::to_string(i);
            }

            MAINLOG(0, "@ strKey ===> %s \n", strKey.c_str());

            struct json_object *item_obj_id = json_object_array_get_idx(tmp_obj_array, i);
            struct json_object *tmp_obj_item = (struct json_object *)json_object_object_get(item_obj_id, strKey.c_str());

            if (!tmp_obj_item)
            {
                MAINLOG(0, "[ITEM] : This command is not for me, ignore it\n");
                break;
            }

            int item_mqttlength = json_object_array_length(tmp_obj_item);
            MAINLOG(0, "@===> %s size = %d\n", strKey.c_str(), item_mqttlength);

            for (int x = 0; x < item_mqttlength; x++)
            {
                std::string strElementKey = "cmd";

                struct json_object *elem_obj_id = json_object_array_get_idx(tmp_obj_item, x);
                struct json_object *tmp_obj_element = (struct json_object *)json_object_object_get(elem_obj_id, strElementKey.c_str());

                if (tmp_obj_element != nullptr)
                {

                    std::string strCAM = json_object_get_string(tmp_obj_element);
                    MAINLOG(0, "@===> %s [ %d ] = %s\n", strKey.c_str(), x, json_object_get_string(tmp_obj_element));

                    setAlgo_ParamAssign_Dual(strCAM.c_str(), (struct json_object *)elem_obj_id, &seInfo, iID);

                    memcpy(setFunc, seInfo.szCmd, strlen(seInfo.szCmd));

                    JsonQ_EnQ_Dual(seInfo, iID);
                    usleep(50); /* delay 0.05 ms */

                    innerQ_Main_EnQ(seInfo.szCmd);

                    usleep(50); /* delay 0.05 ms */

                    memset(&seInfo, 0x00, sizeof(seJsonInfo));
                }
                else
                {

                    MAINLOG(0, "[MAIN] : This command is not for me, ignore it\n");
                }
            }
        }
    }
    //================= end of AutoMode Array of Json ===================

    /* free object */
    json_object_put(root);

    return 0;
}

/***********************************************************
 *	Function 	: ext_message_callback_Dual
 *	Description : It's callback function that deals with the 
 *                message from mqtt service.
 *	Param 		: mosquitto *ext_mosq:
 *                void *obj:
 *                mosquitto_message *message:
 *	Return		: NONE
 *************************************************************/
static int8_t setFunc[1024] = {'\0'};
void ext_message_callback_Dual(struct mosquitto *ext_mosq, void *obj, const struct mosquitto_message *message)
{
    struct mosq_config *mqcfg;
    int32_t i;
    bool res;

    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration(0);
    std::string strtmp;

    //cycletime_start >>
    start = std::chrono::high_resolution_clock::now();


    MAINLOG(1, "__%s__\n", __func__);
    if (ext_process_messages == false)
        return;

    assert(obj);
    mqcfg = (struct mosq_config *)obj;

    if (message->retain && mqcfg->no_retain)
        return;
    if (message->payloadlen == 0)
        return;
    if (mqcfg->filter_outs)
    {
        for (i = 0; i < mqcfg->filter_out_count; i++)
        {
            mosquitto_topic_matches_sub(mqcfg->filter_outs[i], message->topic, &res);
            if (res)
            {
                MAINLOG(0, "[MQTT]: filter_outs subscribed --> %s/%p\n", message->topic, message->payload);
            }
        }
    }
    else
    {
        MAINLOG(0, " @ >> @ >> [MQTT]: mid = %d, len=%d, qos=%d, retain=%d\n", message->mid, message->payloadlen, message->qos, message->retain);
        MAINLOG(0, "[MQTT]: message->payload --> %p\n", message->payload);
        if (message->payloadlen == (int)strlen((const char*)message->payload))
        {
            // # cycletime_end <<
            end = std::chrono::high_resolution_clock::now();
            duration = end - start;
            strtmp = std::to_string(duration.count());
            MAINLOG(0, RED "[__%s__] CycleTime : %s (ms)\n", "01", strtmp.c_str());

            //cycletime_start >>
            start = std::chrono::high_resolution_clock::now();

            /////////////////////////////////////////////////////////////////////////////
            // Parsing MQTT command by the seAlgoParamReg definition ---> start
            /////////////////////////////////////////////////////////////////////////////

            memset(setFunc, '\0', sizeof(setFunc));

            MAINLOG(0, LIGHT_GREEN "%d [MQTT-EXT] : message->topic \"%s\"-->\n" NONE, ext_gettime_ms(), message->topic);
            if(!strcmp(message->topic, subscribe_topic[0])) {
                ios_cameraid = 0;
            } else if(!strcmp(message->topic, subscribe_topic[1])) {
                ios_cameraid = 1;
            } else {
                MAINLOG(0, "Error: Unknow topic %s ...!!!\n", message->topic);
                ios_cameraid = 0;
            }
            // "PX/VBS/Cmd/Cam02"
            if ( strcmp(message->topic, subscribe_topic[1]) == 0 ) {
                
                MAINLOG(0, LIGHT_GREEN "%d [MQTT-EXT] : subscribe_topic[1] \"%s\"-->\n" NONE, ext_gettime_ms(), subscribe_topic[1]);                

                if (ext_json_parse_Dual((uint8_t*)message->payload, (uint8_t*)setFunc, 1) != 0)
                {
                    sprintf((char*)setFunc, "%s", "MQTT_PARSER_ERROR");
                }          
            }
            else {  //"PX/VBS/Cmd/Cam01"

                MAINLOG(0, LIGHT_GREEN "%d [MQTT-EXT] : subscribe_topic[0] \"%s\"-->\n" NONE, ext_gettime_ms(), subscribe_topic[0]);                     

                if (ext_json_parse_Dual((uint8_t*)message->payload, (uint8_t*)setFunc, 0) != 0)
                {
                    sprintf((char*)setFunc, "%s", "MQTT_PARSER_ERROR");
                }                            
            }      

            /////////////////////////////////////////////////////////////////////////////
            // Parsing MQTT command by the seAlgoParamReg definition <--- end
            /////////////////////////////////////////////////////////////////////////////

            // # cycletime_end <<
            end = std::chrono::high_resolution_clock::now();
            duration = end - start;
            strtmp = std::to_string(duration.count());
            MAINLOG(0, RED "[__%s__] CycleTime : %s (ms)\n", "02", strtmp.c_str());

        }
        else
        {
            MAINLOG(0, "[MAIN-ECT] : MQTT ERR subscribed payload length is different, check it ...!!!\n");
            exit(1);
        }
    }
}

static int32_t mid;
/***********************************************************
 *	Function 	: ext_connect_callback_Dual
 *	Description : Provides feedback on MQTT client status 
 *                and subscription details when a subscription 
 *                operation is successful.
 *	Param 		: mosquitto *ext_mosq:
 *                void *obj:
 *                int32_t mid:
 *                int32_t qos_count:
 *                int32_t *granted_qos:
 *	Return		: NONE
 *************************************************************/
void ext_connect_callback_Dual(struct mosquitto *ext_mosq, void *obj, int32_t result)
{
    struct mosq_config *mqcfg;
    int32_t i;

    MAINLOG(0, "__%s__\n", __func__);
    assert(obj);
    mqcfg = (struct mosq_config *)obj;

    for (i = 0; i < mqcfg->topic_count; i++)
    {
        MAINLOG(1, "set topics_sub[%d] = %s\n", i, mqcfg->topics_sub[i]);
        mosquitto_subscribe(ext_mosq, &mid, mqcfg->topics_sub[i], mqcfg->qos);
    }
}

/***********************************************************
 *	Function 	: ext_init_config_Dual
 *	Description : Used to set various parameters for MQTT connection and message handling.
 *	Param 		: mosq_config *ext_cfg: Client configuration structure
 *	Return		: NONE
 *************************************************************/
void ext_init_config_Dual(struct mosq_config *ext_cfg)
{
    memset(ext_cfg, 0, sizeof(*ext_cfg));
    ext_cfg->host = "localhost";
    ext_cfg->port = 1883;
    ext_cfg->debug = false;
    ext_cfg->max_inflight = 20;
    ext_cfg->keepalive = 60;
    ext_cfg->clean_session = true;
    ext_cfg->eol = true;
    ext_cfg->qos = 0;
    ext_cfg->retain = 0;
    ext_cfg->protocol_version = MQTT_PROTOCOL_V311;
    /* for subscription */
    ext_cfg->topic_count = 2;
    ext_cfg->topics_sub = (char**)realloc(ext_cfg->topics_sub, ext_cfg->topic_count * sizeof(char *));
    ext_cfg->topics_sub[0] = strdup(subscribe_topic[0]);  //"PX/VBS/Cmd/Cam01";
    ext_cfg->topics_sub[1] = strdup(subscribe_topic[1]);  //"PX/VBS/Cmd/Cam02";
    /* for publishing */
    ext_cfg->topics_pub = (char**)realloc(ext_cfg->topics_pub, ext_cfg->topic_count * sizeof(char *));
    ext_cfg->topics_pub[0] = strdup(publish_topic[0]);   // "PX/VBS/Resp/Cam01";
    ext_cfg->topics_pub[1] = strdup(publish_topic[1]);   // "PX/VBS/Resp/Cam02";
}

/***********************************************************
 *	Function 	: ext_mqtt_release
 *	Description : Disconnect mqtt.
 *	Param 		: NONE 
 *	Return		: NONE
 *************************************************************/
void ext_mqtt_release()
{
    mosquitto_disconnect(ext_mosq);
}

/***********************************************************
 *	Function 	: ext_client_id_generate
 *	Description : Generating the MQTT client identifier (Client ID).
 *	Param 		: mosq_config *ext_cfg: Client configuration structure
 *                char *id_base: 
 *	Return		: NONE
 *************************************************************/
int32_t ext_client_id_generate(struct mosq_config *ext_cfg, const char *id_base)
{
    int32_t len;
    char hostname[256];

    if (ext_cfg->id_prefix)
    {
        ext_cfg->id = (char*)malloc(strlen(ext_cfg->id_prefix) + 10);
        if (!ext_cfg->id)
        {
            if (!ext_cfg->quiet)
                fprintf(stderr, "Error: Out of memory.\n");
            mosquitto_lib_cleanup();
            return 1;
        }
        snprintf(ext_cfg->id, strlen(ext_cfg->id_prefix) + 10, "%s%d", ext_cfg->id_prefix, getpid());
    }
    else if (!ext_cfg->id)
    {
        hostname[0] = '\0';
        gethostname(hostname, 256);
        hostname[255] = '\0';
        len = strlen(id_base) + strlen("/-") + 6 + strlen(hostname);
        ext_cfg->id = (char*)malloc(len);
        if (!ext_cfg->id)
        {
            if (!ext_cfg->quiet)
                fprintf(stderr, "Error: Out of memory.\n");
            mosquitto_lib_cleanup();
            return 1;
        }
        snprintf(ext_cfg->id, len, "%s/%d-%s", id_base, getpid(), hostname);
        if (strlen(ext_cfg->id) > MOSQ_MQTT_ID_MAX_LENGTH)
        {
            /* Enforce maximum client id length of 23 characters */
            ext_cfg->id[MOSQ_MQTT_ID_MAX_LENGTH] = '\0';
        }
    }
    return MOSQ_ERR_SUCCESS;
}

/***********************************************************
 *	Function 	: ext_mqtt_subscriber_Dual
 *	Description : Establishes and operates an MQTT client for 
 *                subscribing to and receiving MQTT messages.
 *	Param 		: NONE 
 *	Return		: error number
 *************************************************************/
int32_t ext_mqtt_subscriber_Dual()
{
    int32_t rc;

    MAINLOG(0, " @@ >> %s, ---> . ---> ext_mqtt_subscriber : Init\n", __func__);

    /* initial parameters */
    ext_cfg = (mosq_config*)malloc(sizeof(struct mosq_config));

    ext_init_config_Dual(ext_cfg);

    mosquitto_lib_init();

    if (ext_client_id_generate(ext_cfg, "vbox_id_"))
        {
            return 1;
        }

    MAINLOG(0, " @ >> %s, --- . ---> clientid : %s\n", __func__, ext_cfg->id);

    ext_mosq = mosquitto_new(ext_cfg->id, ext_cfg->clean_session, ext_cfg);
    if (!ext_mosq)
    {
        switch (errno)
        {
        case ENOMEM:
            if (!ext_cfg->quiet)
                fprintf(stderr, "Error: Out of memory.\n");
            break;
        case EINVAL:
            if (!ext_cfg->quiet)
                fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
            break;
        }
        mosquitto_lib_cleanup();
        return 1;
    }

    mosquitto_connect_callback_set(ext_mosq, ext_connect_callback_Dual);    //Dual_camera
    mosquitto_message_callback_set(ext_mosq, ext_message_callback_Dual);    //Dual_camera
    rc = mosquitto_connect(ext_mosq, ext_cfg->host, ext_cfg->port, ext_cfg->keepalive);
    if (rc)
        return rc;

    MAINLOG(0, " @@ >> %s, ---> . ---> mosquitto_loop_forever : Start\n", __func__);

    rc = mosquitto_loop_forever(ext_mosq, -1, 1);

    MAINLOG(0, " @@ >> %s, <--- . <--- mosquitto_loop_forever : End\n", __func__);

    mosquitto_destroy(ext_mosq);
    mosquitto_lib_cleanup();

    if (ext_cfg->msg_count > 0 && rc == MOSQ_ERR_NO_CONN)
    {
        rc = 0;
    }
    if (rc)
    {
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        MAINLOG(0, " ?? >> %s, ---> . ---> mosquitto_strerror : [ %d ] \n", __func__, rc);
    }

    MAINLOG(0, " @@ >> %s, <--- . <--- ext_mqtt_subscriber : Exit\n", __func__);

    return rc;
}

/***********************************************************
 *	Function 	: ext_mqtt_publisher_Dual
 *	Description : Publish a JSON string message to the configured MQTT topic.
 *	Param 		: char *jString: 
 *                bool bCameID:
 *	Return		: error number
 *************************************************************/
int32_t ext_mqtt_publisher_Dual(char *jString, const bool bCameID)
{
    pthread_mutex_lock(&ext_mqtt_publisher_Dual_Mutex);
    std::string strText(jString);

    int ret = 0;

    MAINLOG(1, "%s, jString=%s\n", __func__, jString);

    MAINLOG(0, LIGHT_PURPLE "[MAIN-EXT] : MQTT publish topics_pub[%d] \"%s\" --> %s\n" NONE, bCameID, ext_cfg->topics_pub[bCameID], strText.c_str());

    // Publish the message to the topic
    ret = mosquitto_publish(ext_mosq, &mid, ext_cfg->topics_pub[bCameID], strText.size(), strText.c_str(), ext_cfg->qos, ext_cfg->retain);

    if (ret)
    {
        fprintf(stderr, "Can't publish to Mosquitto server\n");
        printf("Error (%d) publishing message.\n", ret);
        exit(-1);
    }
    pthread_mutex_unlock(&ext_mqtt_publisher_Dual_Mutex);
    return ret;
}