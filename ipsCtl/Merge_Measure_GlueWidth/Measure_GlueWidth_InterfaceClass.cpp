#include "Measure_GlueWidth_InterfaceClass.h"  //Interface class
#include "Measure_GlueWidth_ConcreteClass.h"


// Create Object 


#if defined(_MSC_VER)	// /* Visual C++ */


    DLLCALL PTWDLL_I_GlueWidth* _cdecl CreateObject_Class() 
    {
        return new PTWDLL_C_GlueWidth();
    }


#elif defined (__GNUC__)		//__GNUC__

    PTWDLL_I_GlueWidth* CreateObject_Class()
    {
        return new PTWDLL_C_GlueWidth();
    }

#endif 



