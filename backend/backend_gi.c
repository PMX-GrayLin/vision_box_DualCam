#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> /* memset */
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <mqueue.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
// #include <semaphore.h>
#include <json.h>
#include "../common.h"
// #include "mainCtl.h"

/* SW Add */
#include <semaphore.h>
// #include "mediaCtl.h"

#include <poll.h>

/* define msgQ type
   1 is from main to peripheral
   2 is from peripheral to main */
#define SEND_2_EXTERNAL
#define RECEIVE_FROM_EXTERNAL
#define MSGQ_SEND_MCU SEND_2_EXTERNAL      // 1
#define MSGQ_REC_MCU RECEIVE_FROM_EXTERNAL // 2
#define MSGQ_SEND_NET SEND_2_EXTERNAL      // 1
#define MSGQ_REC_NET RECEIVE_FROM_EXTERNAL // 2

#define ENB_VSB_AUTORUN_MODE

#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

#define End_GpioCtrl (0)

extern int32_t backend_mqtt_subscriber();
extern int32_t backend_mqtt_publisher(char *jString);
extern int32_t backend_mqtt_publisher_Dual(char *jString, const int bCameID);
extern int gpio_export(unsigned int gpio);
extern int gpio_set_dir(unsigned int gpio, unsigned int out_flag);
extern int gpio_set_value(unsigned int gpio, unsigned int value);
extern int gpio_set_edge(unsigned int gpio, char *edge);
extern int gpio_fd_open(unsigned int gpio);
extern int gpio_fd_close(int fd);

uint32_t backendDebugLevel;
#define BACKENDLOG(level, format, b...)     if ( (backendDebugLevel+1) >= (level+1) )  printf("%s:%s()%d:" LIGHT_BLUE format NONE, __FILE__, __FUNCTION__, __LINE__, ##b)

pthread_t thread1, thread2, thread3;

static int _exit_;
static char buf[40960] = { 0 };

typedef struct __MSGBUF__
{
    long mtype;
    char mtext[MAX_MSG_SIZE];
} MSGBUF;


// static UINT32 mainDebugLevel = 0;
// #define BACKENDLOG(level, format, b...)     if ( (mainDebugLevel+1) >= (level+1) )  printf(BLUE format NONE, ##b)

unsigned char mainCtl_testVolFlg = 0;
unsigned char mainCtl_testVolCnt = 0;

/*********************************************************************
        Prototype
*********************************************************************/
extern int mqtt_init();
/*********************************************************************
        Tools
*********************************************************************/
#define LOG(x)                           \
    {                                    \
        static char str[32];             \
        if (strcmp(x, &str) != 0)        \
        {                                \
            printf(x);                   \
            memset(str, 0, sizeof(str)); \
            strcpy(str, x);              \
        }                                \
    }

/*********************************************************************
        Function
*********************************************************************/

void sigExit_main(int sig)
{
    BACKENDLOG(0, "[BACKENDLOG] Got %s signal\n", __func__);
    BACKENDLOG(0, "[BACKENDLOG] signal %d caught\n", sig);
    fflush(stderr);
    fflush(stdout);
    exit(1);
}

static int sort_fn(const void *j1, const void *j2)
{
    struct json_object *const *jso1, *const *jso2;
    int i1, i2;

    jso1 = (struct json_object *const *)j1;
    jso2 = (struct json_object *const *)j2;
    if (!*jso1 && !*jso2)
        return 0;
    if (!*jso1)
        return -1;
    if (!*jso2)
        return 1;

    i1 = json_object_get_int(*jso1);
    i2 = json_object_get_int(*jso2);

    return i1 - i2;
}

///<=== IPS <==== IPS //////////////////////////////////////////////////////////

int camera_autorunning_json_create_TESTING(char *jstring)
{
    struct json_object *main_root, *root, *obj_2, *obj_3, *obj_4, *obj_5, *obj_6, *obj_7;
    struct json_object *array0, *array1, *array2, *array3, *array4, *array5;
    struct json_object *array_root, *array_root1, *array_root2, *array_root3, *array_root4;

    if (jstring == nullptr)
        return -1;

    main_root = (struct json_object *)json_object_new_object();

    array0 = (struct json_object *)json_object_new_array();
    if (!array0)
    {
        return -1;
    }
    array1 = (struct json_object *)json_object_new_array();
    if (!array1)
    {
        return -1;
    }
    array2 = (struct json_object *)json_object_new_array();
    if (!array2)
    {
        return -1;
    }
    array3 = (struct json_object *)json_object_new_array();
    if (!array3)
    {
        return -1;
    }
    array4 = (struct json_object *)json_object_new_array();
    if (!array4)
    {
        return -1;
    }

    array_root = (struct json_object *)json_object_new_object();
    if (!array_root)
    {
        return -1;
    }
    array_root1 = (struct json_object *)json_object_new_object();
    if (!array_root1)
    {
        return -1;
    }
    array_root2 = (struct json_object *)json_object_new_object();
    if (!array_root2)
    {
        return -1;
    }
    array_root3 = (struct json_object *)json_object_new_object();
    if (!array_root3)
    {
        return -1;
    }
    array_root4 = (struct json_object *)json_object_new_object();
    if (!array_root4)
    {
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // ROI_1 --> _Array
    //////////////////////////////////////////////////////////////////////////////////

    /* create json format */
    //  AUTORUNNING_SET_PARAM ===> True
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("AUTORUNNING_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "Enb_AutoRunning", json_object_new_int(1));
    json_object_object_add(obj_2, "Enb_TriggerMode", json_object_new_int(1));

    json_object_array_add(array1, root);

    json_object_object_add(array_root1, "CMD_START", array1);
    json_object_array_add(array0, array_root1);

    //////////////////////////////////////////////////////////////////////////////////
    // Array.start
    //////////////////////////////////////////////////////////////////////////////////

    // // CAMERA_SET_CONFIG ?? You only need to set it once.
    // root = (struct json_object *)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_CONFIG"));

    // obj_2 = (struct json_object *)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);

    // // json_object_object_add(obj_2, "PixelFormat", json_object_new_string("BayerGR8"));	 //0:Mono8, 1:BayerGR8
    // json_object_object_add(obj_2, "PixelFormat", json_object_new_string("Mono8")); // 0:Mono8, 1:BayerGR8

    // json_object_object_add(obj_2, "Width", json_object_new_int(2592));
    // json_object_object_add(obj_2, "Height", json_object_new_int(1944));
    // json_object_object_add(obj_2, "Offset_X", json_object_new_int(0));
    // json_object_object_add(obj_2, "Offset_Y", json_object_new_int(0));

    // // json_object_object_add(obj_2, "BinningScale", json_object_new_int(1));
    // json_object_object_add(obj_2, "BinningScale", json_object_new_int(2));

    // json_object_object_add(obj_2, "ExposureMode", json_object_new_string("Auto"));
    // // json_object_object_add(obj_2, "ExposureMode", json_object_new_string("Off"));
    // json_object_object_add(obj_2, "ExposureTime", json_object_new_double(311514.44));

    // json_object_array_add(array2, root);

    // # Using Read Image Path
    // CAMERA_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "IsEnbReadImageMode", json_object_new_int(1));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));

    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/Ret_SourceImage.png"));

    json_object_array_add(array2, root);

    //// # CAMERA_STREAM_SET_CAPTURE
    // root = (struct json_object *)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("CAMERA_STREAM_SET_CAPTURE"));
    // obj_2 = (struct json_object *)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);

    // json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/RetStreaming_SourceImage.png"));

    // json_object_array_add(array2, root);

    //  PATTERNMATCH_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("PATTERNMATCH_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_SearchRect", obj_3);
    json_object_object_add(obj_3, "Top", json_object_new_int(800));
    json_object_object_add(obj_3, "Left", json_object_new_int(800));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(1900));
    json_object_object_add(obj_3, "Right", json_object_new_int(2500));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    json_object_object_add(obj_2, "TemplateImgPath", json_object_new_string("/home/user/rextyw/ImageSet/crop_template.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_pattern_match.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  IBOXANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("IBOXANNULUS_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(1396));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(972));
    json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(154.0));
    json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(672.0));
    json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_inspectionbox_annulus.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  CALCOORDINATE_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "CalibCoord_G", obj_5);
    json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
    json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(2354));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(1625));
    json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Top", json_object_new_int(1446));
    json_object_object_add(obj_3, "Left", json_object_new_int(2263));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(1805));
    json_object_object_add(obj_3, "Right", json_object_new_int(2446));

    obj_6 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
    json_object_object_add(obj_6, "Center_X", json_object_new_int(2354));
    json_object_object_add(obj_6, "Center_Y", json_object_new_int(1625));
    json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_6, "Top", json_object_new_int(1446));
    json_object_object_add(obj_6, "Left", json_object_new_int(2263));
    json_object_object_add(obj_6, "Bottom", json_object_new_int(1805));
    json_object_object_add(obj_6, "Right", json_object_new_int(2446));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
    json_object_object_add(obj_4, "Center_X", json_object_new_int(1396));
    json_object_object_add(obj_4, "Center_Y", json_object_new_int(972));
    json_object_object_add(obj_4, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_4, "Top", json_object_new_int(300));
    json_object_object_add(obj_4, "Left", json_object_new_int(724));
    json_object_object_add(obj_4, "Bottom", json_object_new_int(1644));
    json_object_object_add(obj_4, "Right", json_object_new_int(2068));

    obj_7 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
    json_object_object_add(obj_7, "Center_X", json_object_new_int(1396));
    json_object_object_add(obj_7, "Center_Y", json_object_new_int(972));
    json_object_object_add(obj_7, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_7, "Top", json_object_new_int(300));
    json_object_object_add(obj_7, "Left", json_object_new_int(724));
    json_object_object_add(obj_7, "Bottom", json_object_new_int(1644));
    json_object_object_add(obj_7, "Right", json_object_new_int(2068));

    json_object_array_add(array2, root);

    /* create json format */
    // CROPROIIMG_ANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_ANNULUS_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_cropimg_annulus.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  THRESHOLD_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(35));
    json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(200));
    json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_threshold.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  MROPHOLOGY_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(1));
    json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
    json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_morphology.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  MROPHOLOGY_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(2));
    json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
    json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_morphology.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  NOISEREMOVAL_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("NOISEREMOVAL_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "dbLimit_min", json_object_new_double(10.0));
    json_object_object_add(obj_2, "dbLimit_max", json_object_new_double(15000.0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/noiseremoval.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_noiseremoval.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_ANNULUS_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    // obj_3 = (struct json_object*)json_object_new_object();
    // json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    // json_object_object_add(obj_3, "Center_X", json_object_new_int(1550));
    // json_object_object_add(obj_3, "Center_Y", json_object_new_int(930));
    // json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(345.0));
    // json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(672));
    // json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    // json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));

    json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
    json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(10.5));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/noiseremoval.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgMorphology_Resize.png"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_gluewidth_measure_annulus.jpg"));

    json_object_array_add(array2, root);

    json_object_object_add(array_root2, "ROI_1", array2);
    json_object_array_add(array0, array_root2);

    //////////////////////////////////////////////////////////////////////////////////
    // Array.end
    //////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////
    //// ROI_2 --> _Array
    ////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////
    //// Array.start
    ////////////////////////////////////////////////////////////////////////////////////

    //// CAMERA_SET_PARAM
    // root = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_PARAM"));
    // obj_2 = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);
    ////json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/source_20220825.jpg"));
    // json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/source_20220825.png"));

    // json_object_array_add(array2, root);

    ////  PATTERNMATCH_SET_PARAM
    // root = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("PATTERNMATCH_SET_PARAM"));

    // obj_2 = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);
    // obj_3 = (struct json_object*)json_object_new_object();
    // json_object_object_add(obj_2, "ROI_SearchRect", obj_3);
    // json_object_object_add(obj_3, "Top", json_object_new_int(800));
    // json_object_object_add(obj_3, "Left", json_object_new_int(800));
    // json_object_object_add(obj_3, "Bottom", json_object_new_int(1900));
    // json_object_object_add(obj_3, "Right", json_object_new_int(2500));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    // json_object_object_add(obj_2, "TemplateImgPath", json_object_new_string("/home/user/rextyw/ImageSet/crop_template.jpg"));
    // json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_pattern_match.jpg"));

    // json_object_array_add(array3, root);

    ///* create json format */
    ////  IBOXANNULUS_SET_PARAM
    // root = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("IBOXANNULUS_SET_PARAM"));

    // obj_2 = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);
    // obj_3 = (struct json_object*)json_object_new_object();
    // json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    // json_object_object_add(obj_3, "Center_X", json_object_new_int(596));
    // json_object_object_add(obj_3, "Center_Y", json_object_new_int(472));
    // json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(54.0));
    // json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(172.0));
    // json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    // json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    // json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_inspectionbox_annulus.jpg"));

    // json_object_array_add(array3, root);

    ///* create json format */
    ////  CALCOORDINATE_SET_PARAM
    // root = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));

    // obj_2 = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);

    // obj_5 = (struct json_object*)json_object_new_object();
    // json_object_object_add(obj_2, "CalibCoord_G", obj_5);
    // json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
    // json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
    // json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
    // json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
    // json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));

    // obj_3 = (struct json_object*)json_object_new_object();
    // json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
    // json_object_object_add(obj_3, "Center_X", json_object_new_int(2354));
    // json_object_object_add(obj_3, "Center_Y", json_object_new_int(1625));
    // json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
    // json_object_object_add(obj_3, "Top", json_object_new_int(1446));
    // json_object_object_add(obj_3, "Left", json_object_new_int(2263));
    // json_object_object_add(obj_3, "Bottom", json_object_new_int(1805));
    // json_object_object_add(obj_3, "Right", json_object_new_int(2446));

    // obj_6 = (struct json_object*)json_object_new_object();
    // json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
    // json_object_object_add(obj_6, "Center_X", json_object_new_int(2354));
    // json_object_object_add(obj_6, "Center_Y", json_object_new_int(1625));
    // json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
    // json_object_object_add(obj_6, "Top", json_object_new_int(1446));
    // json_object_object_add(obj_6, "Left", json_object_new_int(2263));
    // json_object_object_add(obj_6, "Bottom", json_object_new_int(1805));
    // json_object_object_add(obj_6, "Right", json_object_new_int(2446));

    // obj_4 = (struct json_object*)json_object_new_object();
    // json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
    // json_object_object_add(obj_4, "Center_X", json_object_new_int(596));
    // json_object_object_add(obj_4, "Center_Y", json_object_new_int(472));
    // json_object_object_add(obj_4, "Angle", json_object_new_double(0.0));
    // json_object_object_add(obj_4, "Top", json_object_new_int(200));
    // json_object_object_add(obj_4, "Left", json_object_new_int(324));
    // json_object_object_add(obj_4, "Bottom", json_object_new_int(744));
    // json_object_object_add(obj_4, "Right", json_object_new_int(868));

    // obj_7 = (struct json_object*)json_object_new_object();
    // json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
    // json_object_object_add(obj_7, "Center_X", json_object_new_int(596));
    // json_object_object_add(obj_7, "Center_Y", json_object_new_int(472));
    // json_object_object_add(obj_7, "Angle", json_object_new_double(0.0));
    // json_object_object_add(obj_7, "Top", json_object_new_int(200));
    // json_object_object_add(obj_7, "Left", json_object_new_int(324));
    // json_object_object_add(obj_7, "Bottom", json_object_new_int(744));
    // json_object_object_add(obj_7, "Right", json_object_new_int(868));

    // json_object_array_add(array3, root);

    ///* create json format */
    //// CROPROIIMG_ANNULUS_SET_PARAM
    // root = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_ANNULUS_SET_PARAM"));

    // obj_2 = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);

    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    // json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/cropimg_annulus.jpg"));
    // json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_cropimg_annulus.jpg"));

    // json_object_array_add(array3, root);

    ///* create json format */
    ////  THRESHOLD_SET_PARAM
    // root = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));

    // obj_2 = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);
    // json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(35));
    // json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(200));
    // json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(0));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/cropimg_annulus.jpg"));
    // json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/threshold.jpg"));
    // json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_threshold.jpg"));

    // json_object_array_add(array3, root);

    ///* create json format */
    ////  MROPHOLOGY_SET_PARAM
    // root = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    // obj_2 = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);
    // json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(1));
    // json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
    // json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/threshold.jpg"));
    // json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/morphology.jpg"));
    // json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_morphology.jpg"));

    // json_object_array_add(array3, root);

    ///* create json format */
    ////  MROPHOLOGY_SET_PARAM
    // root = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    // obj_2 = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);
    // json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(2));
    // json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
    // json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
    ////json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/threshold.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/morphology.jpg"));
    // json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/morphology.jpg"));
    // json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_morphology.jpg"));

    // json_object_array_add(array3, root);

    ///* create json format */
    ////  NOISEREMOVAL_SET_PARAM
    // root = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("NOISEREMOVAL_SET_PARAM"));

    // obj_2 = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);
    // json_object_object_add(obj_2, "dbLimit_min", json_object_new_double(10.0));
    // json_object_object_add(obj_2, "dbLimit_max", json_object_new_double(15000.0));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/morphology.jpg"));
    ////json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgThd.png"));
    // json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/noiseremoval.jpg"));
    // json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_noiseremoval.jpg"));

    // json_object_array_add(array3, root);

    ///* create json format */
    ////  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
    // root = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_ANNULUS_SET_PARAM"));
    // obj_2 = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);

    ////obj_3 = (struct json_object*)json_object_new_object();
    ////json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    ////json_object_object_add(obj_3, "Center_X", json_object_new_int(596));
    ////json_object_object_add(obj_3, "Center_Y", json_object_new_int(472));
    ////json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(54.0));
    ////json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(272.0));
    ////json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    ////json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));

    // json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
    // json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(10.5));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/noiseremoval.jpg"));
    // json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_gluewidth_measure_annulus.jpg"));

    // json_object_array_add(array3, root);

    // json_object_object_add(array_root3, "ROI_2", array3);
    // json_object_array_add(array0, array_root3);

    ////////////////////////////////////////////////////////////////////////////////////
    //// Array.end
    ////////////////////////////////////////////////////////////////////////////////////

    /* create json format */
    //  AUTORUNNING_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("AUTORUNNING_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "Enb_AutoRunning", json_object_new_int(0));
    json_object_object_add(obj_2, "Enb_TriggerMode", json_object_new_int(0));

    json_object_array_add(array4, root);

    json_object_object_add(array_root4, "CMD_END", array4);
    json_object_array_add(array0, array_root4);

    //////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////

    // json_object_array_add(array0, array_root);

    json_object_object_add(main_root, "ARRAY", array0);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(main_root, "args", obj_3);
    json_object_object_add(obj_3, "msgId", json_object_new_string("5244_1658801859648"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(main_root));
    BACKENDLOG(0, " json context : %s\n\n", jstring);

    json_object_put(main_root);

    return 0;
}

// Two array settings for automatic running.
int camera_autorunning_json_create_TESTING_2(char *jstring)
{
    struct json_object *main_root, *root, *obj_2, *obj_3, *obj_4, *obj_5, *obj_6, *obj_7;
    struct json_object *array0, *array1;

    if (jstring == nullptr)
        return -1;

    main_root = (struct json_object *)json_object_new_object();

    array0 = (struct json_object *)json_object_new_array();
    if (!array0)
    {
        return -1;
    }

    /* create json format */
    //  AUTORUNNING_SET_PARAM ===> True
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd1", json_object_new_string("AUTORUNNING_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "Enb_AutoRunning", json_object_new_int(1));
    json_object_object_add(obj_2, "Enb_TriggerMode", json_object_new_int(1));

    json_object_array_add(array0, root);

    //////////////////////////////////////////////////////////////////////////////////
    // Array_1
    //////////////////////////////////////////////////////////////////////////////////

    /* create json format */
    //  PATTERNMATCH_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd2", json_object_new_string("PATTERNMATCH_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_SearchRect", obj_3);
    json_object_object_add(obj_3, "Top", json_object_new_int(358));
    json_object_object_add(obj_3, "Left", json_object_new_int(558));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(583));
    json_object_object_add(obj_3, "Right", json_object_new_int(778));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));
    json_object_object_add(obj_2, "TemplateImgPath", json_object_new_string("/home/user/rextyw/ImageSet/crop_template.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_pattern_match.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  IBOXANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd3", json_object_new_string("IBOXANNULUS_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(403));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(283));
    json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(54.0));
    json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(172.0));
    json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_annulus.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  CALCOORDINATE_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd4", json_object_new_string("CALCOORDINATE_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(697));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(468));
    json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Top", json_object_new_int(416));
    json_object_object_add(obj_3, "Left", json_object_new_int(672));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(521));
    json_object_object_add(obj_3, "Right", json_object_new_int(723));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
    json_object_object_add(obj_4, "Center_X", json_object_new_int(396));
    json_object_object_add(obj_4, "Center_Y", json_object_new_int(276));
    json_object_object_add(obj_4, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_4, "Top", json_object_new_int(111));
    json_object_object_add(obj_4, "Left", json_object_new_int(231));
    json_object_object_add(obj_4, "Bottom", json_object_new_int(441));
    json_object_object_add(obj_4, "Right", json_object_new_int(561));

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "CalibCoord_G", obj_5);
    json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
    json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));

    obj_6 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
    json_object_object_add(obj_6, "Center_X", json_object_new_int(0));
    json_object_object_add(obj_6, "Center_Y", json_object_new_int(0));
    json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_6, "Top", json_object_new_int(0));
    json_object_object_add(obj_6, "Left", json_object_new_int(0));
    json_object_object_add(obj_6, "Bottom", json_object_new_int(0));
    json_object_object_add(obj_6, "Right", json_object_new_int(0));

    obj_7 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
    json_object_object_add(obj_7, "Center_X", json_object_new_int(0));
    json_object_object_add(obj_7, "Center_Y", json_object_new_int(0));
    json_object_object_add(obj_7, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_7, "Top", json_object_new_int(0));
    json_object_object_add(obj_7, "Left", json_object_new_int(0));
    json_object_object_add(obj_7, "Bottom", json_object_new_int(0));
    json_object_object_add(obj_7, "Right", json_object_new_int(0));

    json_object_array_add(array0, root);

    /* create json format */
    // CROPROIIMG_ANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd5", json_object_new_string("CROPROIIMG_ANNULUS_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/cropimg_annulus.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_cropimg_annulus.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  THRESHOLD_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd6", json_object_new_string("THRESHOLD_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(35));
    json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(200));
    json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/cropimg_annulus.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/threshold.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_threshold.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  MROPHOLOGY_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd7", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(1));
    json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
    json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/threshold.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgThd.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/morphology.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_morphology.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  MROPHOLOGY_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd8", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(1));
    json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
    json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/threshold.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/morphology.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgThd.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/morphology.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_morphology.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  NOISEREMOVAL_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd9", json_object_new_string("NOISEREMOVAL_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "dbLimit_min", json_object_new_double(10.0));
    json_object_object_add(obj_2, "dbLimit_max", json_object_new_double(15000.0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/morphology.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgThd.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/noiseremoval.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_noiseremoval.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd10", json_object_new_string("GLUEWIDTHMEAS_ANNULUS_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
    json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(10.5));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/noiseremoval.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgMorphology_Resize.png"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_gluewidth_measure_annulus.jpg"));

    json_object_array_add(array0, root);

    //////////////////////////////////////////////////////////////////////////////////
    // Array_2
    //////////////////////////////////////////////////////////////////////////////////

    /* create json format */
    //  PATTERNMATCH_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd11", json_object_new_string("PATTERNMATCH_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_SearchRect", obj_3);
    json_object_object_add(obj_3, "Top", json_object_new_int(358));
    json_object_object_add(obj_3, "Left", json_object_new_int(558));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(583));
    json_object_object_add(obj_3, "Right", json_object_new_int(778));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));
    json_object_object_add(obj_2, "TemplateImgPath", json_object_new_string("/home/user/rextyw/ImageSet/crop_template.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_pattern_match.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  IBOXANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd12", json_object_new_string("IBOXANNULUS_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(403));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(283));
    json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(54.0));
    json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(172.0));
    json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_annulus.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  CALCOORDINATE_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd13", json_object_new_string("CALCOORDINATE_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(697));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(468));
    json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Top", json_object_new_int(416));
    json_object_object_add(obj_3, "Left", json_object_new_int(672));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(521));
    json_object_object_add(obj_3, "Right", json_object_new_int(723));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
    json_object_object_add(obj_4, "Center_X", json_object_new_int(396));
    json_object_object_add(obj_4, "Center_Y", json_object_new_int(276));
    json_object_object_add(obj_4, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_4, "Top", json_object_new_int(111));
    json_object_object_add(obj_4, "Left", json_object_new_int(231));
    json_object_object_add(obj_4, "Bottom", json_object_new_int(441));
    json_object_object_add(obj_4, "Right", json_object_new_int(561));

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "CalibCoord_G", obj_5);
    json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
    json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));

    obj_6 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
    json_object_object_add(obj_6, "Center_X", json_object_new_int(0));
    json_object_object_add(obj_6, "Center_Y", json_object_new_int(0));
    json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_6, "Top", json_object_new_int(0));
    json_object_object_add(obj_6, "Left", json_object_new_int(0));
    json_object_object_add(obj_6, "Bottom", json_object_new_int(0));
    json_object_object_add(obj_6, "Right", json_object_new_int(0));

    obj_7 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
    json_object_object_add(obj_7, "Center_X", json_object_new_int(0));
    json_object_object_add(obj_7, "Center_Y", json_object_new_int(0));
    json_object_object_add(obj_7, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_7, "Top", json_object_new_int(0));
    json_object_object_add(obj_7, "Left", json_object_new_int(0));
    json_object_object_add(obj_7, "Bottom", json_object_new_int(0));
    json_object_object_add(obj_7, "Right", json_object_new_int(0));

    json_object_array_add(array0, root);

    /* create json format */
    // CROPROIIMG_ANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd14", json_object_new_string("CROPROIIMG_ANNULUS_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/cropimg_annulus.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_cropimg_annulus.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  THRESHOLD_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd15", json_object_new_string("THRESHOLD_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(35));
    json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(200));
    json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/cropimg_annulus.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/threshold.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_threshold.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  MROPHOLOGY_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd16", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(1));
    json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
    json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/threshold.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgThd.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/morphology.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_morphology.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  MROPHOLOGY_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd17", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(1));
    json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
    json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/threshold.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/morphology.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgThd.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/morphology.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_morphology.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  NOISEREMOVAL_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd18", json_object_new_string("NOISEREMOVAL_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "dbLimit_min", json_object_new_double(10.0));
    json_object_object_add(obj_2, "dbLimit_max", json_object_new_double(15000.0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/morphology.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgThd.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/noiseremoval.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_noiseremoval.jpg"));

    json_object_array_add(array0, root);

    /* create json format */
    //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd19", json_object_new_string("GLUEWIDTHMEAS_ANNULUS_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
    json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(10.5));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/noiseremoval.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgMorphology_Resize.png"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_gluewidth_measure_annulus.jpg"));

    json_object_array_add(array0, root);

    //////////////////////////////////////////////////////////////////////////////////

    /* create json format */
    //  AUTORUNNING_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd20", json_object_new_string("AUTORUNNING_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "Enb_AutoRunning", json_object_new_int(0));
    json_object_object_add(obj_2, "Enb_TriggerMode", json_object_new_int(0));

    json_object_array_add(array0, root);

    json_object_object_add(main_root, "ARRAY", array0);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(main_root, "args", obj_3);
    json_object_object_add(obj_3, "msgId", json_object_new_string("5244_1658801859648"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(main_root));
    BACKENDLOG(0, " json context : %s\n\n", jstring);

    json_object_put(main_root);

    return 0;
}

// Two array settings for automatic running.
int camera_autorunning_json_create_TESTING_3(char *jstring)
{
    struct json_object *main_root, *root, *obj_2, *obj_3, *obj_4, *obj_5, *obj_6, *obj_7;
    struct json_object *array0, *array1, *array2, *array3, *array4, *array5;
    struct json_object *array_root, *array_root1, *array_root2, *array_root3, *array_root4;

    if (jstring == nullptr)
        return -1;

    main_root = (struct json_object *)json_object_new_object();

    array0 = (struct json_object *)json_object_new_array();
    if (!array0)
    {
        return -1;
    }
    array1 = (struct json_object *)json_object_new_array();
    if (!array1)
    {
        return -1;
    }
    array2 = (struct json_object *)json_object_new_array();
    if (!array2)
    {
        return -1;
    }
    array3 = (struct json_object *)json_object_new_array();
    if (!array3)
    {
        return -1;
    }
    array4 = (struct json_object *)json_object_new_array();
    if (!array4)
    {
        return -1;
    }

    array_root = (struct json_object *)json_object_new_object();
    if (!array_root)
    {
        return -1;
    }
    array_root1 = (struct json_object *)json_object_new_object();
    if (!array_root1)
    {
        return -1;
    }
    array_root2 = (struct json_object *)json_object_new_object();
    if (!array_root2)
    {
        return -1;
    }
    array_root3 = (struct json_object *)json_object_new_object();
    if (!array_root3)
    {
        return -1;
    }
    array_root4 = (struct json_object *)json_object_new_object();
    if (!array_root4)
    {
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // ROI_1 --> _Array
    //////////////////////////////////////////////////////////////////////////////////

    /* create json format */
    //  AUTORUNNING_SET_PARAM ===> True
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("AUTORUNNING_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "Enb_AutoRunning", json_object_new_int(1));
    json_object_object_add(obj_2, "Enb_TriggerMode", json_object_new_int(1));

    json_object_array_add(array1, root);

    json_object_object_add(array_root1, "CMD_START", array1);
    json_object_array_add(array0, array_root1);

    //////////////////////////////////////////////////////////////////////////////////
    // Array.start
    //////////////////////////////////////////////////////////////////////////////////

    /* create json format */
    //
    // root = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_PARAM"));
    // obj_2 = (struct json_object*)json_object_new_object();
    // json_object_object_add(root, "args", obj_2);
    // json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/source_20220822.jpg"));

    // json_object_array_add(array2, root);

    //  PATTERNMATCH_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("PATTERNMATCH_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_SearchRect", obj_3);
    json_object_object_add(obj_3, "Top", json_object_new_int(800));
    json_object_object_add(obj_3, "Left", json_object_new_int(800));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(1900));
    json_object_object_add(obj_3, "Right", json_object_new_int(2500));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    json_object_object_add(obj_2, "TemplateImgPath", json_object_new_string("/home/user/rextyw/ImageSet/crop_template.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_pattern_match.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  IBOXANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("IBOXANNULUS_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(1396));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(972));
    json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(154.0));
    json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(672.0));
    json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_inspectionbox_annulus.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  CALCOORDINATE_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "CalibCoord_G", obj_5);
    json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
    json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(2354));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(1625));
    json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Top", json_object_new_int(1446));
    json_object_object_add(obj_3, "Left", json_object_new_int(2263));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(1805));
    json_object_object_add(obj_3, "Right", json_object_new_int(2446));

    obj_6 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
    json_object_object_add(obj_6, "Center_X", json_object_new_int(2354));
    json_object_object_add(obj_6, "Center_Y", json_object_new_int(1625));
    json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_6, "Top", json_object_new_int(1446));
    json_object_object_add(obj_6, "Left", json_object_new_int(2263));
    json_object_object_add(obj_6, "Bottom", json_object_new_int(1805));
    json_object_object_add(obj_6, "Right", json_object_new_int(2446));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
    json_object_object_add(obj_4, "Center_X", json_object_new_int(1396));
    json_object_object_add(obj_4, "Center_Y", json_object_new_int(972));
    json_object_object_add(obj_4, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_4, "Top", json_object_new_int(300));
    json_object_object_add(obj_4, "Left", json_object_new_int(724));
    json_object_object_add(obj_4, "Bottom", json_object_new_int(1644));
    json_object_object_add(obj_4, "Right", json_object_new_int(2068));

    obj_7 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
    json_object_object_add(obj_7, "Center_X", json_object_new_int(1396));
    json_object_object_add(obj_7, "Center_Y", json_object_new_int(972));
    json_object_object_add(obj_7, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_7, "Top", json_object_new_int(300));
    json_object_object_add(obj_7, "Left", json_object_new_int(724));
    json_object_object_add(obj_7, "Bottom", json_object_new_int(1644));
    json_object_object_add(obj_7, "Right", json_object_new_int(2068));

    json_object_array_add(array2, root);

    /* create json format */
    // CROPROIIMG_ANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_ANNULUS_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_cropimg_annulus.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  THRESHOLD_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(35));
    json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(200));
    json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_threshold.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  MROPHOLOGY_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(1));
    json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
    json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_morphology.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  MROPHOLOGY_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(2));
    json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
    json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_morphology.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  NOISEREMOVAL_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("NOISEREMOVAL_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "dbLimit_min", json_object_new_double(10.0));
    json_object_object_add(obj_2, "dbLimit_max", json_object_new_double(15000.0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/noiseremoval.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_noiseremoval.jpg"));

    json_object_array_add(array2, root);

    /* create json format */
    //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_ANNULUS_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    // obj_3 = (struct json_object*)json_object_new_object();
    // json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    // json_object_object_add(obj_3, "Center_X", json_object_new_int(1550));
    // json_object_object_add(obj_3, "Center_Y", json_object_new_int(930));
    // json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(345.0));
    // json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(672));
    // json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    // json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));

    json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
    json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(10.5));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/noiseremoval.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgMorphology_Resize.png"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_gluewidth_measure_annulus.jpg"));

    json_object_array_add(array2, root);

    json_object_object_add(array_root2, "ROI_1", array2);
    json_object_array_add(array0, array_root2);

    //////////////////////////////////////////////////////////////////////////////////
    // Array.end
    //////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////
    // ROI_2 --> _Array
    //////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////
    // Array.start
    //////////////////////////////////////////////////////////////////////////////////

    /* create json format */
    //  PATTERNMATCH_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("PATTERNMATCH_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_SearchRect", obj_3);
    json_object_object_add(obj_3, "Top", json_object_new_int(800));
    json_object_object_add(obj_3, "Left", json_object_new_int(800));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(1900));
    json_object_object_add(obj_3, "Right", json_object_new_int(2500));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    json_object_object_add(obj_2, "TemplateImgPath", json_object_new_string("/home/user/rextyw/ImageSet/crop_template.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_pattern_match.jpg"));

    json_object_array_add(array3, root);

    /* create json format */
    //  IBOXANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("IBOXANNULUS_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(596));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(472));
    json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(54.0));
    json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(172.0));
    json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_inspectionbox_annulus.jpg"));

    json_object_array_add(array3, root);

    /* create json format */
    //  CALCOORDINATE_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "CalibCoord_G", obj_5);
    json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
    json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(2354));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(1625));
    json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Top", json_object_new_int(1446));
    json_object_object_add(obj_3, "Left", json_object_new_int(2263));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(1805));
    json_object_object_add(obj_3, "Right", json_object_new_int(2446));

    obj_6 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
    json_object_object_add(obj_6, "Center_X", json_object_new_int(2354));
    json_object_object_add(obj_6, "Center_Y", json_object_new_int(1625));
    json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_6, "Top", json_object_new_int(1446));
    json_object_object_add(obj_6, "Left", json_object_new_int(2263));
    json_object_object_add(obj_6, "Bottom", json_object_new_int(1805));
    json_object_object_add(obj_6, "Right", json_object_new_int(2446));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
    json_object_object_add(obj_4, "Center_X", json_object_new_int(596));
    json_object_object_add(obj_4, "Center_Y", json_object_new_int(472));
    json_object_object_add(obj_4, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_4, "Top", json_object_new_int(200));
    json_object_object_add(obj_4, "Left", json_object_new_int(324));
    json_object_object_add(obj_4, "Bottom", json_object_new_int(744));
    json_object_object_add(obj_4, "Right", json_object_new_int(868));

    obj_7 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
    json_object_object_add(obj_7, "Center_X", json_object_new_int(596));
    json_object_object_add(obj_7, "Center_Y", json_object_new_int(472));
    json_object_object_add(obj_7, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_7, "Top", json_object_new_int(200));
    json_object_object_add(obj_7, "Left", json_object_new_int(324));
    json_object_object_add(obj_7, "Bottom", json_object_new_int(744));
    json_object_object_add(obj_7, "Right", json_object_new_int(868));

    json_object_array_add(array3, root);

    /* create json format */
    // CROPROIIMG_ANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_ANNULUS_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/cropimg_annulus.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_cropimg_annulus.jpg"));

    json_object_array_add(array3, root);

    /* create json format */
    //  THRESHOLD_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(35));
    json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(200));
    json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/cropimg_annulus.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/threshold.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_threshold.jpg"));

    json_object_array_add(array3, root);

    /* create json format */
    //  MROPHOLOGY_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(1));
    json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
    json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/threshold.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/morphology.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_morphology.jpg"));

    json_object_array_add(array3, root);

    /* create json format */
    //  MROPHOLOGY_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(2));
    json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
    json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/threshold.jpg"));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/morphology.jpg"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/morphology.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_morphology.jpg"));

    json_object_array_add(array3, root);

    /* create json format */
    //  NOISEREMOVAL_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("NOISEREMOVAL_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "dbLimit_min", json_object_new_double(10.0));
    json_object_object_add(obj_2, "dbLimit_max", json_object_new_double(15000.0));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/morphology.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgThd.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/noiseremoval.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_noiseremoval.jpg"));

    json_object_array_add(array3, root);

    /* create json format */
    //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_ANNULUS_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    // obj_3 = (struct json_object*)json_object_new_object();
    // json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    // json_object_object_add(obj_3, "Center_X", json_object_new_int(596));
    // json_object_object_add(obj_3, "Center_Y", json_object_new_int(472));
    // json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(54.0));
    // json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(272.0));
    // json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    // json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));

    json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
    json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(10.5));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/noiseremoval.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_gluewidth_measure_annulus.jpg"));

    json_object_array_add(array3, root);

    json_object_object_add(array_root3, "ROI_2", array3);
    json_object_array_add(array0, array_root3);

    //////////////////////////////////////////////////////////////////////////////////
    // Array.end
    //////////////////////////////////////////////////////////////////////////////////

    /* create json format */
    //  AUTORUNNING_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("AUTORUNNING_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "Enb_AutoRunning", json_object_new_int(0));
    json_object_object_add(obj_2, "Enb_TriggerMode", json_object_new_int(0));

    json_object_array_add(array4, root);

    json_object_object_add(array_root4, "CMD_END", array4);
    json_object_array_add(array0, array_root4);

    //////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////

    // json_object_array_add(array0, array_root);

    json_object_object_add(main_root, "ARRAY", array0);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(main_root, "args", obj_3);
    json_object_object_add(obj_3, "msgId", json_object_new_string("5244_1658801859648"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(main_root));
    BACKENDLOG(0, " json context : %s\n\n", jstring);

    json_object_put(main_root);

    return 0;
}

int camera_autorunning_RECT_create_TESTING(char *jstring)
{
    struct json_object *main_root, *root, *obj_2, *obj_3, *obj_4, *obj_5, *obj_6, *obj_7;
    struct json_object *array0, *array1, *array2, *array3, *array4, *array5;
    struct json_object *array_root, *array_root1, *array_root2, *array_root3, *array_root4, *array_root5;
    struct json_object *arrayS, *arrayE, *arrayObj_start, *arrayObj_end;

    if (jstring == nullptr)
        return -1;

    main_root = (struct json_object *)json_object_new_object();

    arrayS = (struct json_object *)json_object_new_array();
    if (!arrayS)
    {
        return -1;
    }
    arrayE = (struct json_object *)json_object_new_array();
    if (!arrayE)
    {
        return -1;
    }
    arrayObj_start = (struct json_object *)json_object_new_object();
    if (!arrayObj_start)
    {
        return -1;
    }
    arrayObj_end = (struct json_object *)json_object_new_object();
    if (!arrayObj_end)
    {
        return -1;
    }

    array0 = (struct json_object *)json_object_new_array();
    if (!array0)
    {
        return -1;
    }
    array1 = (struct json_object *)json_object_new_array();
    if (!array1)
    {
        return -1;
    }
    array2 = (struct json_object *)json_object_new_array();
    if (!array2)
    {
        return -1;
    }
    array3 = (struct json_object *)json_object_new_array();
    if (!array3)
    {
        return -1;
    }
    array4 = (struct json_object *)json_object_new_array();
    if (!array4)
    {
        return -1;
    }
    array5 = (struct json_object *)json_object_new_array();
    if (!array5)
    {
        return -1;
    }

    array_root = (struct json_object *)json_object_new_object();
    if (!array_root)
    {
        return -1;
    }
    array_root1 = (struct json_object *)json_object_new_object();
    if (!array_root1)
    {
        return -1;
    }
    array_root2 = (struct json_object *)json_object_new_object();
    if (!array_root2)
    {
        return -1;
    }
    array_root3 = (struct json_object *)json_object_new_object();
    if (!array_root3)
    {
        return -1;
    }
    array_root4 = (struct json_object *)json_object_new_object();
    if (!array_root4)
    {
        return -1;
    }
    array_root5 = (struct json_object *)json_object_new_object();
    if (!array_root5)
    {
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////////
    // --. --> _Array
    //////////////////////////////////////////////////////////////////////////////////

    /* create json format */
    //  AUTORUNNING_SET_PARAM ===> True
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("AUTORUNNING_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "Enb_AutoRunning", json_object_new_int(1));
    json_object_object_add(obj_2, "Enb_TriggerMode", json_object_new_int(1));

    json_object_array_add(arrayS, root);

    json_object_object_add(arrayObj_start, "CMD_START", arrayS);
    json_object_array_add(array0, arrayObj_start);

    //////////////////////////////////////////////////////////////////////////////////
    // [ ROI_1 ] Array.start
    //////////////////////////////////////////////////////////////////////////////////
    if (1)
    {

        // # Using Read Image Path
        // CAMERA_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_PARAM"));
        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        json_object_object_add(obj_2, "IsEnbReadImageMode", json_object_new_int(1));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/SourceImage.png"));
        json_object_array_add(array2, root);

        /* create json format */
        //  IBOXANNULUS_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("IBOXRECT_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        obj_3 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "ROIBB_Rect", obj_3);
        json_object_object_add(obj_3, "Center_X", json_object_new_int(463));
        json_object_object_add(obj_3, "Center_Y", json_object_new_int(310));
        json_object_object_add(obj_3, "Angle", json_object_new_double(270.0));
        json_object_object_add(obj_3, "Top", json_object_new_int(93));
        json_object_object_add(obj_3, "Left", json_object_new_int(402));
        json_object_object_add(obj_3, "Bottom", json_object_new_int(523));
        json_object_object_add(obj_3, "Right", json_object_new_int(527));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/Ret_SourceImage.png"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_rectangle.jpg"));

        json_object_array_add(array2, root);

        /* create json format */
        //  CALCOORDINATE_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        obj_5 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "CalibCoord_G", obj_5);
        json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
        json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
        json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
        json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
        json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));

        obj_3 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
        json_object_object_add(obj_3, "Center_X", json_object_new_int(0));
        json_object_object_add(obj_3, "Center_Y", json_object_new_int(0));
        json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
        json_object_object_add(obj_3, "Top", json_object_new_int(0));
        json_object_object_add(obj_3, "Left", json_object_new_int(0));
        json_object_object_add(obj_3, "Bottom", json_object_new_int(0));
        json_object_object_add(obj_3, "Right", json_object_new_int(0));

        obj_6 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
        json_object_object_add(obj_6, "Center_X", json_object_new_int(0));
        json_object_object_add(obj_6, "Center_Y", json_object_new_int(0));
        json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
        json_object_object_add(obj_6, "Top", json_object_new_int(0));
        json_object_object_add(obj_6, "Left", json_object_new_int(0));
        json_object_object_add(obj_6, "Bottom", json_object_new_int(0));
        json_object_object_add(obj_6, "Right", json_object_new_int(0));

        obj_4 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);

        json_object_object_add(obj_4, "Center_X", json_object_new_int(463));
        json_object_object_add(obj_4, "Center_Y", json_object_new_int(310));
        json_object_object_add(obj_4, "Angle", json_object_new_double(270.0));
        json_object_object_add(obj_4, "Top", json_object_new_int(247));
        json_object_object_add(obj_4, "Left", json_object_new_int(248));
        json_object_object_add(obj_4, "Bottom", json_object_new_int(374));
        json_object_object_add(obj_4, "Right", json_object_new_int(679));

        obj_7 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
        json_object_object_add(obj_7, "Center_X", json_object_new_int(463));
        json_object_object_add(obj_7, "Center_Y", json_object_new_int(310));
        json_object_object_add(obj_7, "Angle", json_object_new_double(270.0));
        json_object_object_add(obj_7, "Top", json_object_new_int(247));
        json_object_object_add(obj_7, "Left", json_object_new_int(248));
        json_object_object_add(obj_7, "Bottom", json_object_new_int(374));
        json_object_object_add(obj_7, "Right", json_object_new_int(679));

        json_object_array_add(array2, root);

        /* create json format */
        // CROPROIIMG_ANNULUS_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_RECT_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_cropimg_annulus.jpg"));

        json_object_array_add(array2, root);

        /* create json format */
        //  THRESHOLD_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);
        json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(60));
        json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(175));
        json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(0));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_threshold.jpg"));

        json_object_array_add(array2, root);

        /* create json format */
        //  MROPHOLOGY_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);
        json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(0));
        json_object_object_add(obj_2, "iKSize", json_object_new_int(5));
        json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(3));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_morphology.jpg"));

        json_object_array_add(array2, root);

        /* create json format */
        //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_RECT_SET_PARAM"));
        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
        json_object_object_add(obj_2, "SearchDirection", json_object_new_int(2)); // 0: Left2Right, 1:Right2Left, 2:Top2Bottom, 3:Bottom2Top
        json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(0.03));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/noiseremoval.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_gluewidth_measure_rectangle.jpg"));

        json_object_array_add(array2, root);

        json_object_object_add(array_root2, "ROI_1", array2);
        json_object_array_add(array0, array_root2);
    }
    //////////////////////////////////////////////////////////////////////////////////
    // [ ROI_1 ] Array.end
    //////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////
    // [ ROI_2] Array.start
    //////////////////////////////////////////////////////////////////////////////////
    if (1)
    {

        // # Using Read Image Path
        // CAMERA_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_PARAM"));
        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        json_object_object_add(obj_2, "IsEnbReadImageMode", json_object_new_int(1));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/SourceImage.png"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/Ret_SourceImage.png"));
        json_object_array_add(array3, root);

        /* create json format */
        //  IBOXANNULUS_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("IBOXRECT_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        obj_3 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "ROIBB_Rect", obj_3);
        json_object_object_add(obj_3, "Center_X", json_object_new_int(1456));
        json_object_object_add(obj_3, "Center_Y", json_object_new_int(310));
        json_object_object_add(obj_3, "Angle", json_object_new_double(270.0));
        json_object_object_add(obj_3, "Top", json_object_new_int(85));
        json_object_object_add(obj_3, "Left", json_object_new_int(1388));
        json_object_object_add(obj_3, "Bottom", json_object_new_int(537));
        json_object_object_add(obj_3, "Right", json_object_new_int(1523));

        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/Ret_SourceImage.png"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_rectangle.jpg"));

        json_object_array_add(array3, root);

        /* create json format */
        //  CALCOORDINATE_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        obj_5 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "CalibCoord_G", obj_5);
        json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
        json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
        json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
        json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
        json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));

        obj_3 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
        json_object_object_add(obj_3, "Center_X", json_object_new_int(0));
        json_object_object_add(obj_3, "Center_Y", json_object_new_int(0));
        json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
        json_object_object_add(obj_3, "Top", json_object_new_int(0));
        json_object_object_add(obj_3, "Left", json_object_new_int(0));
        json_object_object_add(obj_3, "Bottom", json_object_new_int(0));
        json_object_object_add(obj_3, "Right", json_object_new_int(0));

        obj_6 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
        json_object_object_add(obj_6, "Center_X", json_object_new_int(0));
        json_object_object_add(obj_6, "Center_Y", json_object_new_int(0));
        json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
        json_object_object_add(obj_6, "Top", json_object_new_int(0));
        json_object_object_add(obj_6, "Left", json_object_new_int(0));
        json_object_object_add(obj_6, "Bottom", json_object_new_int(0));
        json_object_object_add(obj_6, "Right", json_object_new_int(0));

        obj_4 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
        json_object_object_add(obj_4, "Center_X", json_object_new_int(1456));
        json_object_object_add(obj_4, "Center_Y", json_object_new_int(310));
        json_object_object_add(obj_4, "Angle", json_object_new_double(270.0));
        json_object_object_add(obj_4, "Top", json_object_new_int(242));
        json_object_object_add(obj_4, "Left", json_object_new_int(1230));
        json_object_object_add(obj_4, "Bottom", json_object_new_int(379));
        json_object_object_add(obj_4, "Right", json_object_new_int(1683));

        obj_7 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
        json_object_object_add(obj_7, "Center_X", json_object_new_int(1456));
        json_object_object_add(obj_7, "Center_Y", json_object_new_int(310));
        json_object_object_add(obj_7, "Angle", json_object_new_double(270.0));
        json_object_object_add(obj_7, "Top", json_object_new_int(242));
        json_object_object_add(obj_7, "Left", json_object_new_int(1230));
        json_object_object_add(obj_7, "Bottom", json_object_new_int(379));
        json_object_object_add(obj_7, "Right", json_object_new_int(1683));

        json_object_array_add(array3, root);

        /* create json format */
        // CROPROIIMG_ANNULUS_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_RECT_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_cropimg_annulus.jpg"));

        json_object_array_add(array3, root);

        /* create json format */
        //  THRESHOLD_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);
        json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(70));
        json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(1758));
        json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(0));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_threshold.jpg"));

        json_object_array_add(array3, root);

        /* create json format */
        //  MROPHOLOGY_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);
        json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(0));
        json_object_object_add(obj_2, "iKSize", json_object_new_int(5));
        json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(3));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_morphology.jpg"));

        json_object_array_add(array3, root);

        /* create json format */
        //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_RECT_SET_PARAM"));
        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
        json_object_object_add(obj_2, "SearchDirection", json_object_new_int(2)); // 0: Left2Right, 1:Right2Left, 2:Top2Bottom, 3:Bottom2Top
        json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(0.03));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/noiseremoval.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_gluewidth_measure_rectangle.jpg"));

        json_object_array_add(array3, root);

        json_object_object_add(array_root3, "ROI_2", array3);
        json_object_array_add(array0, array_root3);
    }
    //////////////////////////////////////////////////////////////////////////////////
    // [ ROI_2] Array.end
    //////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////
    // [ ROI_3] Array.start
    //////////////////////////////////////////////////////////////////////////////////
    if (1)
    {

        // # Using Read Image Path
        // CAMERA_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_PARAM"));
        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        json_object_object_add(obj_2, "IsEnbReadImageMode", json_object_new_int(1));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/SourceImage.png"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/Ret_SourceImage.png"));
        json_object_array_add(array4, root);

        /* create json format */
        //  IBOXANNULUS_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("IBOXRECT_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        obj_3 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "ROIBB_Rect", obj_3);
        json_object_object_add(obj_3, "Center_X", json_object_new_int(466));
        json_object_object_add(obj_3, "Center_Y", json_object_new_int(922));
        json_object_object_add(obj_3, "Angle", json_object_new_double(270.0));
        json_object_object_add(obj_3, "Top", json_object_new_int(680));
        json_object_object_add(obj_3, "Left", json_object_new_int(409));
        json_object_object_add(obj_3, "Bottom", json_object_new_int(1157));
        json_object_object_add(obj_3, "Right", json_object_new_int(523));

        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/Ret_SourceImage.png"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_rectangle.jpg"));

        json_object_array_add(array4, root);

        /* create json format */
        //  CALCOORDINATE_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        obj_5 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "CalibCoord_G", obj_5);
        json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
        json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
        json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
        json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
        json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));

        obj_3 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
        json_object_object_add(obj_3, "Center_X", json_object_new_int(0));
        json_object_object_add(obj_3, "Center_Y", json_object_new_int(0));
        json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
        json_object_object_add(obj_3, "Top", json_object_new_int(0));
        json_object_object_add(obj_3, "Left", json_object_new_int(0));
        json_object_object_add(obj_3, "Bottom", json_object_new_int(0));
        json_object_object_add(obj_3, "Right", json_object_new_int(0));

        obj_6 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
        json_object_object_add(obj_6, "Center_X", json_object_new_int(0));
        json_object_object_add(obj_6, "Center_Y", json_object_new_int(0));
        json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
        json_object_object_add(obj_6, "Top", json_object_new_int(0));
        json_object_object_add(obj_6, "Left", json_object_new_int(0));
        json_object_object_add(obj_6, "Bottom", json_object_new_int(0));
        json_object_object_add(obj_6, "Right", json_object_new_int(0));

        obj_4 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
        json_object_object_add(obj_4, "Center_X", json_object_new_int(466));
        json_object_object_add(obj_4, "Center_Y", json_object_new_int(922));
        json_object_object_add(obj_4, "Angle", json_object_new_double(270.0));
        json_object_object_add(obj_4, "Top", json_object_new_int(865));
        json_object_object_add(obj_4, "Left", json_object_new_int(227));
        json_object_object_add(obj_4, "Bottom", json_object_new_int(980));
        json_object_object_add(obj_4, "Right", json_object_new_int(706));

        obj_7 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
        json_object_object_add(obj_7, "Center_X", json_object_new_int(466));
        json_object_object_add(obj_7, "Center_Y", json_object_new_int(922));
        json_object_object_add(obj_7, "Angle", json_object_new_double(270.0));
        json_object_object_add(obj_7, "Top", json_object_new_int(865));
        json_object_object_add(obj_7, "Left", json_object_new_int(227));
        json_object_object_add(obj_7, "Bottom", json_object_new_int(980));
        json_object_object_add(obj_7, "Right", json_object_new_int(706));

        json_object_array_add(array4, root);

        /* create json format */
        // CROPROIIMG_ANNULUS_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_RECT_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_cropimg_annulus.jpg"));

        json_object_array_add(array4, root);

        /* create json format */
        //  THRESHOLD_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);
        json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(70));
        json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(175));
        json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(0));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_threshold.jpg"));

        json_object_array_add(array4, root);

        /* create json format */
        //  MROPHOLOGY_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);
        json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(0));
        json_object_object_add(obj_2, "iKSize", json_object_new_int(5));
        json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(3));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_morphology.jpg"));

        json_object_array_add(array4, root);

        /* create json format */
        //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_RECT_SET_PARAM"));
        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
        json_object_object_add(obj_2, "SearchDirection", json_object_new_int(2)); // 0: Left2Right, 1:Right2Left, 2:Top2Bottom, 3:Bottom2Top
        json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(0.03));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_3/noiseremoval.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_3/ret_gluewidth_measure_rectangle.jpg"));

        json_object_array_add(array4, root);

        json_object_object_add(array_root4, "ROI_3", array4);
        json_object_array_add(array0, array_root4);
    }
    //////////////////////////////////////////////////////////////////////////////////
    // [ ROI_3] Array.end
    //////////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////////
    // [ ROI_4] Array.start
    //////////////////////////////////////////////////////////////////////////////////
    if (1)
    {

        // # Using Read Image Path
        // CAMERA_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_PARAM"));
        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        json_object_object_add(obj_2, "IsEnbReadImageMode", json_object_new_int(1));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/SourceImage.png"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/Ret_SourceImage.png"));
        json_object_array_add(array5, root);

        /* create json format */
        //  IBOXANNULUS_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("IBOXRECT_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        obj_3 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "ROIBB_Rect", obj_3);
        json_object_object_add(obj_3, "Center_X", json_object_new_int(1456));
        json_object_object_add(obj_3, "Center_Y", json_object_new_int(911));
        json_object_object_add(obj_3, "Angle", json_object_new_double(270.0));
        json_object_object_add(obj_3, "Top", json_object_new_int(683));
        json_object_object_add(obj_3, "Left", json_object_new_int(1381));
        json_object_object_add(obj_3, "Bottom", json_object_new_int(1139));
        json_object_object_add(obj_3, "Right", json_object_new_int(1527));

        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/Ret_SourceImage.png"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_rectangle.jpg"));

        json_object_array_add(array5, root);

        /* create json format */
        //  CALCOORDINATE_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        obj_5 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "CalibCoord_G", obj_5);
        json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
        json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
        json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
        json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
        json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));

        obj_3 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
        json_object_object_add(obj_3, "Center_X", json_object_new_int(0));
        json_object_object_add(obj_3, "Center_Y", json_object_new_int(0));
        json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
        json_object_object_add(obj_3, "Top", json_object_new_int(0));
        json_object_object_add(obj_3, "Left", json_object_new_int(0));
        json_object_object_add(obj_3, "Bottom", json_object_new_int(0));
        json_object_object_add(obj_3, "Right", json_object_new_int(0));

        obj_6 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
        json_object_object_add(obj_6, "Center_X", json_object_new_int(0));
        json_object_object_add(obj_6, "Center_Y", json_object_new_int(0));
        json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
        json_object_object_add(obj_6, "Top", json_object_new_int(0));
        json_object_object_add(obj_6, "Left", json_object_new_int(0));
        json_object_object_add(obj_6, "Bottom", json_object_new_int(0));
        json_object_object_add(obj_6, "Right", json_object_new_int(0));

        obj_4 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
        json_object_object_add(obj_4, "Center_X", json_object_new_int(1456));
        json_object_object_add(obj_4, "Center_Y", json_object_new_int(911));
        json_object_object_add(obj_4, "Angle", json_object_new_double(270.0));
        json_object_object_add(obj_4, "Top", json_object_new_int(838));
        json_object_object_add(obj_4, "Left", json_object_new_int(1228));
        json_object_object_add(obj_4, "Bottom", json_object_new_int(985));
        json_object_object_add(obj_4, "Right", json_object_new_int(1685));

        obj_7 = (struct json_object *)json_object_new_object();
        json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
        json_object_object_add(obj_7, "Center_X", json_object_new_int(1456));
        json_object_object_add(obj_7, "Center_Y", json_object_new_int(911));
        json_object_object_add(obj_7, "Angle", json_object_new_double(270.0));
        json_object_object_add(obj_7, "Top", json_object_new_int(838));
        json_object_object_add(obj_7, "Left", json_object_new_int(1228));
        json_object_object_add(obj_7, "Bottom", json_object_new_int(985));
        json_object_object_add(obj_7, "Right", json_object_new_int(1685));

        json_object_array_add(array5, root);

        /* create json format */
        // CROPROIIMG_ANNULUS_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_RECT_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_cropimg_annulus.jpg"));

        json_object_array_add(array5, root);

        /* create json format */
        //  THRESHOLD_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);
        json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(70));
        json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(175));
        json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(0));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_threshold.jpg"));

        json_object_array_add(array5, root);

        /* create json format */
        //  MROPHOLOGY_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);
        json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(0));
        json_object_object_add(obj_2, "iKSize", json_object_new_int(5));
        json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(3));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
        json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_morphology.jpg"));

        json_object_array_add(array5, root);

        /* create json format */
        //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
        root = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_RECT_SET_PARAM"));
        obj_2 = (struct json_object *)json_object_new_object();
        json_object_object_add(root, "args", obj_2);

        json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
        json_object_object_add(obj_2, "SearchDirection", json_object_new_int(2)); // 0: Left2Right, 1:Right2Left, 2:Top2Bottom, 3:Bottom2Top
        json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(0.02));
        json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_4/noiseremoval.jpg"));
        json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_4/ret_gluewidth_measure_rectangle.jpg"));

        json_object_array_add(array5, root);

        json_object_object_add(array_root5, "ROI_4", array5);
        json_object_array_add(array0, array_root5);
    }
    //////////////////////////////////////////////////////////////////////////////////
    // [ ROI_4] Array.end
    //////////////////////////////////////////////////////////////////////////////////

    /* create json format */
    //  AUTORUNNING_SET_PARAM
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("AUTORUNNING_SET_PARAM"));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "Enb_AutoRunning", json_object_new_int(0));
    json_object_object_add(obj_2, "Enb_TriggerMode", json_object_new_int(0));

    json_object_array_add(arrayE, root);

    json_object_object_add(arrayObj_end, "CMD_END", arrayE);
    json_object_array_add(array0, arrayObj_end);

    //////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////

    // json_object_array_add(array0, array_root);

    json_object_object_add(main_root, "ARRAY", array0);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(main_root, "args", obj_3);
    json_object_object_add(obj_3, "msgId", json_object_new_string("5244_1658801859648"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(main_root));
    BACKENDLOG(0, " json context : %s\n\n", jstring);

    json_object_put(main_root);

    return 0;
}

// int camera_autorunning_RECT_create_TESTING(char* jstring)
//{
//     struct json_object* main_root, * root, * obj_2, * obj_3, * obj_4, * obj_5, * obj_6, * obj_7;
//     struct json_object* array0, * array1, * array2, * array3, * array4, * array5;
//     struct json_object* array_root, * array_root1, * array_root2, * array_root3, * array_root4, * array_root5;
//     struct json_object* arrayS, * arrayE, * arrayObj_start, * arrayObj_end;
//
//     if (jstring == nullptr)
//         return -1;
//
//     main_root = (struct json_object*)json_object_new_object();
//
//     arrayS = (struct json_object*)json_object_new_array();
//     if (!arrayS)
//     {
//         return -1;
//     }
//     arrayE = (struct json_object*)json_object_new_array();
//     if (!arrayE)
//     {
//         return -1;
//     }
//     arrayObj_start = (struct json_object*)json_object_new_object();
//     if (!arrayObj_start)
//     {
//         return -1;
//     }
//     arrayObj_end = (struct json_object*)json_object_new_object();
//     if (!arrayObj_end)
//     {
//         return -1;
//     }
//
//
//     array0 = (struct json_object*)json_object_new_array();
//     if (!array0)
//     {
//         return -1;
//     }
//     array1 = (struct json_object*)json_object_new_array();
//     if (!array1)
//     {
//         return -1;
//     }
//     array2 = (struct json_object*)json_object_new_array();
//     if (!array2)
//     {
//         return -1;
//     }
//     array3 = (struct json_object*)json_object_new_array();
//     if (!array3)
//     {
//         return -1;
//     }
//     array4 = (struct json_object*)json_object_new_array();
//     if (!array4)
//     {
//         return -1;
//     }
//     array5 = (struct json_object*)json_object_new_array();
//     if (!array5)
//     {
//         return -1;
//     }
//
//     array_root = (struct json_object*)json_object_new_object();
//     if (!array_root)
//     {
//         return -1;
//     }
//     array_root1 = (struct json_object*)json_object_new_object();
//     if (!array_root1)
//     {
//         return -1;
//     }
//     array_root2 = (struct json_object*)json_object_new_object();
//     if (!array_root2)
//     {
//         return -1;
//     }
//     array_root3 = (struct json_object*)json_object_new_object();
//     if (!array_root3)
//     {
//         return -1;
//     }
//     array_root4 = (struct json_object*)json_object_new_object();
//     if (!array_root4)
//     {
//         return -1;
//     }
//     array_root5 = (struct json_object*)json_object_new_object();
//     if (!array_root5)
//     {
//         return -1;
//     }
//
//     //////////////////////////////////////////////////////////////////////////////////
//     // --. --> _Array
//     //////////////////////////////////////////////////////////////////////////////////
//
//     /* create json format */
//     //  AUTORUNNING_SET_PARAM ===> True
//     root = (struct json_object*)json_object_new_object();
//     json_object_object_add(root, "cmd", json_object_new_string("AUTORUNNING_SET_PARAM"));
//     obj_2 = (struct json_object*)json_object_new_object();
//     json_object_object_add(root, "args", obj_2);
//     json_object_object_add(obj_2, "Enb_AutoRunning", json_object_new_int(1));
//     json_object_object_add(obj_2, "Enb_TriggerMode", json_object_new_int(1));
//
//     json_object_array_add(arrayS, root);
//
//     json_object_object_add(arrayObj_start, "CMD_START", arrayS);
//     json_object_array_add(array0, arrayObj_start);
//
//     //////////////////////////////////////////////////////////////////////////////////
//     // [ ROI_1 ] Array.start
//     //////////////////////////////////////////////////////////////////////////////////
//     if(1)
//     {
//
//         // # Using Read Image Path
//         // CAMERA_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_PARAM"));
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         json_object_object_add(obj_2, "IsEnbReadImageMode", json_object_new_int(1));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/SourceImage.png"));
//         json_object_array_add(array2, root);
//
//
//         /* create json format */
//         //  IBOXANNULUS_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("IBOXRECT_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         obj_3 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "ROIBB_Rect", obj_3);
//         json_object_object_add(obj_3, "Center_X", json_object_new_int(1486));
//         json_object_object_add(obj_3, "Center_Y", json_object_new_int(384));
//         json_object_object_add(obj_3, "Angle", json_object_new_double(24.0));
//         json_object_object_add(obj_3, "Top", json_object_new_int(245));
//         json_object_object_add(obj_3, "Left", json_object_new_int(1330));
//         json_object_object_add(obj_3, "Bottom", json_object_new_int(523));
//         json_object_object_add(obj_3, "Right", json_object_new_int(1643));
//
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/Ret_SourceImage.png"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_rectangle.jpg"));
//
//         json_object_array_add(array2, root);
//
//
//         /* create json format */
//         //  CALCOORDINATE_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         obj_5 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "CalibCoord_G", obj_5);
//         json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
//         json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
//         json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
//         json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
//         json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));
//
//         obj_3 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
//         json_object_object_add(obj_3, "Center_X", json_object_new_int(1));
//         json_object_object_add(obj_3, "Center_Y", json_object_new_int(1));
//         json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
//         json_object_object_add(obj_3, "Top", json_object_new_int(0));
//         json_object_object_add(obj_3, "Left", json_object_new_int(0));
//         json_object_object_add(obj_3, "Bottom", json_object_new_int(2));
//         json_object_object_add(obj_3, "Right", json_object_new_int(2));
//
//         obj_6 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
//         json_object_object_add(obj_6, "Center_X", json_object_new_int(1));
//         json_object_object_add(obj_6, "Center_Y", json_object_new_int(1));
//         json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
//         json_object_object_add(obj_6, "Top", json_object_new_int(0));
//         json_object_object_add(obj_6, "Left", json_object_new_int(0));
//         json_object_object_add(obj_6, "Bottom", json_object_new_int(2));
//         json_object_object_add(obj_6, "Right", json_object_new_int(2));
//
//         obj_4 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
//
//         json_object_object_add(obj_4, "Center_X", json_object_new_int(1486));
//         json_object_object_add(obj_4, "Center_Y", json_object_new_int(384));
//         json_object_object_add(obj_4, "Angle", json_object_new_double(24.0));
//         json_object_object_add(obj_4, "Top", json_object_new_int(245));
//         json_object_object_add(obj_4, "Left", json_object_new_int(1330));
//         json_object_object_add(obj_4, "Bottom", json_object_new_int(523));
//         json_object_object_add(obj_4, "Right", json_object_new_int(1643));
//
//         obj_7 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
//         json_object_object_add(obj_7, "Center_X", json_object_new_int(1486));
//         json_object_object_add(obj_7, "Center_Y", json_object_new_int(384));
//         json_object_object_add(obj_7, "Angle", json_object_new_double(24.0));
//         json_object_object_add(obj_7, "Top", json_object_new_int(245));
//         json_object_object_add(obj_7, "Left", json_object_new_int(1330));
//         json_object_object_add(obj_7, "Bottom", json_object_new_int(523));
//         json_object_object_add(obj_7, "Right", json_object_new_int(1643));
//
//         json_object_array_add(array2, root);
//
//
//
//         /* create json format */
//         // CROPROIIMG_ANNULUS_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_RECT_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_cropimg_annulus.jpg"));
//
//         json_object_array_add(array2, root);
//
//
//
//         /* create json format */
//         //  THRESHOLD_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//         json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(19));
//         json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(200));
//         json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(1));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_threshold.jpg"));
//
//         json_object_array_add(array2, root);
//
//
//         /* create json format */
//         //  MROPHOLOGY_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//         json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(1));
//         json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
//         json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_morphology.jpg"));
//
//         json_object_array_add(array2, root);
//
//
//         /* create json format */
//         //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_RECT_SET_PARAM"));
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
//         json_object_object_add(obj_2, "SearchDirection", json_object_new_int(2)); // 0: Left2Right, 1:Right2Left, 2:Top2Bottom, 3:Bottom2Top
//         json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(0.02));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/noiseremoval.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_gluewidth_measure_rectangle.jpg"));
//
//         json_object_array_add(array2, root);
//
//         json_object_object_add(array_root2, "ROI_1", array2);
//         json_object_array_add(array0, array_root2);
//
//     }
//     //////////////////////////////////////////////////////////////////////////////////
//     // [ ROI_1 ] Array.end
//     //////////////////////////////////////////////////////////////////////////////////
//
//     //////////////////////////////////////////////////////////////////////////////////
//     // [ ROI_2] Array.start
//     //////////////////////////////////////////////////////////////////////////////////
//     if(1)
//     {
//
//         // # Using Read Image Path
//         // CAMERA_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_PARAM"));
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         json_object_object_add(obj_2, "IsEnbReadImageMode", json_object_new_int(1));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/SourceImage.png"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/Ret_SourceImage.png"));
//         json_object_array_add(array3, root);
//
//
//         /* create json format */
//         //  IBOXANNULUS_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("IBOXRECT_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         obj_3 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "ROIBB_Rect", obj_3);
//         json_object_object_add(obj_3, "Center_X", json_object_new_int(495));
//         json_object_object_add(obj_3, "Center_Y", json_object_new_int(960));
//         json_object_object_add(obj_3, "Angle", json_object_new_double(90.0));
//         json_object_object_add(obj_3, "Top", json_object_new_int(780));
//         json_object_object_add(obj_3, "Left", json_object_new_int(440));
//         json_object_object_add(obj_3, "Bottom", json_object_new_int(1140));
//         json_object_object_add(obj_3, "Right", json_object_new_int(550));
//
//
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/Ret_SourceImage.png"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_rectangle.jpg"));
//
//         json_object_array_add(array3, root);
//
//
//         /* create json format */
//         //  CALCOORDINATE_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         obj_5 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "CalibCoord_G", obj_5);
//         json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
//         json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
//         json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
//         json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
//         json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));
//
//         obj_3 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
//         json_object_object_add(obj_3, "Center_X", json_object_new_int(1));
//         json_object_object_add(obj_3, "Center_Y", json_object_new_int(1));
//         json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
//         json_object_object_add(obj_3, "Top", json_object_new_int(0));
//         json_object_object_add(obj_3, "Left", json_object_new_int(0));
//         json_object_object_add(obj_3, "Bottom", json_object_new_int(2));
//         json_object_object_add(obj_3, "Right", json_object_new_int(2));
//
//         obj_6 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
//         json_object_object_add(obj_6, "Center_X", json_object_new_int(1));
//         json_object_object_add(obj_6, "Center_Y", json_object_new_int(1));
//         json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
//         json_object_object_add(obj_6, "Top", json_object_new_int(0));
//         json_object_object_add(obj_6, "Left", json_object_new_int(0));
//         json_object_object_add(obj_6, "Bottom", json_object_new_int(2));
//         json_object_object_add(obj_6, "Right", json_object_new_int(2));
//
//         obj_4 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
//         json_object_object_add(obj_4, "Center_X", json_object_new_int(495));
//         json_object_object_add(obj_4, "Center_Y", json_object_new_int(960));
//         json_object_object_add(obj_4, "Angle", json_object_new_double(90.0));
//         json_object_object_add(obj_4, "Top", json_object_new_int(780));
//         json_object_object_add(obj_4, "Left", json_object_new_int(440));
//         json_object_object_add(obj_4, "Bottom", json_object_new_int(1140));
//         json_object_object_add(obj_4, "Right", json_object_new_int(550));
//
//         obj_7 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
//         json_object_object_add(obj_7, "Center_X", json_object_new_int(495));
//         json_object_object_add(obj_7, "Center_Y", json_object_new_int(960));
//         json_object_object_add(obj_7, "Angle", json_object_new_double(90.0));
//         json_object_object_add(obj_7, "Top", json_object_new_int(780));
//         json_object_object_add(obj_7, "Left", json_object_new_int(440));
//         json_object_object_add(obj_7, "Bottom", json_object_new_int(1140));
//         json_object_object_add(obj_7, "Right", json_object_new_int(550));
//
//         json_object_array_add(array3, root);
//
//
//
//         /* create json format */
//         // CROPROIIMG_ANNULUS_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_RECT_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_cropimg_annulus.jpg"));
//
//         json_object_array_add(array3, root);
//
//
//
//         /* create json format */
//         //  THRESHOLD_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//         json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(19));
//         json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(200));
//         json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(1));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_threshold.jpg"));
//
//         json_object_array_add(array3, root);
//
//
//         /* create json format */
//         //  MROPHOLOGY_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//         json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(1));
//         json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
//         json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_morphology.jpg"));
//
//         json_object_array_add(array3, root);
//
//
//         /* create json format */
//         //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_RECT_SET_PARAM"));
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
//         json_object_object_add(obj_2, "SearchDirection", json_object_new_int(2)); // 0: Left2Right, 1:Right2Left, 2:Top2Bottom, 3:Bottom2Top
//         json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(0.02));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/noiseremoval.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_2/ret_gluewidth_measure_rectangle.jpg"));
//
//         json_object_array_add(array3, root);
//
//         json_object_object_add(array_root3, "ROI_2", array3);
//         json_object_array_add(array0, array_root3);
//
//     }
//     //////////////////////////////////////////////////////////////////////////////////
//     // [ ROI_2] Array.end
//     //////////////////////////////////////////////////////////////////////////////////
//
//     //////////////////////////////////////////////////////////////////////////////////
//     // [ ROI_3] Array.start
//     //////////////////////////////////////////////////////////////////////////////////
//     if (1)
//     {
//
//         // # Using Read Image Path
//         // CAMERA_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_PARAM"));
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         json_object_object_add(obj_2, "IsEnbReadImageMode", json_object_new_int(1));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/SourceImage.png"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/Ret_SourceImage.png"));
//         json_object_array_add(array4, root);
//
//
//         /* create json format */
//         //  IBOXANNULUS_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("IBOXRECT_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         obj_3 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "ROIBB_Rect", obj_3);
//         json_object_object_add(obj_3, "Center_X", json_object_new_int(495));
//         json_object_object_add(obj_3, "Center_Y", json_object_new_int(960));
//         json_object_object_add(obj_3, "Angle", json_object_new_double(90.0));
//         json_object_object_add(obj_3, "Top", json_object_new_int(780));
//         json_object_object_add(obj_3, "Left", json_object_new_int(440));
//         json_object_object_add(obj_3, "Bottom", json_object_new_int(1140));
//         json_object_object_add(obj_3, "Right", json_object_new_int(550));
//
//
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/Ret_SourceImage.png"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_rectangle.jpg"));
//
//         json_object_array_add(array4, root);
//
//
//         /* create json format */
//         //  CALCOORDINATE_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         obj_5 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "CalibCoord_G", obj_5);
//         json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
//         json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
//         json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
//         json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
//         json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));
//
//         obj_3 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
//         json_object_object_add(obj_3, "Center_X", json_object_new_int(1));
//         json_object_object_add(obj_3, "Center_Y", json_object_new_int(1));
//         json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
//         json_object_object_add(obj_3, "Top", json_object_new_int(0));
//         json_object_object_add(obj_3, "Left", json_object_new_int(0));
//         json_object_object_add(obj_3, "Bottom", json_object_new_int(2));
//         json_object_object_add(obj_3, "Right", json_object_new_int(2));
//
//         obj_6 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
//         json_object_object_add(obj_6, "Center_X", json_object_new_int(1));
//         json_object_object_add(obj_6, "Center_Y", json_object_new_int(1));
//         json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
//         json_object_object_add(obj_6, "Top", json_object_new_int(0));
//         json_object_object_add(obj_6, "Left", json_object_new_int(0));
//         json_object_object_add(obj_6, "Bottom", json_object_new_int(2));
//         json_object_object_add(obj_6, "Right", json_object_new_int(2));
//
//         obj_4 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
//         json_object_object_add(obj_4, "Center_X", json_object_new_int(495));
//         json_object_object_add(obj_4, "Center_Y", json_object_new_int(960));
//         json_object_object_add(obj_4, "Angle", json_object_new_double(90.0));
//         json_object_object_add(obj_4, "Top", json_object_new_int(780));
//         json_object_object_add(obj_4, "Left", json_object_new_int(440));
//         json_object_object_add(obj_4, "Bottom", json_object_new_int(1140));
//         json_object_object_add(obj_4, "Right", json_object_new_int(550));
//
//         obj_7 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
//         json_object_object_add(obj_7, "Center_X", json_object_new_int(495));
//         json_object_object_add(obj_7, "Center_Y", json_object_new_int(960));
//         json_object_object_add(obj_7, "Angle", json_object_new_double(90.0));
//         json_object_object_add(obj_7, "Top", json_object_new_int(780));
//         json_object_object_add(obj_7, "Left", json_object_new_int(440));
//         json_object_object_add(obj_7, "Bottom", json_object_new_int(1140));
//         json_object_object_add(obj_7, "Right", json_object_new_int(550));
//
//         json_object_array_add(array4, root);
//
//
//
//         /* create json format */
//         // CROPROIIMG_ANNULUS_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_RECT_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_cropimg_annulus.jpg"));
//
//         json_object_array_add(array4, root);
//
//
//
//         /* create json format */
//         //  THRESHOLD_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//         json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(19));
//         json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(200));
//         json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(1));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_threshold.jpg"));
//
//         json_object_array_add(array4, root);
//
//
//         /* create json format */
//         //  MROPHOLOGY_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//         json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(1));
//         json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
//         json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_morphology.jpg"));
//
//         json_object_array_add(array4, root);
//
//
//         /* create json format */
//         //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_RECT_SET_PARAM"));
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
//         json_object_object_add(obj_2, "SearchDirection", json_object_new_int(2)); // 0: Left2Right, 1:Right2Left, 2:Top2Bottom, 3:Bottom2Top
//         json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(0.02));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_3/noiseremoval.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_3/ret_gluewidth_measure_rectangle.jpg"));
//
//         json_object_array_add(array4, root);
//
//         json_object_object_add(array_root4, "ROI_3", array4);
//         json_object_array_add(array0, array_root4);
//
//     }
//     //////////////////////////////////////////////////////////////////////////////////
//     // [ ROI_3] Array.end
//     //////////////////////////////////////////////////////////////////////////////////
//
//     //////////////////////////////////////////////////////////////////////////////////
//     // [ ROI_4] Array.start
//     //////////////////////////////////////////////////////////////////////////////////
//     if (1)
//     {
//
//         // # Using Read Image Path
//         // CAMERA_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_PARAM"));
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         json_object_object_add(obj_2, "IsEnbReadImageMode", json_object_new_int(1));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/SourceImage.png"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/Ret_SourceImage.png"));
//         json_object_array_add(array5, root);
//
//
//         /* create json format */
//         //  IBOXANNULUS_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("IBOXRECT_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         obj_3 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "ROIBB_Rect", obj_3);
//         json_object_object_add(obj_3, "Center_X", json_object_new_int(495));
//         json_object_object_add(obj_3, "Center_Y", json_object_new_int(960));
//         json_object_object_add(obj_3, "Angle", json_object_new_double(90.0));
//         json_object_object_add(obj_3, "Top", json_object_new_int(780));
//         json_object_object_add(obj_3, "Left", json_object_new_int(440));
//         json_object_object_add(obj_3, "Bottom", json_object_new_int(1140));
//         json_object_object_add(obj_3, "Right", json_object_new_int(550));
//
//
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/Ret_SourceImage.png"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_rectangle.jpg"));
//
//         json_object_array_add(array5, root);
//
//
//         /* create json format */
//         //  CALCOORDINATE_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         obj_5 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "CalibCoord_G", obj_5);
//         json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
//         json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
//         json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
//         json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
//         json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));
//
//         obj_3 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
//         json_object_object_add(obj_3, "Center_X", json_object_new_int(1));
//         json_object_object_add(obj_3, "Center_Y", json_object_new_int(1));
//         json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
//         json_object_object_add(obj_3, "Top", json_object_new_int(0));
//         json_object_object_add(obj_3, "Left", json_object_new_int(0));
//         json_object_object_add(obj_3, "Bottom", json_object_new_int(2));
//         json_object_object_add(obj_3, "Right", json_object_new_int(2));
//
//         obj_6 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
//         json_object_object_add(obj_6, "Center_X", json_object_new_int(1));
//         json_object_object_add(obj_6, "Center_Y", json_object_new_int(1));
//         json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
//         json_object_object_add(obj_6, "Top", json_object_new_int(0));
//         json_object_object_add(obj_6, "Left", json_object_new_int(0));
//         json_object_object_add(obj_6, "Bottom", json_object_new_int(2));
//         json_object_object_add(obj_6, "Right", json_object_new_int(2));
//
//         obj_4 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
//         json_object_object_add(obj_4, "Center_X", json_object_new_int(495));
//         json_object_object_add(obj_4, "Center_Y", json_object_new_int(960));
//         json_object_object_add(obj_4, "Angle", json_object_new_double(90.0));
//         json_object_object_add(obj_4, "Top", json_object_new_int(780));
//         json_object_object_add(obj_4, "Left", json_object_new_int(440));
//         json_object_object_add(obj_4, "Bottom", json_object_new_int(1140));
//         json_object_object_add(obj_4, "Right", json_object_new_int(550));
//
//         obj_7 = (struct json_object*)json_object_new_object();
//         json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
//         json_object_object_add(obj_7, "Center_X", json_object_new_int(495));
//         json_object_object_add(obj_7, "Center_Y", json_object_new_int(960));
//         json_object_object_add(obj_7, "Angle", json_object_new_double(90.0));
//         json_object_object_add(obj_7, "Top", json_object_new_int(780));
//         json_object_object_add(obj_7, "Left", json_object_new_int(440));
//         json_object_object_add(obj_7, "Bottom", json_object_new_int(1140));
//         json_object_object_add(obj_7, "Right", json_object_new_int(550));
//
//         json_object_array_add(array5, root);
//
//
//
//         /* create json format */
//         // CROPROIIMG_ANNULUS_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_RECT_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_cropimg_annulus.jpg"));
//
//         json_object_array_add(array5, root);
//
//
//
//         /* create json format */
//         //  THRESHOLD_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//         json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(19));
//         json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(200));
//         json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(1));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/cropimg_annulus.jpg"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_threshold.jpg"));
//
//         json_object_array_add(array5, root);
//
//
//         /* create json format */
//         //  MROPHOLOGY_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));
//
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//         json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(1));
//         json_object_object_add(obj_2, "iKSize", json_object_new_int(3));
//         json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(2));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/threshold.jpg"));
//         json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/morphology.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_1/ret_morphology.jpg"));
//
//         json_object_array_add(array5, root);
//
//
//         /* create json format */
//         //  GLUEWIDTHMEAS_ANNULUS_SET_PARAM
//         root = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_RECT_SET_PARAM"));
//         obj_2 = (struct json_object*)json_object_new_object();
//         json_object_object_add(root, "args", obj_2);
//
//         json_object_object_add(obj_2, "StepSize", json_object_new_int(10));
//         json_object_object_add(obj_2, "SearchDirection", json_object_new_int(2)); // 0: Left2Right, 1:Right2Left, 2:Top2Bottom, 3:Bottom2Top
//         json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(0.02));
//         json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_4/noiseremoval.jpg"));
//         json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ROI_4/ret_gluewidth_measure_rectangle.jpg"));
//
//         json_object_array_add(array5, root);
//
//         json_object_object_add(array_root5, "ROI_4", array5);
//         json_object_array_add(array0, array_root5);
//
//     }
//     //////////////////////////////////////////////////////////////////////////////////
//     // [ ROI_4] Array.end
//     //////////////////////////////////////////////////////////////////////////////////
//
//
//     /* create json format */
//     //  AUTORUNNING_SET_PARAM
//     root = (struct json_object*)json_object_new_object();
//     json_object_object_add(root, "cmd", json_object_new_string("AUTORUNNING_SET_PARAM"));
//     obj_2 = (struct json_object*)json_object_new_object();
//     json_object_object_add(root, "args", obj_2);
//     json_object_object_add(obj_2, "Enb_AutoRunning", json_object_new_int(0));
//     json_object_object_add(obj_2, "Enb_TriggerMode", json_object_new_int(0));
//
//     json_object_array_add(arrayE, root);
//
//     json_object_object_add(arrayObj_end, "CMD_END", arrayE);
//     json_object_array_add(array0, arrayObj_end);
//
//     //////////////////////////////////////////////////////////////////////////////////
//     //////////////////////////////////////////////////////////////////////////////////
//     //////////////////////////////////////////////////////////////////////////////////
//
//     // json_object_array_add(array0, array_root);
//
//     json_object_object_add(main_root, "ARRAY", array0);
//
//     obj_3 = (struct json_object*)json_object_new_object();
//     json_object_object_add(main_root, "args", obj_3);
//     json_object_object_add(obj_3, "msgId", json_object_new_string("5244_1658801859648"));
//
//     memset(jstring, '\0', sizeof(buf));
//     sprintf(jstring, "%s", (char*)json_object_to_json_string(main_root));
//     BACKENDLOG(0, " json context : %s\n\n", jstring);
//
//     json_object_put(main_root);
//
//     return 0;
// }

int camera_enable_triggermode_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("TRIGGERMODE_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "Enb_TriggerMode_Activate", json_object_new_int(1));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

//////////////////////////////////////////////////////////////////////////
///
///
//////////////////////////////////////////////////////////////////////////

int camera_autorunning_json_create_True(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("AUTORUNNING_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "Enb_AutoRunning", json_object_new_int(1));
    json_object_object_add(obj_2, "Enb_TriggerMode", json_object_new_int(0));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int camera_autorunning_json_create_False(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("AUTORUNNING_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "Enb_AutoRunning", json_object_new_int(0));
    json_object_object_add(obj_2, "Enb_TriggerMode", json_object_new_int(0));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int camera_para_json_initializa(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_INITIALIZE"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "status", json_object_new_string("TBD")); // 0:Mono8, 1:BayerGR8

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);

    return 0;
}

int camera_status_json_inquiry(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_INQUIRY"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "status", json_object_new_string("TBD")); // 0:Mono8, 1:BayerGR8

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int camera_cnfg_json_create(char *jstring, bool bEnb_Decimations4)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_CONFIG"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    // json_object_object_add(obj_2, "PixelFormat", json_object_new_string("BayerGR8"));	 //0:Mono8, 1:BayerGR8
    json_object_object_add(obj_2, "PixelFormat", json_object_new_string("Mono8")); // 0:Mono8, 1:BayerGR8

    json_object_object_add(obj_2, "Width", json_object_new_int(2592));
    json_object_object_add(obj_2, "Height", json_object_new_int(1944));
    json_object_object_add(obj_2, "Offset_X", json_object_new_int(0));
    json_object_object_add(obj_2, "Offset_Y", json_object_new_int(0));

    if (bEnb_Decimations4)
    {
        json_object_object_add(obj_2, "BinningScale", json_object_new_int(2));
    }
    else
    {
        json_object_object_add(obj_2, "BinningScale", json_object_new_int(1));
    }

    json_object_object_add(obj_2, "ExposureMode", json_object_new_string("Auto"));
    // json_object_object_add(obj_2, "ExposureMode", json_object_new_string("Off"));
    json_object_object_add(obj_2, "ExposureTime", json_object_new_double(311514.44));
    // json_object_object_add(obj_2, "ExposureTime", json_object_new_double(311514.44));

    json_object_object_add(obj_2, "PersistentIP", json_object_new_string("192.168.0.105"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int camera_cnfg_json_create_ExposureTime(char *jstring, bool bEnb_Decimations4, bool bflgSet_ExposureTime, double dbVal_ExposureTime)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_CONFIG"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    //json_object_object_add(obj_2, "PixelFormat", json_object_new_string("BayerGR8")); // 0:Mono8, 1:BayerGR8
     json_object_object_add(obj_2, "PixelFormat", json_object_new_string("Mono8"));	 //0:Mono8, 1:BayerGR8

    // json_object_object_add(obj_2, "Width", json_object_new_int(2448));
    // json_object_object_add(obj_2, "Height", json_object_new_int(2048));
    //json_object_object_add(obj_2, "Width", json_object_new_int(2592));
    //json_object_object_add(obj_2, "Height", json_object_new_int(1944));
    json_object_object_add(obj_2, "Width", json_object_new_int(3072));
    json_object_object_add(obj_2, "Height", json_object_new_int(2048));
    json_object_object_add(obj_2, "Offset_X", json_object_new_int(0));
    json_object_object_add(obj_2, "Offset_Y", json_object_new_int(0));

    if (bEnb_Decimations4)
    {
        // json_object_object_add(obj_2, "BinningScale", json_object_new_int(2));
        json_object_object_add(obj_2, "TriggerMode", json_object_new_int(1));
        json_object_object_add(obj_2, "TriggerActivation", json_object_new_int(1));
    }
    else
    {
        // json_object_object_add(obj_2, "BinningScale", json_object_new_int(1));
        json_object_object_add(obj_2, "TriggerMode", json_object_new_int(0));
    }

    if (bflgSet_ExposureTime)
    {
        json_object_object_add(obj_2, "ExposureMode", json_object_new_string("Off"));
        json_object_object_add(obj_2, "ExposureTime", json_object_new_double(dbVal_ExposureTime));
    }
    else
    {
        json_object_object_add(obj_2, "ExposureMode", json_object_new_string("Auto"));
    }

    json_object_object_add(obj_2, "PersistentIP", json_object_new_string("192.168.0.105"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

typedef struct Tag_ImageFormat
{

    unsigned int iOffset_X;
    unsigned int iOffset_Y;
    unsigned int iWidth;
    unsigned int iHeight;

    Tag_ImageFormat()
    {

        iOffset_X = 0;
        iOffset_Y = 0;
        iWidth = 2448;
        iHeight = 2048;
    }

    Tag_ImageFormat(
        unsigned int iOffset_X, unsigned int iOffset_Y, unsigned int iWidth, unsigned int iHeight)
        : iOffset_X(0), iOffset_Y(0), iWidth(2952), iHeight(1944)
    {
    }

} seImageFormat, *LpImageFormat;

int camera_cnfg_json_create_ROI(char *jstring, bool bflgSet_ROI, LpImageFormat pFormat)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_CONFIG"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    // json_object_object_add(obj_2, "PixelFormat", json_object_new_string("BayerGR8"));	 //0:Mono8, 1:BayerGR8
    json_object_object_add(obj_2, "PixelFormat", json_object_new_string("Mono8")); // 0:Mono8, 1:BayerGR8

    json_object_object_add(obj_2, "BinningScale", json_object_new_int(1));
    // json_object_object_add(obj_2, "BinningScale", json_object_new_int(2));

    if (bflgSet_ROI)
    {

        json_object_object_add(obj_2, "Width", json_object_new_int(pFormat->iWidth));
        json_object_object_add(obj_2, "Height", json_object_new_int(pFormat->iHeight));
        json_object_object_add(obj_2, "Offset_X", json_object_new_int(pFormat->iOffset_X));
        json_object_object_add(obj_2, "Offset_Y", json_object_new_int(pFormat->iOffset_Y));
    }
    else
    {

        // json_object_object_add(obj_2, "Width", json_object_new_int(2448));
        // json_object_object_add(obj_2, "Height", json_object_new_int(2048));
        json_object_object_add(obj_2, "Width", json_object_new_int(2592));
        json_object_object_add(obj_2, "Height", json_object_new_int(1944));
        json_object_object_add(obj_2, "Offset_X", json_object_new_int(0));
        json_object_object_add(obj_2, "Offset_Y", json_object_new_int(0));
    }

    json_object_object_add(obj_2, "ExposureMode", json_object_new_string("Auto"));
    // json_object_object_add(obj_2, "ExposureTime", json_object_new_double(85930.0));

    json_object_object_add(obj_2, "PersistentIP", json_object_new_string("192.168.0.105"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int camera_para_json_capture(char *jstring, bool bEnb_Decimations4)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "IsEnbReadImageMode", json_object_new_int(0));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string(""));

    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/Ret_SourceImage.png"));

    // # save image to /tmp/ramdisk/primax/vsb/grap
    // json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/tmp/ramdisk/primax/vsb/grap/source_Img.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);

    return 0;
}

int camera_para_json_release(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_RELEASE"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "status", json_object_new_string("TBD")); // 0:Mono8, 1:BayerGR8

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);

    return 0;
}

// int camera_para_json_streaming(char* jstring, int iIsEnd)
//{
//	struct json_object* root, * obj_2;
//
//	if (jstring == nullptr) return -1;
//	/* create json format */
//	root = (struct json_object*)json_object_new_object();
//	json_object_object_add(root, "cmd", json_object_new_string("CAMERA_SET_STREAMING"));
//
//	obj_2 = (struct json_object*)json_object_new_object();
//	json_object_object_add(root, "args", obj_2);
//
//	json_object_object_add(obj_2, "camera", json_object_new_int(1));
//
//	if (iIsEnd) {
//		json_object_object_add(obj_2, "streaming", json_object_new_string("ON"));
//	}
//	else {
//		json_object_object_add(obj_2, "streaming", json_object_new_string("OFF"));
//	}
//
//
//	memset(jstring, '\0', sizeof(buf));
//	sprintf(jstring, "%s", (char*)json_object_to_json_string(root));
//	BACKENDLOG(0, " json context : %s\n", jstring);
//
//
//	json_object_put(root);
//
//	return 0;
// }

int camera_status_json_streaming_prepare(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_STREAM_SET_PREPARE"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "status", json_object_new_string("TBD")); // 0:Mono8, 1:BayerGR8

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int camera_status_json_streaming_start(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_STREAM_SET_START"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "status", json_object_new_string("TBD")); // 0:Mono8, 1:BayerGR8

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int camera_status_json_streaming_stop(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_STREAM_SET_STOP"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "status", json_object_new_string("TBD")); // 0:Mono8, 1:BayerGR8

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int camera_status_json_streaming_close(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_STREAM_SET_CLOSE"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "status", json_object_new_string("TBD")); // 0:Mono8, 1:BayerGR8

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int image_calibration_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("IMGCALIBRATION_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "E2E_Distance_mm", json_object_new_double(50.35));

    // json_object_object_add(obj_2, "PatternImgType", json_object_new_string("Circle"));

    // obj_3 = (struct json_object*)json_object_new_object();
    // json_object_object_add(obj_2, "ROI_Rect_01", obj_3);
    // json_object_object_add(obj_3, "Top", json_object_new_int(430));
    // json_object_object_add(obj_3, "Left", json_object_new_int(198));
    // json_object_object_add(obj_3, "Bottom", json_object_new_int(467));
    // json_object_object_add(obj_3, "Right", json_object_new_int(234));

    // obj_4 = (struct json_object*)json_object_new_object();
    // json_object_object_add(obj_2, "ROI_Rect_02", obj_4);
    // json_object_object_add(obj_4, "Top", json_object_new_int(430));
    // json_object_object_add(obj_4, "Left", json_object_new_int(352));
    // json_object_object_add(obj_4, "Bottom", json_object_new_int(467));
    // json_object_object_add(obj_4, "Right", json_object_new_int(392));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/PointPattern.png"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_image_calibration.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);

    return 0;
}

int crop_template_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CROPTEMPIMG_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Rect", obj_3);
    json_object_object_add(obj_3, "Top", json_object_new_int(415));
    json_object_object_add(obj_3, "Left", json_object_new_int(670));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(526));
    json_object_object_add(obj_3, "Right", json_object_new_int(726));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));
    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/crop_template.jpg"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_crop_template.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int pattern_match_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("PATTERNMATCH_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_SearchRect", obj_3);

    //json_object_object_add(obj_3, "Top", json_object_new_int(358));
    //json_object_object_add(obj_3, "Left", json_object_new_int(558));
    //json_object_object_add(obj_3, "Bottom", json_object_new_int(583));
    //json_object_object_add(obj_3, "Right", json_object_new_int(778));
    //json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));
    //json_object_object_add(obj_2, "TemplateImgPath", json_object_new_string("/home/user/rextyw/ImageSet/crop_template.jpg"));
    //json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_pattern_match.jpg"));

    json_object_object_add(obj_3, "Top", json_object_new_int(1200));
    json_object_object_add(obj_3, "Left", json_object_new_int(2000));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(1900));
    json_object_object_add(obj_3, "Right", json_object_new_int(2500));
    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/SourceImage.png"));
    json_object_object_add(obj_2, "TemplateImgPath", json_object_new_string("/home/user/rextyw/ImageSet/template_grayImg.png"));
    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_pattern_match.jpg"));


    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);

    return 0;
}

int find_profile_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("FINDPROFILE_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus_Outer", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(364));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(326));
    json_object_object_add(obj_3, "Angle", json_object_new_double(360.0));
    json_object_object_add(obj_3, "Radius", json_object_new_double(146.0));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus_Inner", obj_4);
    json_object_object_add(obj_4, "Center_X", json_object_new_int(364));
    json_object_object_add(obj_4, "Center_Y", json_object_new_int(326));
    json_object_object_add(obj_4, "Angle", json_object_new_double(360.0));
    json_object_object_add(obj_4, "Radius", json_object_new_double(56.0));

    json_object_object_add(obj_2, "SearchDirection", json_object_new_int(1));
    json_object_object_add(obj_2, "EdgePolarity", json_object_new_int(0));
    json_object_object_add(obj_2, "SelLineNo", json_object_new_int(1));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/Shoan_06_Resize.png"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_find_profile.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);

    return 0;
}

int detect_circle_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("FINDCIRCLE_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus_Outer", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(928.0));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(533.0));
    json_object_object_add(obj_3, "Angle", json_object_new_double(360.0));
    json_object_object_add(obj_3, "Radius", json_object_new_double(319.0));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus_Inner", obj_4);
    json_object_object_add(obj_4, "Center_X", json_object_new_int(928.0));
    json_object_object_add(obj_4, "Center_Y", json_object_new_int(533.0));
    json_object_object_add(obj_4, "Angle", json_object_new_double(360.0));
    json_object_object_add(obj_4, "Radius", json_object_new_double(102.0));

    //bool SearchDirection;		//0: Outside to Inside:??$)AuW????????$)AuW??; 1: Inside to Outside:??????$)AuW????$)AuW??
    //bool EdgePolarity;		//0: Rising Edges_??$)AuW????????$)AuW??; 1: Falling Edges_??$)AuW???$)Ahn???$)AuW????
    //int m_iMinEdgeStrength;	//?????????$)Ai*?$)Ah&?	??$)AuW?????�$)AEK?????�$)AEK???$)A8D???????$)AuW??$)Ap{??????$)A#*?????????????$)AuW????

    json_object_object_add(obj_2, "SearchDirection", json_object_new_int(1));
    json_object_object_add(obj_2, "EdgePolarity", json_object_new_int(0));
    json_object_object_add(obj_2, "iMinEdgeStrength", json_object_new_int(100));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/tim_DetectCircle--source.png"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_detect_circle_forTim.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);

    return 0;
}

int ibox_annulus_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("IBOXANNULUS_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(403));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(283));
    json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(54.0));
    json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(172.0));
    json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_annulus.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int ibox_rectangle_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("IBOXRECT_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROIBB_Rect", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(395));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(285));
    json_object_object_add(obj_3, "Angle", json_object_new_double(11.5));
    json_object_object_add(obj_3, "Top", json_object_new_int(156));
    json_object_object_add(obj_3, "Left", json_object_new_int(234));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(414));
    json_object_object_add(obj_3, "Right", json_object_new_int(556));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_rectangle.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int ibox_circle_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("IBOXCIRCLE_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Circle", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(395));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(285));
    json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Radius", json_object_new_double(127.0));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_inspectionbox_circle.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int calcCoord_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5, *obj_6, *obj_7;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CALCOORDINATE_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_C_FMark", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(697));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(468));
    json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Top", json_object_new_int(416));
    json_object_object_add(obj_3, "Left", json_object_new_int(672));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(521));
    json_object_object_add(obj_3, "Right", json_object_new_int(723));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_C_IBox", obj_4);
    json_object_object_add(obj_4, "Center_X", json_object_new_int(396));
    json_object_object_add(obj_4, "Center_Y", json_object_new_int(276));
    json_object_object_add(obj_4, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_4, "Top", json_object_new_int(111));
    json_object_object_add(obj_4, "Left", json_object_new_int(231));
    json_object_object_add(obj_4, "Bottom", json_object_new_int(441));
    json_object_object_add(obj_4, "Right", json_object_new_int(561));

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "CalibCoord_G", obj_5);
    json_object_object_add(obj_5, "Anlgle", json_object_new_double(0.0));
    json_object_object_add(obj_5, "Delta_FMark_W", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_FMark_H", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_IBox_W", json_object_new_int(0));
    json_object_object_add(obj_5, "Delta_IBox_H", json_object_new_int(0));

    obj_6 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_G_FMark", obj_6);
    json_object_object_add(obj_6, "Center_X", json_object_new_int(0));
    json_object_object_add(obj_6, "Center_Y", json_object_new_int(0));
    json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_6, "Top", json_object_new_int(0));
    json_object_object_add(obj_6, "Left", json_object_new_int(0));
    json_object_object_add(obj_6, "Bottom", json_object_new_int(0));
    json_object_object_add(obj_6, "Right", json_object_new_int(0));

    obj_7 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_G_IBox", obj_7);
    json_object_object_add(obj_7, "Center_X", json_object_new_int(0));
    json_object_object_add(obj_7, "Center_Y", json_object_new_int(0));
    json_object_object_add(obj_7, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_7, "Top", json_object_new_int(0));
    json_object_object_add(obj_7, "Left", json_object_new_int(0));
    json_object_object_add(obj_7, "Bottom", json_object_new_int(0));
    json_object_object_add(obj_7, "Right", json_object_new_int(0));

    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int cropimg_annulus_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5, *obj_6, *obj_7;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_ANNULUS_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "CalibCoord", obj_3);
    json_object_object_add(obj_3, "Anlgle", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Delta_FMark_W", json_object_new_int(0));
    json_object_object_add(obj_3, "Delta_FMark_H", json_object_new_int(0));
    json_object_object_add(obj_3, "Delta_IBox_W", json_object_new_int(0));
    json_object_object_add(obj_3, "Delta_IBox_H", json_object_new_int(0));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_FMark", obj_4);
    json_object_object_add(obj_4, "Center_X", json_object_new_int(697));
    json_object_object_add(obj_4, "Center_Y", json_object_new_int(468));
    json_object_object_add(obj_4, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_4, "Top", json_object_new_int(416));
    json_object_object_add(obj_4, "Left", json_object_new_int(672));
    json_object_object_add(obj_4, "Bottom", json_object_new_int(521));
    json_object_object_add(obj_4, "Right", json_object_new_int(723));

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_IBox", obj_5);
    json_object_object_add(obj_5, "Center_X", json_object_new_int(396));
    json_object_object_add(obj_5, "Center_Y", json_object_new_int(276));
    json_object_object_add(obj_5, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_5, "Top", json_object_new_int(111));
    json_object_object_add(obj_5, "Left", json_object_new_int(231));
    json_object_object_add(obj_5, "Bottom", json_object_new_int(441));
    json_object_object_add(obj_5, "Right", json_object_new_int(561));

    obj_6 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus", obj_6);
    json_object_object_add(obj_6, "Center_X", json_object_new_int(403));
    json_object_object_add(obj_6, "Center_Y", json_object_new_int(283));
    json_object_object_add(obj_6, "Radius_Inner", json_object_new_double(54.0));
    json_object_object_add(obj_6, "Radius_Outer", json_object_new_double(172.0));
    json_object_object_add(obj_6, "Angle_Start", json_object_new_double(0.0));
    json_object_object_add(obj_6, "Angle_End", json_object_new_double(360.0));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));

    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/cropimg_annulus.jpg"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_cropimg_annulus.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int cropimg_rectangle_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5, *obj_6, *obj_7;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_RECT_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "CalibCoord", obj_3);
    json_object_object_add(obj_3, "Anlgle", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Delta_FMark_W", json_object_new_int(0));
    json_object_object_add(obj_3, "Delta_FMark_H", json_object_new_int(0));
    json_object_object_add(obj_3, "Delta_IBox_W", json_object_new_int(0));
    json_object_object_add(obj_3, "Delta_IBox_H", json_object_new_int(0));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_FMark", obj_4);
    json_object_object_add(obj_4, "Center_X", json_object_new_int(697));
    json_object_object_add(obj_4, "Center_Y", json_object_new_int(468));
    json_object_object_add(obj_4, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_4, "Top", json_object_new_int(416));
    json_object_object_add(obj_4, "Left", json_object_new_int(672));
    json_object_object_add(obj_4, "Bottom", json_object_new_int(521));
    json_object_object_add(obj_4, "Right", json_object_new_int(723));

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_IBox", obj_5);
    json_object_object_add(obj_5, "Center_X", json_object_new_int(396));
    json_object_object_add(obj_5, "Center_Y", json_object_new_int(291));
    json_object_object_add(obj_5, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_5, "Top", json_object_new_int(125));
    json_object_object_add(obj_5, "Left", json_object_new_int(220));
    json_object_object_add(obj_5, "Bottom", json_object_new_int(458));
    json_object_object_add(obj_5, "Right", json_object_new_int(573));

    obj_6 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROIBB_Rect", obj_6);
    json_object_object_add(obj_6, "Center_X", json_object_new_int(369));
    json_object_object_add(obj_6, "Center_Y", json_object_new_int(291));
    json_object_object_add(obj_6, "Angle", json_object_new_double(11.5));
    json_object_object_add(obj_6, "Top", json_object_new_int(150));
    json_object_object_add(obj_6, "Left", json_object_new_int(242));
    json_object_object_add(obj_6, "Bottom", json_object_new_int(432));
    json_object_object_add(obj_6, "Right", json_object_new_int(549));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));

    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/cropimg_rectangle.jpg"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_cropimg_rectangle.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int cropimg_circle_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3, *obj_4, *obj_5, *obj_6, *obj_7;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CROPROIIMG_CIRCLE_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "CalibCoord", obj_3);
    json_object_object_add(obj_3, "Anlgle", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Delta_FMark_W", json_object_new_int(0));
    json_object_object_add(obj_3, "Delta_FMark_H", json_object_new_int(0));
    json_object_object_add(obj_3, "Delta_IBox_W", json_object_new_int(0));
    json_object_object_add(obj_3, "Delta_IBox_H", json_object_new_int(0));

    obj_4 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_FMark", obj_4);
    json_object_object_add(obj_4, "Center_X", json_object_new_int(697));
    json_object_object_add(obj_4, "Center_Y", json_object_new_int(468));
    json_object_object_add(obj_4, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_4, "Top", json_object_new_int(416));
    json_object_object_add(obj_4, "Left", json_object_new_int(672));
    json_object_object_add(obj_4, "Bottom", json_object_new_int(521));
    json_object_object_add(obj_4, "Right", json_object_new_int(723));

    obj_5 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "BoundingBox_IBox", obj_5);
    json_object_object_add(obj_5, "Center_X", json_object_new_int(400));
    json_object_object_add(obj_5, "Center_Y", json_object_new_int(284));
    json_object_object_add(obj_5, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_5, "Top", json_object_new_int(149));
    json_object_object_add(obj_5, "Left", json_object_new_int(265));
    json_object_object_add(obj_5, "Bottom", json_object_new_int(419));
    json_object_object_add(obj_5, "Right", json_object_new_int(535));

    obj_6 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Circle", obj_6);
    json_object_object_add(obj_6, "Center_X", json_object_new_int(400));
    json_object_object_add(obj_6, "Center_Y", json_object_new_int(284));
    json_object_object_add(obj_6, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_6, "Radius", json_object_new_int(135));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));

    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/cropimg_circle.jpg"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_cropimg_circle.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int histogram_annulus_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("HISTOGRAM_ANNULUS_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(403));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(283));
    json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(54.0));
    json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(172.0));
    json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_histogram_annulus.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int histogram_rectangle_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("HISTOGRAM_RECT_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROIBB_Rect", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(395));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(285));
    json_object_object_add(obj_3, "Angle", json_object_new_double(11.5));
    json_object_object_add(obj_3, "Top", json_object_new_int(156));
    json_object_object_add(obj_3, "Left", json_object_new_int(234));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(414));
    json_object_object_add(obj_3, "Right", json_object_new_int(556));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_histogram_rectangle.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int histogram_circle_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("HISTOGRAM_CIRCLE_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Circle", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(395));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(285));
    json_object_object_add(obj_3, "Angle", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Radius", json_object_new_double(127.0));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_histogram_circle.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int threshold_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("THRESHOLD_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "pThresh_Min", json_object_new_int(35));
    json_object_object_add(obj_2, "pThresh_Max", json_object_new_int(200));
    json_object_object_add(obj_2, "emThresholdTypes", json_object_new_int(0));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/cropimg_annulus.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/ResizeSourceImage.png"));

    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/threshold.jpg"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_threshold.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int moophology_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("MROPHOLOGY_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "emMorphShapes", json_object_new_int(0));
    json_object_object_add(obj_2, "iKSize", json_object_new_int(5));
    json_object_object_add(obj_2, "emMorphOperation", json_object_new_int(3));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/threshold.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgThd.png"));

    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/morphology.jpg"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_morphology.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int noiseremoval_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("NOISEREMOVAL_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "dbLimit_min", json_object_new_double(10.0));
    json_object_object_add(obj_2, "dbLimit_max", json_object_new_double(15000.0));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/morphology.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgThd.png"));

    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/noiseremoval.jpg"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_noiseremoval.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int dataaugmentation_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("DATAAUGMENTATION_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    json_object_object_add(obj_2, "bEnb_Flip_Xasix", json_object_new_int(1));
    json_object_object_add(obj_2, "bEnb_Flip_Yasix", json_object_new_int(1));
    json_object_object_add(obj_2, "bEnb_Flip_XYasix", json_object_new_int(1));

    json_object_object_add(obj_2, "dbRotateAngle", json_object_new_double(15.0));

    json_object_object_add(obj_2, "iVal_Brightness_R", json_object_new_int(100));
    json_object_object_add(obj_2, "iVal_Brightness_G", json_object_new_int(128));
    json_object_object_add(obj_2, "iVal_Brightness_B", json_object_new_int(156));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/SourceImage.png"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/dataaugmentation.jpg"));

    json_object_object_add(obj_2, "SaveImgPath", json_object_new_string("/home/user/primax/vsb/grap/dataaugmentation"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_dataaugmentation.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int gluewidth_measure_annulus_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_ANNULUS_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROI_Annulus", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(178));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(141));
    json_object_object_add(obj_3, "Radius_Inner", json_object_new_double(59.0));
    json_object_object_add(obj_3, "Radius_Outer", json_object_new_double(118.0));
    json_object_object_add(obj_3, "Angle_Start", json_object_new_double(0.0));
    json_object_object_add(obj_3, "Angle_End", json_object_new_double(360.0));

    json_object_object_add(obj_2, "StepSize", json_object_new_int(10));

    json_object_object_add(obj_2, "mm_per_pixel", json_object_new_double(10.5));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/primax/vsb/grap/noiseremoval.jpg"));
    // json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgMorphology_Resize.png"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_gluewidth_measure_annulus.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

int gluewidth_measure_rectangle_json_create(char *jstring)
{
    struct json_object *root, *obj_2, *obj_3;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("GLUEWIDTHMEAS_RECT_SET_PARAM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    obj_3 = (struct json_object *)json_object_new_object();
    json_object_object_add(obj_2, "ROIBB_Rect", obj_3);
    json_object_object_add(obj_3, "Center_X", json_object_new_int(121));
    json_object_object_add(obj_3, "Center_Y", json_object_new_int(122));
    json_object_object_add(obj_3, "Angle", json_object_new_double(45.0));
    json_object_object_add(obj_3, "Top", json_object_new_int(58));
    json_object_object_add(obj_3, "Left", json_object_new_int(50));
    json_object_object_add(obj_3, "Bottom", json_object_new_int(187));
    json_object_object_add(obj_3, "Right", json_object_new_int(192));

    json_object_object_add(obj_2, "StepSize", json_object_new_int(30));

    json_object_object_add(obj_2, "InputImgPath", json_object_new_string("/home/user/rextyw/ImageSet/imgMorphology_Resize.png"));

    json_object_object_add(obj_2, "ResultImgPath", json_object_new_string("/home/user/primax/vsb/grap/ret_gluewidth_measure_rectangle.jpg"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, "%s", (char *)json_object_to_json_string(root));
    BACKENDLOG(0, " json context : %s\n", jstring);

    json_object_put(root);
    return 0;
}

///<=== IPS <==== IPS //////////////////////////////////////////////////////////

int ios_light_set_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("LIGHT_SET_PWM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "inLight", json_object_new_int(1));
    json_object_object_add(obj_2, "value", json_object_new_int(100));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "light json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_light_get_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("LIGHT_GET_PWM"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "inLight", json_object_new_int(1));
    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "response", obj_2);

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "light json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_trigger_set_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("TRIGGER_SET_PROCESS"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "inPin", json_object_new_int(1));
    json_object_object_add(obj_2, "inMode", json_object_new_string("PNP"));
    json_object_object_add(obj_2, "outPin", json_object_new_int(4));
    json_object_object_add(obj_2, "outDelay", json_object_new_int64(1000));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "trigger json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_trigger_get_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("TRIGGER_GET_PROCESS"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "inPin", json_object_new_int(1));
    json_object_object_add(obj_2, "outPin", json_object_new_int(4));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "trigger json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_di_set_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("DIN_SET_PROCESS"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "inPin", json_object_new_int(1));
    json_object_object_add(obj_2, "inMode", json_object_new_string("PNP"));
    json_object_object_add(obj_2, "inControlMode", json_object_new_int(0));
    json_object_object_add(obj_2, "inDelay", json_object_new_int64(2000));
    json_object_object_add(obj_2, "outPin", json_object_new_int(2));
    json_object_object_add(obj_2, "outMode", json_object_new_string("PNP"));
    json_object_object_add(obj_2, "outControlMode", json_object_new_int(0));
    json_object_object_add(obj_2, "outDelay", json_object_new_int64(2000));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "trigger json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_di_get_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("DIN_GET_PROCESS"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "inPin", json_object_new_int(1));
    json_object_object_add(obj_2, "outPin", json_object_new_int(2));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "trigger json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_dio_get_status_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("DIO_GET_STATUS"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "trigger json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_get_status_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("IOS_GET_STATUS"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "trigger json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_led_set_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("LED_SET_PROCESS"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "outPin", json_object_new_int(4));
    json_object_object_add(obj_2, "outMode", json_object_new_string("AI status"));
    json_object_object_add(obj_2, "outStatus", json_object_new_string("Red ON"));
    json_object_object_add(obj_2, "outBlinkDelay", json_object_new_int64(1000));
    json_object_object_add(obj_2, "outOffDelay", json_object_new_int64(0));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "trigger json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_led_get_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("LED_GET_PROCESS"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "outPin", json_object_new_int(1));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "trigger json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

/************ for web backend ****************/
/* setting user definition LED */
int ios_led_set_mode_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("LED_SET_MODE"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "LED", json_object_new_int(2)); // LED 1 or LED 2
    json_object_object_add(obj_2, "LedMode", json_object_new_string("AI status"));
    json_object_object_add(obj_2, "Indication", json_object_new_string("ON"));
    json_object_object_add(obj_2, "Color", json_object_new_string("Red"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "LED_SET_MODE json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_led_get_mode_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("LED_GET_MODE"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "LED_SET_MODE json : %s\n", jstring);
    json_object_put(root);
    return 0;
}
/* setting Din */
int ios_din_set_mode_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("DIN_SET_MODE"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "DinPin", json_object_new_int(1)); // 1 ~ 4
    json_object_object_add(obj_2, "DinPolarity", json_object_new_string("PNP"));
    json_object_object_add(obj_2, "SelectMode", json_object_new_string("Glue inspection"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "DIN_SET_MODE json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

/* for backend DOUT_MANUAL_CONTROL */
int ios_dout_manual_control_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("DOUT_MANUAL_CONTROL"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "DoutPin", json_object_new_int(4));
    json_object_object_add(obj_2, "DoutPolarity", json_object_new_string("PNP"));
    json_object_object_add(obj_2, "onoffSetting", json_object_new_int(1));
    json_object_object_add(obj_2, "SelectMode", json_object_new_string("Latch"));
    json_object_object_add(obj_2, "OneShotPeriod", json_object_new_int(0));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "DOUT_MANUAL_CONTROL json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

/* for backend DOUT_SET_MODE */
int ios_dout_set_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("DOUT_SET_MODE"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "DoutPin", json_object_new_int(1));
    json_object_object_add(obj_2, "DoutPolarity", json_object_new_string("PNP"));
    json_object_object_add(obj_2, "SelectMode", json_object_new_string("AutoDone"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "DOUT_SET_MODE json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_light_set_mode_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("LIGHT_SET_BRIGHTNESS"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "lightSource", json_object_new_int(1));
    json_object_object_add(obj_2, "Brightness", json_object_new_int(100));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "light json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_light_get_mode_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("LIGHT_GET_BRIGHTNESS"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "light json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int main_camera_streaming_On_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_STREAMING_CONTROL"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "camera", json_object_new_int(1));
    json_object_object_add(obj_2, "streaming", json_object_new_string("ON"));
    // json_object_object_add(obj_2, "streaming", json_object_new_string("OFF"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "light json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int main_camera_streaming_Off_json_create(char *jstring)
{
    struct json_object *root, *obj_2;

    if (jstring == nullptr)
        return -1;
    /* create json format */
    root = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "cmd", json_object_new_string("CAMERA_STREAMING_CONTROL"));

    obj_2 = (struct json_object *)json_object_new_object();
    json_object_object_add(root, "args", obj_2);
    json_object_object_add(obj_2, "camera", json_object_new_int(1));
    // json_object_object_add(obj_2, "streaming", json_object_new_string("ON"));
    json_object_object_add(obj_2, "streaming", json_object_new_string("OFF"));

    memset(jstring, '\0', sizeof(buf));
    sprintf(jstring, (char *)json_object_to_json_string(root));
    BACKENDLOG(0, "light json : %s\n", jstring);
    json_object_put(root);
    return 0;
}

int ios_readjsonfile1(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd1.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd1.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile1_1(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd1_1.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd1_1.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile2(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd2.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd2.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile3(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd3.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd3.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile4(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd4.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd4.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile5(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd5.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd5.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile6(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd6.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd6.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile7(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd7.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd7.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile8(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd8.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd8.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile9(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd9.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd9.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile10(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd10.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd10.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile11(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd11.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd11.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile12(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd12.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd12.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile13(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd13.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd13.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile14(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd14.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd14.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile15(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd15.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd15.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

int ios_readjsonfile16(char *jstring)
{
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen("/home/user/primax/etc/param/json/backend_cmd16.json", "r");
    if (file == NULL)
    {
        printf("can't open /home/user/primax/etc/param/json/backend_cmd16.json\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    buffer = (char *)malloc(file_size * sizeof(char));
    if (buffer == NULL)
    {
        printf("unable to allocate memory\n");
        return 1;
    }

    fread(buffer, sizeof(char), file_size, file);

    printf("buffer=[%s]", buffer);
    snprintf(jstring, file_size, "%s", &buffer[0]);
    printf("[%s]\n", jstring);

    free(buffer);

    fclose(file);
}

/******************************************************
 * non-block program to get the keyborad input
 * to check the key buffer, once large than 0
 * meaning the keyboard is pressed
 *******************************************************/
int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void *backend_mqtt_sub(void *argu)
{
    /* for backend */
    backend_mqtt_subscriber();
}



int main(int argc, char **argv)
{
    int ret;
    char a[10];
    seImageFormat seROI;

    //////////////////////////////////
    // gpio control
    /////////////////////////////////

#if (End_GpioCtrl)

    struct pollfd fdset[2];
    int nfds = 2;
    int gpio_fd, timeout, rc;
    unsigned int gpio;
    int len, index = 1;

    // Real gpio control function
    gpio = 0;

    gpio_export(gpio);
    gpio_set_dir(gpio, 0);
    gpio_set_edge(gpio, "rising");
    gpio_fd = gpio_fd_open(gpio);

    timeout = POLL_TIMEOUT;

    memset((void *)fdset, 0, sizeof(fdset));
    fdset[0].fd = STDIN_FILENO;
    fdset[0].events = POLLIN;
    fdset[1].fd = gpio_fd;
    fdset[1].events = POLLPRI;

#endif

    /////////////////////////////////

    BACKENDLOG(0, "*********************************\n");
    BACKENDLOG(0, "*****   Backend Handler App.   *****\n");
    BACKENDLOG(0, "*********************************\n");

    /* Signal handling */
    signal(SIGKILL, sigExit_main);
    signal(SIGTERM, sigExit_main);
    signal(SIGSEGV, sigExit_main);
    signal(SIGINT, sigExit_main);

    /* init value */

    /* create a thread for exteranl MQTT subscriber */
    ret = pthread_create(&thread1, nullptr, backend_mqtt_sub, nullptr);
    if (ret < 0)
    {
        perror("Cannot create thread 1 !!\n");
        exit(1);
    }

    usleep(50000);
    
    char ch;
    bool iEnbLoopRunninig = 0;
    ios_readjsonfile1(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 1\n", __FUNCTION__, __LINE__);
    ch = getchar();

    ios_readjsonfile1_1(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 1-1\n", __FUNCTION__, __LINE__);
    ch = getchar();
    
    ios_readjsonfile2(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 2\n", __FUNCTION__, __LINE__);
    ch = getchar();
    
    ios_readjsonfile3(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 3\n", __FUNCTION__, __LINE__);
    ch = getchar();
    
    ios_readjsonfile4(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 4\n", __FUNCTION__, __LINE__);
    ch = getchar();
    
    ios_readjsonfile5(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 5\n", __FUNCTION__, __LINE__);
    ch = getchar();
    
    ios_readjsonfile6(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 6\n", __FUNCTION__, __LINE__);
    ch = getchar();
    
    ios_readjsonfile7(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 7\n", __FUNCTION__, __LINE__);
    ch = getchar();
    
    ios_readjsonfile8(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 8\n", __FUNCTION__, __LINE__);
    ch = getchar();
    
    ios_readjsonfile9(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 9\n", __FUNCTION__, __LINE__);
    ch = getchar();
    
    ios_readjsonfile10(buf); 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 10\n", __FUNCTION__, __LINE__);
    ch = getchar();
    
    ios_readjsonfile11(buf); 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 11\n", __FUNCTION__, __LINE__);
    
    ios_readjsonfile12(buf); 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 12\n", __FUNCTION__, __LINE__);
    
    ios_readjsonfile13(buf); 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 13\n", __FUNCTION__, __LINE__);
    
    ios_readjsonfile14(buf); 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 14\n", __FUNCTION__, __LINE__);

    ios_readjsonfile15(buf); 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(100000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(100000);
    fprintf(stderr, "%s()%d: 15\n", __FUNCTION__, __LINE__);

        
    exit(0);
    
    while (1)
    {

        // if (iEnbLoopRunninig) {

        //	sleep(2);

        //	////////////////////////////////////////////////
        //	//// Streaming testing === === >>>
        //	////////////////////////////////////////////////

        //	camera_para_json_streaming(buf, 0);
        //	
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

        //	sleep(5);

        //	////////////////////////////////////////////////
        //	//// Strwaming testing <<< === ===
        //	////////////////////////////////////////////////

        //	////////////////////////////////////////////////
        //	//// GigE camera testing === === >>>
        //	////////////////////////////////////////////////

        //	camera_para_json_initializa(buf);
        //	
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

        //	sleep(2);

        //	camera_status_json_inquiry(buf);
        //	
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

        //	sleep(2);

        //	camera_cnfg_json_create(buf, 0);
        //	
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

        //	camera_para_json_capture(buf, 0);
        //	
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

        //	sleep(2);

        //	camera_para_json_release(buf);
        //	
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

        //	sleep(5);

        //	////////////////////////////////////////////////
        //	//// GigE camera testing <<< === ===
        //	////////////////////////////////////////////////

        //	////////////////////////////////////////////////
        //	//// Streaming testing === === >>>
        //	////////////////////////////////////////////////

        //	camera_para_json_streaming(buf, 1);
        //	
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

        //	sleep(20);

        //	////////////////////////////////////////////////
        //	//// Strwaming testing <<< === ===
        //	////////////////////////////////////////////////

        //}

        if (kbhit())
        {
            fflush(stdin);
            fgets(a, 10, stdin);
            fflush(stdin);
            BACKENDLOG(2, "backend receive command string = %s\n", a);
            switch (a[0])
            {
            case '0':
                ios_readjsonfile1(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                //ios_readjsonfile1_1(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                ios_readjsonfile2(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                ios_readjsonfile3(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                ios_readjsonfile4(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                ios_readjsonfile5(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                ios_readjsonfile6(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                //ios_readjsonfile7(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                ios_readjsonfile8(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                //ios_readjsonfile9(buf);  
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                ios_readjsonfile10(buf); 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                ios_readjsonfile11(buf);
                break;
            case '1':
                ios_readjsonfile12(buf); //
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                break;
            case '2':
                ios_readjsonfile13(buf); //
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                break;
            case '3':
                ios_readjsonfile11(buf); //
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);    usleep(500000);
                break;
            case 'y': // Loop running
                if (!iEnbLoopRunninig)
                {
                    iEnbLoopRunninig = 1;
                }
                else
                {
                    iEnbLoopRunninig = 0;
                }
                break;

            case 'a':
                ios_dio_get_status_json_create(buf);
                break;
            case 'b':
                ios_din_set_mode_json_create(buf);
                break;
            // case 'c':
            //   ios_led_set_mode_json_create(buf);
            // break;
            case 'd':
                ios_dout_manual_control_json_create(buf);
                break;
            case 'f':
                ios_light_get_mode_json_create(buf);
                break;

            case 'i':
                camera_status_json_inquiry(buf);
                break;
            case 'l':
                camera_status_json_streaming_prepare(buf);
                break;
            case 'g':
                camera_status_json_streaming_start(buf);
                break;
            case 't':
                camera_status_json_streaming_stop(buf);
                break;
            case 'c':
                camera_status_json_streaming_close(buf);
                break;

            case 's':
                camera_cnfg_json_create_ExposureTime(buf, 0, 0, 99999.0);
                break;

            case 'e':
                camera_cnfg_json_create_ExposureTime(buf, 1, 1, 30000.0);
                break;

            case 'h':
                 //camera_autorunning_json_create_TESTING(buf);  //Annulus
                camera_autorunning_RECT_create_TESTING(buf); // Rectanle
                break;

            case 'v':
                camera_enable_triggermode_json_create(buf);
                break;

            case 'p':
                //camera_para_json_capture(buf, 0);

                 pattern_match_json_create(buf);
                 //
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                 //detect_circle_json_create(buf);

                break;

            //case 'p':

                //////////////////////////////////////////////
                // [Large MQTT package] Auto Running_Continue testing  ====== >
                //////////////////////////////////////////////

                // camera_autorunning_json_create_TESTING(buf);
                // camera_autorunning_json_create_TESTING_3(buf);

                /*
                                
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                                camera_enable_triggermode_json_create(buf);
                /*				
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                                camera_enable_triggermode_json_create(buf);
                                
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                                camera_enable_triggermode_json_create(buf);

                                //*/

                //////////////////////////////////////////////
                // [Large MQTT package] Auto Running_Continue testing < ======
                //////////////////////////////////////////////

                //////////////////////////////////////////////
                // Auto Running_Continue testing ====== >
                //////////////////////////////////////////////
                /*
                camera_autorunning_json_create_True(buf);
                
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                //camera_para_json_create(buf);
                //
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                pattern_match_json_create(buf);
                
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);


                ibox_annulus_json_create(buf);
                
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                calcCoord_json_create(buf);
                
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                cropimg_annulus_json_create(buf);
                
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                threshold_json_create(buf);
                
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                moophology_json_create(buf);
                
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                noiseremoval_json_create(buf);
                
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                gluewidth_measure_annulus_json_create(buf);
                
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                camera_autorunning_json_create_False(buf);
                //*/

                //////////////////////////////////////////////
                // Auto Running_Continue testing < ======
                //////////////////////////////////////////////

                //////////////////////////////////////////////
                // Step by Step testing ====== >
                //////////////////////////////////////////////

                //////////////////////////////////////////////
                // Streaming testing === === >>>
                //////////////////////////////////////////////

                // camera_para_json_streaming(buf, 0);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // sleep(5);

                //////////////////////////////////////////////
                // Strwaming testing <<< === ===
                //////////////////////////////////////////////

                // camera_para_json_initializa(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                //camera_status_json_inquiry(buf);
                //
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                //camera_cnfg_json_create(buf, 0);
                //
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                //camera_para_json_capture(buf, 0);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // sleep(1);

                // camera_para_json_release(buf);
                ////
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // sleep(5);

                //////////////////////////////////////////////
                // Streaming testing === === >>>
                //////////////////////////////////////////////

                // camera_para_json_streaming(buf, 1);

                // sleep(8);

                //////////////////////////////////////////////
                // Strwaming testing <<< === ===
                //////////////////////////////////////////////

                // image_calibration_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // crop_template_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // pattern_match_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // find_profile_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // detect_circle_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // ibox_annulus_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // ibox_rectangle_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // ibox_circle_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // calcCoord_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // cropimg_annulus_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // cropimg_rectangle_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // cropimg_circle_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // histogram_annulus_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // histogram_rectangle_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // histogram_circle_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // threshold_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // moophology_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // noiseremoval_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // dataaugmentation_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // gluewidth_measure_annulus_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // gluewidth_measure_rectangle_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // ais_elic_initialize_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                // ais_elic_inference_json_create(buf);
                // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                //////////////////////////////////////////////
                // Step by Step testing < ======
                //////////////////////////////////////////////

                //break;

            // case 'l':
            //	ios_light_set_json_create(buf);
            //	break;
            case 'm':
                ios_light_get_json_create(buf);
                break;
            case 'r':
                ios_trigger_set_json_create(buf);
                break;
            // case 's':
            //	ios_trigger_get_json_create(buf);
            //	break;
            // case 't':
            //	ios_di_set_json_create(buf);
            //	break;
            case 'u':
                ios_di_get_json_create(buf);
                break;
            // case 'v':
            //     ios_led_set_json_create(buf);
            //     break;
            case 'w':
                ios_led_get_json_create(buf);
                break;
            case 'x':
                ios_get_status_json_create(buf);
                break;
            case 'z':
                /* kill all processing ??memory ??$)Aq#O */
                exit(1);
                break;
                // memset(ch,0,500);
            }

            // 
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);
            backend_mqtt_publisher_Dual(buf, 0);
            backend_mqtt_publisher_Dual(buf, 1);
        }

        //////////////////////////////////
        // gpio control
        /////////////////////////////////

#if (End_GpioCtrl)

        // memset((void*)fdset, 0, sizeof(fdset));
        // fdset[0].fd = STDIN_FILENO;
        // fdset[0].events = POLLIN;
        // fdset[1].fd = gpio_fd;
        // fdset[1].events = POLLPRI;

        rc = poll(fdset, nfds, timeout);
        printf("rc=%d\n", rc);

        if (rc == 0)
        {
            printf(".");
        }
        if (fdset[1].revents & POLLPRI)
        {

            len = read(fdset[1].fd, buf, MAX_BUF);
            printf("\npoll() GPIO %d interrupt occurred\n", gpio);

            if (index == 0)
            {

                printf("gpio set to high\n");

                camera_enable_triggermode_json_create(buf);
                
    backend_mqtt_publisher_Dual(buf, 0);    usleep(500000);
    backend_mqtt_publisher_Dual(buf, 1);

                index = 1;
            }
            else
            {

                printf("gpio set to low\n");

                index = 0;
            }
        }
        if (fdset[0].revents & POLLIN)
        {
            (void)read(fdset[0].fd, buf, 1);
            printf("\npoll() stdin read 0x%2.2X\n", (unsigned int)buf[0]);
        }
        fflush(stdout);

#endif

        /////////////////////////////////

        sleep(1);
    };

#if (End_GpioCtrl)

    gpio_fd_close(gpio_fd);

#endif

    return 0;
}
