/*
Copyright (c) 2009-2014 Roger Light <roger@atchoo.org>

All rights reserved. This program and the accompanying materials
are made available under the terms of the Eclipse Public License v1.0
and Eclipse Distribution License v1.0 which accompany this distribution.

The Eclipse Public License is available at
   http://www.eclipse.org/legal/epl-v10.html
and the Eclipse Distribution License is available at
  http://www.eclipse.org/org/documents/edl-v10.php.

Contributors:
   Roger Light - initial implementation and documentation.
*/

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
#include "../common.h"

#include <chrono>
#include <ctime>

extern wifi_start_station();
// extern WIFI_PARA wifi;
extern ETH_PARA eth;
extern int wifi_update_hostapd(unsigned char *ssid, unsigned char *pwd);
extern void wifi_restart_ap();
extern int eth0_set_ip(bool mode, char *ip, char *gateway);
extern int get_ip_by_ifname(char *ifname, char *ip);
extern void wifi_turn_off();
extern void wifi_turn_on();
extern WIFI_AP_PARA wifi;
enum
{
    SLAVE_ID,
    FUNCTION_CODE,
    ADDRESS,
    LENGTH,
    CMD_MAX
};

extern int gpio_export(unsigned int gpio);
extern int gpio_set_dir(unsigned int gpio, unsigned int out_flag);
extern int gpio_set_value(unsigned int gpio, unsigned int value);
extern int gpio_set_edge(unsigned int gpio, char *edge);
extern int gpio_fd_open(unsigned int gpio);
extern int gpio_fd_close(int fd);

static unsigned char *publish_topic[] = { "PX/VBS/Cmd/Cam1", "PX/VBS/Cmd/Cam2" };
static unsigned char *subscribe_topic[] =  { "PX/VBS/Resp/Cam1" , "PX/VBS/Resp/Cam2" };
struct mosquitto *backend_mosq = nullptr; /* external MQTT for backend communication */
struct mosq_config *backend_cfg;
static struct json_object *root, *jpub;

#define End_GpioCtrl (0)

static unsigned int gpioPin = 13;

bool backend_process_messages = true;

#define MAX_SIZE 512 // MQTT max payload

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

static auto gStart = std::chrono::high_resolution_clock::now();
static auto gEnd = std::chrono::high_resolution_clock::now();
static std::chrono::duration<double, std::milli> duration(0);
static std::string strtmp;

static void backend_sig(int32_t sig)
{
    fflush(stderr);
    printf("signal %d caught\n", sig);
    fflush(stdout);
    /* should call mosquitto_destroy(backend_mosq) &
      mosquitto_lib_cleanup() for normally
      disconnecting client ? */
    mosquitto_lib_cleanup();
    mosquitto_disconnect(backend_mosq);
    exit(1);
}

static uint32_t backend_gettime_ms(void)
{
    struct timeval tv;
#if !defined(_MSC_VER)
    gettimeofday(&tv, nullptr);
    return (uint32_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
    return GetTickCount();
#endif
}

void backend_json_response(RESPONSE_TYPE type, char *jstring)
{
    struct json_object *obj_1, *obj_2, *obj_3;
    unsigned char eth0_ip[16] = {'\0'};
    unsigned char gateway_ip[16] = {'\0'};
    FILE *fd;

    BACKENDLOG(3, "%s, %d\n", __func__, type);

    switch (type)
    {
    case WRITE_SSID_PW:

        break;
    case WRITE_AP_SSID_PW:

        break;
    case WIFI_TURN_OFF:

        break;
    case WIFI_TURN_ON:

        break;
    case WRITE_IPCONFIG:

        break;
    case READ_IPCONFIG:

        break;
    default:
        break;
    }
}

int32_t backend_json_parse(char *payload)
{
    char jString[MAX_SIZE];
    unsigned char ssid[64], pwd[16];
    unsigned char ethernet_ip[16], gateway_ip[16];
    bool enable_dhcp, enable_wifi;
    bool reboot = false;
    struct json_object *root, *j_param;
    struct json_object *j_cmd, *j_temp;

    BACKENDLOG(3, "json_parse payload=%s\n", payload);
    root = (struct json_object *)json_tokener_parse(payload);
    // parsing "cmd"
    j_cmd = (struct json_object *)json_object_object_get(root, "cmd");
    BACKENDLOG(3, "cmd=%s\n", json_object_get_string(j_cmd));
    if (j_cmd != nullptr)
    {
        if (strcmp((char *)json_object_get_string(j_cmd), "WRITE_SSID_PW") == 0)
        {
        }
        else if (strcmp((char *)json_object_get_string(j_cmd), "WRITE_AP_SSID_PW") == 0)
        {
        }
        else if (strcmp((char *)json_object_get_string(j_cmd), "WIFI_TURN_OFF") == 0)
        {
        }
        else if (strcmp((char *)json_object_get_string(j_cmd), "WIFI_TURN_ON") == 0)
        {
        }
        else if (strcmp((char *)json_object_get_string(j_cmd), "WRITE_IPCONFIG") == 0)
        {
        }
        else if (strcmp((char *)json_object_get_string(j_cmd), "READ_IPCONFIG") == 0)
        {
        }
    }
    // Decrease counter and free object
    json_object_put(root);
    return 0;
}

int backend_json_parse_ForGpioTrigger(char *payload)
{
    int res = 0;

    struct json_object *root = nullptr;
    struct json_object *j_cmd = nullptr;

    const char *strAlgoName = "GLUEWIDTHMEAS_ANNULUS_GET_RESP";

    BACKENDLOG(3, "json_parse payload=%s\n", payload);
    root = (struct json_object *)json_tokener_parse(payload);

    // parsing "cmd"
    j_cmd = (struct json_object *)json_object_object_get(root, "cmd");
    BACKENDLOG(3, "cmd=%s\n", json_object_get_string(j_cmd));

    if (j_cmd != nullptr)
    {

        if (!strcmp(json_object_get_string(j_cmd), strAlgoName))
        {

            BACKENDLOG(0, "cmd=%s is equal %s\n", json_object_get_string(j_cmd), strAlgoName);

#if (End_GpioCtrl)

            printf("gpio_(%d) set to high\n", gpioPin);
            gpio_set_value(gpioPin, 1); // gpio set to high

            sleep(2);

            printf("gpio_(%d) set to low\n", gpioPin);
            gpio_set_value(gpioPin, 0); // gpio set to low

#endif
        }
    }
    else
    {

        BACKENDLOG(0, "cmd=%s isn't equal %s\n", json_object_get_string(j_cmd), strAlgoName);
    }

    return res;
}

void backend_message_callback(struct mosquitto *backend_mosq, void *obj, const struct mosquitto_message *message)
{
    struct mosq_config *mqcfg;
    int32_t i;
    bool res;

    BACKENDLOG(1, "__%s__\n", __func__);
    if (backend_process_messages == false)
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
                BACKENDLOG(0, "[MQTT]: filter_outs subscribed --> %s/%s\n", message->topic, message->payload);
            }
        }
    }
    else
    {
#if 0	
  printf("[MQTT]: mid = %d, len=%d, qos=%d, retain=%d\n", message->mid, message->payloadlen,
        message->qos, message->retain);
#else
        if (message->payloadlen == strlen(message->payload))
        {

            BACKENDLOG(0, LIGHT_GREEN "[MQTT-EXT] : MQTT subscribed topic \"%s\"-->\n" NONE, message->topic);
            BACKENDLOG(0, "%s\n", message->payload);

            gEnd = std::chrono::high_resolution_clock::now();
            duration = gEnd - gStart;
            strtmp = std::to_string(duration.count());
            BACKENDLOG(0, YELLOW "[BACKENDLOG] : cycleTime : %s (ms)\n", strtmp.c_str());

            /* parse json from payload */
            // backend_json_parse(message->payload);

            // 20240409_rextyw_command out for daul camera impelement and testing.
            // backend_json_parse_ForGpioTrigger(message->payload);
        }
        else
        {
            BACKENDLOG(0, "[MAIN-ECT] : MQTT ERR subscribed payload length is different, check it ...!!!\n");
            exit(1);
        }
#endif
    }
}
static int32_t mid;
void backend_connect_callback(struct mosquitto *backend_mosq, void *obj, int32_t result)
{
    struct mosq_config *mqcfg;
    int32_t i;

    BACKENDLOG(0, "__%s__\n", __func__);
    assert(obj);
    mqcfg = (struct mosq_config *)obj;

    for (i = 0; i < mqcfg->topic_count; i++)
    {
        BACKENDLOG(1, "set topic %d = %s, QoS = %d\n", i, mqcfg->topics[i], mqcfg->qos);
        mosquitto_subscribe(backend_mosq, &mid, mqcfg->topics[i], mqcfg->qos);
    }
}

void backend_connect_callback_Dual(struct mosquitto *backend_mosq, void *obj, int32_t result)
{
    struct mosq_config *mqcfg;
    int32_t i;

    BACKENDLOG(0, "__%s__\n", __func__);
    assert(obj);
    mqcfg = (struct mosq_config *)obj;

    for (i = 0; i < mqcfg->topic_count; i++)
    {
        BACKENDLOG(1, "set topics_sub[%d] = %s, QoS = %d\n", i, mqcfg->topics_sub[i], mqcfg->qos);
        mosquitto_subscribe(backend_mosq, &mid, mqcfg->topics_sub[i], mqcfg->qos);
    }
}

void backend_subscribe_callback(struct mosquitto *backend_mosq, void *obj, int32_t mid, int32_t qos_count, const int32_t *granted_qos)
{
    struct mosq_config *mqcfg;
    int32_t i;

    BACKENDLOG(1, "__%s__\n", __func__);
    assert(obj);
    mqcfg = (struct mosq_config *)obj;
    if (!mqcfg->quiet)
        BACKENDLOG(1, "Subscribed (mid: %d, QoS: %d, QoS count=%d)", mid, granted_qos[0], qos_count);
    for (i = 1; i < qos_count; i++)
    {
        if (!mqcfg->quiet)
            printf(", %d", granted_qos[i]);
    }
    if (!mqcfg->quiet)
        printf("\n");
}

void backend_log_callback(struct mosquitto *backend_mosq, void *obj, int32_t level, const char *str)
{
    printf("%s\n", str);
}

void backend_init_config(struct mosq_config *backend_cfg)
{
    memset(backend_cfg, 0, sizeof(*backend_cfg));
    backend_cfg->id = "vision_box_backend_Cam1";
    backend_cfg->host = "localhost";
    backend_cfg->port = 1883;
    backend_cfg->max_inflight = 20;
    backend_cfg->keepalive = 60;
    backend_cfg->clean_session = true;
    backend_cfg->eol = true;
    // backend_cfg->qos = 2;
    backend_cfg->qos = 0;
    backend_cfg->retain = 0;
    backend_cfg->protocol_version = MQTT_PROTOCOL_V311;
    /* for subscription */
    backend_cfg->topic_count = 1;
    backend_cfg->topics = realloc(backend_cfg->topics, backend_cfg->topic_count * sizeof(char *));
    backend_cfg->topics[0] = "PX/VBS/Resp/Cam1";
    /* for publishing */
    backend_cfg->topic = realloc(backend_cfg->topic, sizeof(char *));
    backend_cfg->topic = "PX/VBS/Cmd/Cam1";
#if 0 // disable topic filter inspection	
  backend_cfg->filter_outs = realloc(backend_cfg->filter_outs, backend_cfg->topic_count*sizeof(char *));
  backend_cfg->filter_out_count = backend_cfg->topic_count;
  backend_cfg->filter_outs[0] = backend_cfg->topics[0];
  backend_cfg->filter_outs[1] = backend_cfg->topics[1];
#endif
}

void backend_init_config_Dual(struct mosq_config *backend_cfg)
{
    memset(backend_cfg, 0, sizeof(*backend_cfg));
    backend_cfg->id = "vision_box_backend_Cam1";
    backend_cfg->host = "localhost";
    backend_cfg->port = 1883;
    backend_cfg->max_inflight = 20;
    backend_cfg->keepalive = 60;
    backend_cfg->clean_session = true;
    backend_cfg->eol = true;
    // backend_cfg->qos = 2;
    backend_cfg->qos = 0;
    backend_cfg->retain = 0;
    backend_cfg->protocol_version = MQTT_PROTOCOL_V311;
    /* for subscription */
    backend_cfg->topic_count = 2;
    backend_cfg->topics_sub = realloc(backend_cfg->topics_sub, backend_cfg->topic_count * sizeof(char *));
    backend_cfg->topics_sub[0] = strdup(subscribe_topic[0]);   //"PX/VBS/Resp/Cam1";
    backend_cfg->topics_sub[1] = strdup(subscribe_topic[1]);    //"PX/VBS/Resp/Cam2";
    /* for publishing */
    backend_cfg->topics_pub = realloc(backend_cfg->topics_pub, backend_cfg->topic_count * sizeof(char *));
    backend_cfg->topics_pub[0] = strdup(publish_topic[0]);    //"PX/VBS/Cmd/Cam1";
    backend_cfg->topics_pub[1] = strdup(publish_topic[1]);   //"PX/VBS/Cmd/Cam2";
#if 0 // disable topic filter inspection	
  backend_cfg->filter_outs = realloc(backend_cfg->filter_outs, backend_cfg->topic_count*sizeof(char *));
  backend_cfg->filter_out_count = backend_cfg->topic_count;
  backend_cfg->filter_outs[0] = backend_cfg->topics[0];
  backend_cfg->filter_outs[1] = backend_cfg->topics[1];
#endif
}

int32_t backend_client_id_generate(struct mosq_config *backend_cfg, const char *id_base)
{
    int32_t len;
    char hostname[256];

    if (backend_cfg->id_prefix)
    {
        backend_cfg->id = malloc(strlen(backend_cfg->id_prefix) + 10);
        if (!backend_cfg->id)
        {
            if (!backend_cfg->quiet)
                fprintf(stderr, "Error: Out of memory.\n");
            mosquitto_lib_cleanup();
            return 1;
        }
        snprintf(backend_cfg->id, strlen(backend_cfg->id_prefix) + 10, "%s%d", backend_cfg->id_prefix, getpid());
    }
    else if (!backend_cfg->id)
    {
        hostname[0] = '\0';
        gethostname(hostname, 256);
        hostname[255] = '\0';
        len = strlen(id_base) + strlen("/-") + 6 + strlen(hostname);
        backend_cfg->id = malloc(len);
        if (!backend_cfg->id)
        {
            if (!backend_cfg->quiet)
                fprintf(stderr, "Error: Out of memory.\n");
            mosquitto_lib_cleanup();
            return 1;
        }
        snprintf(backend_cfg->id, len, "%s/%d-%s", id_base, getpid(), hostname);
        if (strlen(backend_cfg->id) > MOSQ_MQTT_ID_MAX_LENGTH)
        {
            /* Enforce maximum client id length of 23 characters */
            backend_cfg->id[MOSQ_MQTT_ID_MAX_LENGTH] = '\0';
        }
    }
    return MOSQ_ERR_SUCCESS;
}

int32_t backend_mqtt_subscriber()
{
    int32_t rc;

#if (End_GpioCtrl)

    /* init gpio */
    gpio_export(gpioPin);
    gpio_set_dir(gpioPin, 1); // gpio1_09 set to out
    gpio_set_edge(gpioPin, "rising");
    gpio_set_value(gpioPin, 0); // gpio set to low

#endif

    /* initial parameters */
    backend_cfg = malloc(sizeof(struct mosq_config));
    // backend_init_config(backend_cfg);
    backend_init_config_Dual(backend_cfg);

    mosquitto_lib_init();

    if (backend_client_id_generate(backend_cfg, "backendsub"))
    {
        return 1;
    }
    backend_mosq = mosquitto_new(backend_cfg->id, backend_cfg->clean_session, backend_cfg);
    if (!backend_mosq)
    {
        switch (errno)
        {
        case ENOMEM:
            if (!backend_cfg->quiet)
                fprintf(stderr, "Error: Out of memory.\n");
            break;
        case EINVAL:
            if (!backend_cfg->quiet)
                fprintf(stderr, "Error: Invalid id and/or clean_session.\n");
            break;
        }
        mosquitto_lib_cleanup();
        return 1;
    }
    mosquitto_log_callback_set(backend_mosq, backend_log_callback);
    mosquitto_subscribe_callback_set(backend_mosq, backend_subscribe_callback);
    mosquitto_connect_callback_set(backend_mosq, backend_connect_callback_Dual);
    mosquitto_message_callback_set(backend_mosq, backend_message_callback);
    rc = mosquitto_connect(backend_mosq, backend_cfg->host, backend_cfg->port, backend_cfg->keepalive);
    if (rc)
        return rc;

    rc = mosquitto_loop_forever(backend_mosq, -1, 1);

    mosquitto_destroy(backend_mosq);
    mosquitto_lib_cleanup();

    if (backend_cfg->msg_count > 0 && rc == MOSQ_ERR_NO_CONN)
    {
        rc = 0;
    }
    if (rc)
    {
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
    }
    return rc;
}

int32_t backend_mqtt_publisher(char *jString)
{
    std::string strText(jString);
    int32_t ret;

    // sprintf(text, "%s", jString);
    BACKENDLOG(0, LIGHT_PURPLE "[BACKENDLOG] : MQTT publish topic \"%s\" --->\n# >>  %s\n" NONE, backend_cfg->topic, strText.c_str());

    // Publish the message to the topic
    ret = mosquitto_publish(backend_mosq, &mid, backend_cfg->topic, strText.size(), strText.c_str(), backend_cfg->qos, backend_cfg->retain);

    // cycletime_start
    gStart = std::chrono::high_resolution_clock::now();


    if (ret)
    {
        fprintf(stderr, "Can't publish to Mosquitto server\n");
        exit(-1);
    }
}

int32_t backend_mqtt_publisher_Dual(char *jString, const int iCameID)
{
    std::string strText(jString);
    int32_t ret;

    // sprintf(text, "%s", jString);
    BACKENDLOG(0, LIGHT_PURPLE "[BACKENDLOG] : MQTT publish topic_pub[%d] \"%s\" --->\n# >>  %s\n" NONE, iCameID, backend_cfg->topics_pub[iCameID], strText.c_str());

    // Publish the message to the topic
    ret = mosquitto_publish(backend_mosq, &mid, backend_cfg->topics_pub[iCameID], strText.size(), strText.c_str(), backend_cfg->qos, backend_cfg->retain);

    // cycletime_start
    gStart = std::chrono::high_resolution_clock::now();


    if (ret)
    {
        fprintf(stderr, "Can't publish to Mosquitto server\n");
        exit(-1);
    }
}