#include "GigECamLib_InterfaceClass.h"  //Interface class
#include "GigECamLib_ConcreteClass.h"


GigECam_I_Library* CreateObject_GigECam()
{
    return new GigECam_C_Library();
}