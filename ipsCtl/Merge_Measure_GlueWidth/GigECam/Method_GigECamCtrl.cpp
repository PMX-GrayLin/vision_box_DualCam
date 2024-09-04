#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "../cvip.h"
#include "Method_GigECamCtrl.h"



using namespace std;

static OverwriteRingBuffer<cv::Mat> g_owRingbuffer(6);

//////////////////////////////////////////////////////////////////////////////
///  Point Grey Gige camera controller function
/////////////////////////////////////////////////////////////////////////////

CMethod_GigECamCtrl::CMethod_GigECamCtrl()
    : m_system(nullptr)
    , m_pCam(nullptr)
    , m_iFrameRate_Cnt(0)
    , m_RetryCnt(10)
    , m_Timeout(0)
{
    m_camList = CameraList();

}


CMethod_GigECamCtrl::~CMethod_GigECamCtrl()
{

    GigECam_Release();

}


int CMethod_GigECamCtrl::GigeCam_Init()
{

    int result = 0;

    // Print Application Build Information
    cout << endl;
    cout << "GigE Camera information : " << endl;

    cout << "Application build date: " << __DATE__ << " " << __TIME__ << endl << endl;

    //@ 1. Retrieve singleton reference to system object
    m_system = System::GetInstance();

    // Print out current library version
    const LibraryVersion spinnakerLibraryVersion = m_system->GetLibraryVersion();
    cout << "Spinnaker library version: " 
        << spinnakerLibraryVersion.major << "." 
        << spinnakerLibraryVersion.minor << "." 
        << spinnakerLibraryVersion.type << "." 
        << spinnakerLibraryVersion.build << endl << endl;

    //@ 2. Retrieve list of cameras from the system
    m_camList = m_system->GetCameras();

    unsigned int m_numCameras = m_camList.GetSize();
    cout << "Number of cameras detected: " << m_numCameras << endl << endl;

    //@ 3. Finish if there are no cameras
    if (m_numCameras == 0)
    {
        // Release camera list before releasing system
        m_camList.Clear();

        // Release system0
        m_system->ReleaseInstance();
    
        return -1;
    }

    //@ 4. Run example on each camera
    for (unsigned int i = 0; i < m_numCameras; i++)
    {
        // Select camera
        m_pCam = m_camList.GetByIndex(i);
    }
    //m_pCam = m_camList.GetByIndex(m_numCameras);

    // Print device info
    if (PrintDeviceInfo(m_pCam)) {

        return -1;
    }

    // Initialize camera
    m_pCam->Init();

    // Set acquisition mode to continuous
    if (!IsReadable(m_pCam->AcquisitionMode) || !IsWritable(m_pCam->AcquisitionMode))
    {
        cout << " > ! Unable to set acquisition mode to continuous. Aborting..." << endl << endl;
    }
    else {

        m_pCam->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
        cout << "Acquisition mode set to continuous..." << endl;
    }

    //Doump Image Format.
    if (Configure_Get(m_pCam, &m_global_cfg_ParamInfo, 1)) {

        printf("Configure_Get( m_pCam, \" m_global_cfg_ParamInfo \", 1 )\n");
    }


	return result;
}


int CMethod_GigECamCtrl::GigECam_SetConfig(const LpGigECamConfig pParamIn)
{

    int nRet = 0;

    if (m_pCam == nullptr) {
        cout << " > Erroe!!! m_pCam <= 0 " << endl;
        return -1;
    }

    try {

        cout << " >> m_global_cfg_ParamInfo.bIsStreaming : " << m_global_cfg_ParamInfo.bIsStreaming << endl;

        if (!m_global_cfg_ParamInfo.bIsStreaming) {

            if (!nRet) nRet = Configure_Decimation(m_pCam, pParamIn);
            if (nRet) {
                cout << " > Erroe!!! Configure_Decimation(m_pCam, pParamIn) " << endl;
                return nRet;
            }

            //Update camera config to global parameter.
            if (!nRet) nRet = Configure_Get(m_pCam, &m_global_cfg_ParamInfo);
            if (nRet) {
                cout << " > Erroe!!! Configure_Get(m_pCam, pParamIn) " << endl;
                return nRet;
            }

            // Image format information
            if (!nRet) nRet = Configure_ImageFormat(m_pCam, pParamIn);
            if (nRet) {
                cout << " > Erroe!!! Configure_ImageFormat(m_pCam, pParamIn) " << endl;
                return nRet;
            }

            if (!nRet) nRet = Configure_FrameRate(m_pCam, pParamIn);
            if (nRet) {
                cout << " > Erroe!!! Configure_FrameRate(m_pCam, pParamIn) " << endl;
                return nRet;
            }

        }
        else {

            std::cout << "This is streaming mode; settings cannot be configured..\n";
            std::cout << "This is streaming mode; settings cannot be configured..\n";
        }


        // Configure exposure
        if (!nRet) nRet = Configure_Exposure(m_pCam, pParamIn);
        if (nRet) {
            cout << " > Erroe!!! Configure_Exposure(m_pCam, pParamIn) " << endl;
            return nRet;
        }

/*
        if (!nRet) nRet = Configure_FrameRate(m_pCam, pParamIn);
        if (nRet) {
            cout << " > Erroe!!! Configure_FrameRate(m_pCam, pParamIn) " << endl;
            return nRet;
        }   
*/     

        if (nRet)
        {
            if (m_global_cfg_ParamInfo.bIsStreaming) {

                CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

                AcquireStreaming_StartorStop(false);
                usleep(50000);
            }

            return nRet;
        }


        //// Get the UserSetSave node
        //CEnumerationPtr ptrUserSetSave = m_pCam->GetNodeMap().GetNode("UserSetSave");
        //if (IsAvailable(ptrUserSetSave) && IsWritable(ptrUserSetSave))
        //{
        //    // Set the value of UserSetSave
        //    ptrUserSetSave->SetIntValue(ptrUserSetSave->GetEntryByName("On")->GetValue());
        //}
        //else
        //{
        //    cout << "Unable to set UserSetSave." << endl;
        //}

        //////INodeMap& nodeMap = m_pCam->GetNodeMap();
        //////CCommandPtr ptrUserSetSave = nodeMap.GetNode("UserSetSave");
        //////if (!ptrUserSetSave.IsValid())
        //////{
        //////    cout << "Unable to save Settings to User Set 1. Aborting..." << endl << endl;
        //////}
        //////else {

        //////    ptrUserSetSave->Execute();
        //////}


    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;

        if (m_global_cfg_ParamInfo.bIsStreaming) {

            CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

            AcquireStreaming_StartorStop(false);
            usleep(50000);
        }

        return -1;
    }

    return nRet;
}



int CMethod_GigECamCtrl::GigECam_GetConfig(LpGigECamConfig pParamOut)
{
    int nRet = 0;

    if (m_pCam == nullptr) {
        cout << " > Erroe!!! m_pCam <= 0 " << endl;
        return -1;
    }

    if (m_global_cfg_ParamInfo.bIsStreaming) {

        CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

        AcquireStreaming_StartorStop(false);
        usleep(50000);
    }

    // Configure information
    nRet = Configure_Get(m_pCam, pParamOut);

    if (nRet < 0)
    {
        return nRet;
    }

    return nRet;
}



int CMethod_GigECamCtrl::GigECam_AcquireImages(string strFilePath)
{

    int result = 0;
    int err = 0;

    if (m_pCam == nullptr) {
        cout << " > Erroe!!! m_pCam <= 0 " << endl;
        return -1;
    }

    if (m_global_cfg_ParamInfo.bIsStreaming) {

        CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

        AcquireStreaming_StartorStop(false);
        usleep(50000);
    }

    try
    {
        // AcquireImages
        result = result | AcquireImages(m_pCam, strFilePath);

    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }
        

    return result;
}



int CMethod_GigECamCtrl::GigECam_AcquireImages(cv::Mat& matImg)
{

    int result = 0;
    int err = 0;

    if (m_pCam == nullptr) {
        cout << " > Erroe!!! m_pCam <= 0 " << endl;
        return -1;
    }

    if (m_global_cfg_ParamInfo.bIsStreaming) {

        CAMI("Inof ~~, Streaming mode is already running. Stop now.\n");

        AcquireStreaming_StartorStop(false);
        usleep(50000);
    }

    try
    {
        // AcquireImages
        result = result | AcquireImages(m_pCam, matImg);

    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}



int CMethod_GigECamCtrl::GigECam_Release()
{

    cout << " > # GigECam_Release() " << endl;

    int result = 0;

    if (m_pCam) {

        // Deinitialize camera
        m_pCam->DeInit();

        m_pCam = nullptr;

        // Release camera list before releasing system
        m_camList.Clear();

        // Release system
        m_system->ReleaseInstance();
    }

    return result;
}


int CMethod_GigECamCtrl::GigECam_Strm_Prepare()
{
    int ret = 0;

    AcquireStreaming_Prepare();

    return ret;
}


int CMethod_GigECamCtrl::GigECam_Strm_Start()
{
    int ret = 0;

    AcquireStreaming_StartorStop(true);
    usleep(50000);

    return ret;
}


int CMethod_GigECamCtrl::GigECam_Strm_Stop()
{
    int ret = 0;

    AcquireStreaming_StartorStop(false);
    usleep(50000);

    return ret;
}


int CMethod_GigECamCtrl::GigECam_Strm_Close()
{
    int ret = 0;

    AcquireStreaming_Close();

    return ret;
}



// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo example for more in-depth comments on printing
// device information from the nodemap.
int CMethod_GigECamCtrl::PrintDeviceInfo(CameraPtr pCam)
{
    int result = 0;

    if (pCam == nullptr) {
        return -1;
    }

    CameraPtr camera = pCam;

    cout << endl << "*** DEVICE INFORMATION ***" << endl << endl;
    try
    {
        INodeMap& nodeMap = camera->GetTLDeviceNodeMap();
        FeatureList_t features;
        CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
        if (IsAvailable(category) && IsReadable(category))
        {
            category->GetFeatures(features);
            FeatureList_t::const_iterator it;
            for (it = features.begin(); it != features.end(); ++it)
            {
                CNodePtr pfeatureNode = *it;
                cout << pfeatureNode->GetName() << " : ";
                CValuePtr pValue = (CValuePtr)pfeatureNode;
                cout << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
                cout << endl;
            }
        }
        else
        {
            cout << "Device control information not available." << endl;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    cout << endl;

    return result;

}


int CMethod_GigECamCtrl::Configure_ImageFormat(CameraPtr pCam, const LpGigECamConfig pParamIn)
{
    int result = 0;

    if (pCam == nullptr) {

        return -1;
    }

    cout << endl << endl << "*** CONFIGURING CUSTOM IMAGE SETTINGS ***" << endl << endl;
    //try
    //{

        int iRange_Max_Offset_X = 0, iRange_Max_Offset_Y = 0;
        int iRange_Max_Width = 0, iRange_Max_Height = 0;

        int iLimit_Max_Width = 0, iLimit_Max_Height = 0;

        iLimit_Max_Width = m_global_cfg_ParamInfo.iSensor_Width;
        iLimit_Max_Height = m_global_cfg_ParamInfo.iSensor_Height;


        if (iLimit_Max_Width == 0 || iLimit_Max_Height == 0) {

            return -1;
        }


        int iOffsetX = 0, iOffsetY = 0;
        int iWidth = 0, iHeight = 0;
        int iDiff_X = 0, iDiff_Y = 0;


        //�e���]�m�ɻݦҼ{�B�i(4)�A�Y�]�m�e����4������
        //����ƾڳ]�w�A���� width -> offset_x ->height -> offset_y


        /// # ////////////////////////////////////////////
        // config : iOffset_X, iWidth --. -->
        iDiff_X = abs((int)(pParamIn->iWidth + pParamIn->iOffset_X));
        printf(" Configure parameters is iOffset_X = %d, iWidth = %d\n", pParamIn->iOffset_X, pParamIn->iWidth);

        if (iDiff_X > iLimit_Max_Width) {

            printf(" Out Of Range(Max Width = %d) : iOffset_X + iWidth = %d\n", iLimit_Max_Width, iDiff_X);

            iWidth = iLimit_Max_Width;
            iOffsetX = 0;
        }
        else {

            iWidth = calcMultiplesofFour(pParamIn->iWidth);
            iOffsetX = calcMultiplesofFour(pParamIn->iOffset_X);

        }

        iRange_Max_Width = iWidth;
        iRange_Max_Offset_X = iOffsetX; // (iLimit_Max_Width - iWidth);
        iDiff_X = abs((int)(iRange_Max_Width + iRange_Max_Offset_X));
        printf(" parameters is Range_iOffset_X = %d, Range_iWidth = %d\n", iRange_Max_Offset_X, iRange_Max_Width);

        if (iDiff_X > iLimit_Max_Width) {

            printf(" Out Of Range(Max Width = %d) : Range_iOffset_X + Range_iWidth = %d\n", iLimit_Max_Width, iDiff_X);

            iWidth = iLimit_Max_Width;
            iOffsetX = 0;
        }

        // Apply minimum to offset X
        //if (IsReadable(pCam->OffsetX) && IsWritable(pCam->OffsetX))
        if (IsAvailable(pCam->OffsetX) && IsWritable(pCam->OffsetX))
        {
            pCam->OffsetX.SetValue(0);

            usleep(500); /* delay 5  ms */
        }
        else
        {
            cout << "Offset X not available..." << endl;
            result = -1;
        }

        // Set width
        //if (IsReadable(pCam->Width) && IsWritable(pCam->Width) && pCam->Width.GetInc() != 0 && pCam->Width.GetMax() != 0)
        if (IsAvailable(pCam->Width) && IsWritable(pCam->Width) && pCam->Width.GetInc() != 0 && pCam->Width.GetMax() != 0)
        {
            pCam->Width.SetValue(iLimit_Max_Width);

            printf(" Configure parameters : Range_iWidth = %d\n", iRange_Max_Width);
            pCam->Width.SetValue(iRange_Max_Width);

            usleep(500); /* delay 5  ms */
        }
        else
        {
            cout << "Width not available..." << endl;
            result = -1;
        }

        //Set offset X
        //if (IsReadable(pCam->OffsetX) && IsWritable(pCam->OffsetX))
        if (IsAvailable(pCam->OffsetX) && IsWritable(pCam->OffsetX))
        {
            printf(" Configure parameters : Range_iOffsetX = %d\n", iRange_Max_Offset_X);

            pCam->OffsetX.SetValue(iRange_Max_Offset_X);

            usleep(500); /* delay 5  ms */
        }
        else
        {
            cout << "Offset X not available..." << endl;
            result = -1;
        }



        /// # ////////////////////////////////////////////
        // 2. > config : OffsetY, Height --. -->
        iDiff_Y = abs((int)(pParamIn->iHeight + pParamIn->iOffset_Y));
        printf(" Configure parameters is iOffset_Y = %d, iHeight = %d\n", pParamIn->iOffset_Y, pParamIn->iHeight);

        if (iDiff_Y > iLimit_Max_Height) {

            printf(" Out Of Range(Max Height = %d) : iOffset_Y + iHeight = %d\n", iLimit_Max_Height, iDiff_Y);

            iHeight = iLimit_Max_Height;
            iOffsetY = 0;
        }
        else {

            iHeight = calcMultiplesofTwo(pParamIn->iHeight);
            iOffsetY = calcMultiplesofTwo(pParamIn->iOffset_Y);
        }

        iRange_Max_Height = iHeight;
        iRange_Max_Offset_Y = iOffsetY; // (iLimit_Max_Height - iHeight);
        iDiff_Y = abs((int)(iRange_Max_Width + iRange_Max_Offset_X));
        printf(" parameters is Range_iOffset_Y = %d, Range_iHeight = %d\n", iRange_Max_Offset_Y, iRange_Max_Height);

        if (iDiff_X > iLimit_Max_Width) {

            printf(" Out Of Range(Max Width = %d) : Range_iOffset_X + Range_iWidth = %d\n", iLimit_Max_Width, iDiff_X);

            iWidth = iLimit_Max_Width;
            iOffsetX = 0;
        }

        // Apply minimum to offset Y
        //if (IsReadable(pCam->OffsetY) && IsWritable(pCam->OffsetY))
        if (IsAvailable(pCam->OffsetY) && IsWritable(pCam->OffsetY))
        {
            pCam->OffsetY.SetValue(0);

            usleep(500); /* delay 5  ms */
        }
        else
        {
            cout << "Offset Y not available..." << endl;
            result = -1;
        }


        // Set height
        //if (IsReadable(pCam->Height) && IsWritable(pCam->Height) && pCam->Height.GetInc() != 0 && pCam->Height.GetMax() != 0)
        if (IsAvailable(pCam->Height) && IsWritable(pCam->Height) && pCam->Height.GetInc() != 0 && pCam->Height.GetMax() != 0)
        {

            pCam->Height.SetValue(iLimit_Max_Height);

            printf(" Configure parameters : Range_iHeight = %d\n", iRange_Max_Height);

            pCam->Height.SetValue(iRange_Max_Height);

            usleep(500); /* delay 5  ms */
        }
        else
        {
            cout << "Height not available..." << endl;
            result = -1;
        }


        // Set offset Y
        //if (IsReadable(pCam->OffsetY) && IsWritable(pCam->OffsetY))
        if (IsAvailable(pCam->OffsetY) && IsWritable(pCam->OffsetY))
        {
            printf(" Configure parameters : Range_iOffsetY = %d\n", iRange_Max_Offset_Y);

            pCam->OffsetY.SetValue(iOffsetY);

            usleep(500); /* delay 5  ms */
        }
        else
        {
            cout << "Offset Y not available..." << endl;
            result = -1;
        }



        /// # ////////////////////////////////////////////
        // 3. > config : PixelFormat  --. -->
        //if (IsReadable(pCam->PixelFormat) && IsWritable(pCam->PixelFormat))
        if (IsAvailable(pCam->PixelFormat) && IsWritable(pCam->PixelFormat)) 
        {

            printf(" pixel_format = %d\n", pParamIn->bPixelFormat);

            if (pParamIn->bPixelFormat == 0) {  //0:PixelFormat_Mono8, 1:PixelFormat_RGB8Packed
                pCam->PixelFormat.SetValue(PixelFormat_Mono8);
            }
            else {
                pCam->PixelFormat.SetValue(PixelFormat_RGB8Packed);
            }

            usleep(500); /* delay 5  ms */

        }
        else
        {
            cout << "Unable to set PixalFormat, Pixel format not available..." << endl;
            result = -1;
        }
         
    //}
    //catch (Spinnaker::Exception& e)
    //{
    //    cout << "Error: " << e.what() << endl;
    //    result = -1;
    //}

    return result;
}



int CMethod_GigECamCtrl::Configure_Decimation(CameraPtr pCam, const LpGigECamConfig pParamIn)
{
    int nRet = 0;

    if (pCam == nullptr) {

        return -1;
    }

    /* Connect to the first available camera */
    CameraPtr camera = pCam;

    if (camera) {
        
        int iScale = pParamIn->iBinning_Scale;

        if (iScale > 2) {
            iScale = 2;
        }
        else if (iScale < 0) {
            iScale = 1;
        }

        if (!nRet) {

            printf(" > Configure parameters : DecimationVertical = %d\n", iScale);

            // Set decimation
            CIntegerPtr decimation_V = camera->GetNodeMap().GetNode("DecimationVertical");

            if (IsAvailable(decimation_V) && IsWritable(decimation_V)) {

                decimation_V->SetValue(iScale);
            }
            else if (IsAvailable(decimation_V) || IsReadable(decimation_V)) {
                cout << " > decimation_V->GetValue() : " << decimation_V->GetValue() << endl;
            }
            else {
                cout << "Error!! Not support the Decimation Mode !!!" << endl;
            }

            printf(" > Done~~~\n");

            usleep(500); /* delay 5  ms */
        }

        // --> the Node of "DecimationHorizontal" is read only <---
        //if (!nRet) {

        //    printf(" Configure parameters : DecimationHorizontal = %d\n", iScale);

        //    // Set decimation
        //    CIntegerPtr decimation_H = camera->GetNodeMap().GetNode("DecimationHorizontal");
        //    decimation_H->SetValue(iScale);

        //    printf("Done~~~\n");

        //    usleep(500); /* delay 5  ms */
        //}
        // --> the Node of "DecimationHorizontal" is read only <---


        if (nRet) {

            /* En error happened, display the correspdonding message */
            printf("Error: [%x]\n", nRet);
            return -1;
        }

    }
    else {
        /* En error happened, display the correspdonding message */
        printf("error: OpenDevice fail [%x]\n", nRet);
        return -1;
    }

    return nRet;
}



// This function configures a custom exposure time. Automatic exposure is turned
// off in order to allow for the customization, and then the custom setting is
// applied.
int CMethod_GigECamCtrl::Configure_Exposure(CameraPtr pCam, const LpGigECamConfig pParamIn)
{
    int result = 0;

    if (pCam == nullptr) {

        return -1;
    }


    cout << endl << endl << "*** CONFIGURING EXPOSURE ***" << endl << endl;
    try
    {

        double exposureTimeToSet = seGigECamConfig().dbExposureTime;
        int iEnum_ExposureMode = 0; // <---ExposureAutoEnums::ExposureAuto_Continuous;

        if (pParamIn->bExposureAuto == static_cast<int>(emExposureAuto::Timed_Auto)) {
            
            iEnum_ExposureMode = 0;//<-- ExposureAutoEnums::ExposureAuto_Continuous;
            exposureTimeToSet = 0.0;
        }
        else {
        
            iEnum_ExposureMode = 1; //<-- ExposureAutoEnums::ExposureAuto_Off;
            exposureTimeToSet = pParamIn->dbExposureTime;
        }


        if (!IsAvailable(pCam->ExposureAuto) || !IsWritable(pCam->ExposureAuto))
        {
            cout << " > ! Unable to disable automatic exposure. Aborting..." << endl << endl;
            return -1;
        }

        if (0 == iEnum_ExposureMode) {


            cout << "pCam->ExposureAuto " << pCam->ExposureAuto.GetValue() << "..." << endl;

            int mode = pCam->ExposureAuto.GetValue();

            if (mode != ExposureAuto_Continuous) {

                pCam->ExposureAuto.SetValue(ExposureAuto_Off);
                pCam->ExposureAuto.SetValue(ExposureAuto_Continuous);
            }  
        }
        else {

            pCam->ExposureAuto.SetValue(ExposureAuto_Off);
            cout << "Automatic exposure disabled..." << endl;

            if (!IsAvailable(pCam->ExposureTime) || !IsWritable(pCam->ExposureTime))
            {
                cout << " > ! Unable to set exposure time. Aborting..." << endl << endl;
                return -1;
            }
            // Ensure desired exposure time does not exceed the maximum
            const double exposureTimeMax = pCam->ExposureTime.GetMax();
            if (exposureTimeToSet > exposureTimeMax)
            {
                exposureTimeToSet = exposureTimeMax;
            }
            pCam->ExposureTime.SetValue(exposureTimeToSet);
            cout << std::fixed << "Shutter time set to " << exposureTimeToSet << " us..." << endl << endl;
        }

        // Get the value of exposure time to set an appropriate timeout for GetNextImage
        if (!IsAvailable(pCam->ExposureTime) || !IsReadable(pCam->ExposureTime))
        {
            cout << " > ! Unable to read exposure time. Aborting..." << endl << endl;
            return -1;
        }

        // The exposure time is retrieved in �gs so it needs to be converted to ms to keep consistency with the unit 
        // being used in GetNextImage
        m_Timeout = static_cast<uint64_t>( (pCam->ExposureTime.GetValue() / 1000) + 1000);
        cout << " timeout value :" << std::to_string(m_Timeout) << endl;

    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }
    return result;
}


bool CMethod_GigECamCtrl::Configure_EnableManualFramerate(CameraPtr pCam)
{
    INodeMap& NodeMap = pCam->GetNodeMap();
    
    // Turning AcquisitionFrameRateEnable on
    CBooleanPtr ptrFrameRateEnable = NodeMap.GetNode("AcquisitionFrameRateEnable");
    if (ptrFrameRateEnable == nullptr)
    {
        // AcquisitionFrameRateEnabled is used for Gen2 devices
        ptrFrameRateEnable = NodeMap.GetNode("AcquisitionFrameRateEnabled");
    }
    if (IsAvailable(ptrFrameRateEnable) && IsWritable(ptrFrameRateEnable))
    {
        ptrFrameRateEnable->SetValue(true);
        //cout << " > @ AcquisitionFrameRateEnable set to True" << endl;
    }

    // Turning AcquisitionFrameRateAuto off
    CEnumerationPtr ptrFrameRateAuto = NodeMap.GetNode("AcquisitionFrameRateAuto");
    if (!IsAvailable(ptrFrameRateAuto) || !IsWritable(ptrFrameRateAuto))
    {
        cout << " > ! Unable to set AcquisitionFrameRateAuto..." << endl << endl;
        return false;
    }
    CEnumEntryPtr ptrFrameRateAutoModeOff = ptrFrameRateAuto->GetEntryByName("Off");
    if (!IsAvailable(ptrFrameRateAutoModeOff) || !IsReadable(ptrFrameRateAutoModeOff))
    {
        cout << " > ! Unable to set AcquisitionFrameRateAuto to OFF. Aborting..." << endl << endl;
        return false;
    }
    // Retrieve integer value from entry node
    const int64_t valueFrameRateAutoOff = ptrFrameRateAutoModeOff->GetValue();
    // Set integer value from entry node as new value of enumeration node
    ptrFrameRateAuto->SetIntValue(valueFrameRateAutoOff);
    //cout << " > @ AcquisitionFrameRateAuto set to OFF" << endl;

    return true;
}


int CMethod_GigECamCtrl::Configure_FrameRate(CameraPtr pCam, const LpGigECamConfig pParamIn)
{
    int nRet = 0;

    if (pCam == nullptr) {

        return -1;
    }

    //try
    //{
        Configure_EnableManualFramerate(pCam);

        cout << " > Setting maximum framerate" << endl;
        INodeMap& nodeMap = pCam->GetNodeMap();
        // Set AcquisitionFrameRate to maximum
        CFloatPtr AcquisitionFrameRateNode = nodeMap.GetNode("AcquisitionFrameRate");
        if (!IsAvailable(AcquisitionFrameRateNode) || !IsWritable(AcquisitionFrameRateNode))
        {
            cout << " > ! Unable to set AcquisitionFrameRate to Max. Aborting..." << endl << endl;
            //return false;
        }
        cout << " > @ AcquisitionFrameRateNode->GetMax() :" << AcquisitionFrameRateNode->GetMax() << endl;
        cout << " > @ pParamIn->dbAcquisitionFrameRate :" << pParamIn->dbAcquisitionFrameRate << endl;

        //double dbFrameRate_Val = min(pParamIn->dbAcquisitionFrameRate, AcquisitionFrameRateNode->GetMax());
        double dbFrameRate_Val = max(pParamIn->dbAcquisitionFrameRate, AcquisitionFrameRateNode->GetMax());
        AcquisitionFrameRateNode->SetValue(dbFrameRate_Val);

    //}
    //catch (Spinnaker::Exception& e)
    //{
    //    cout << "Exception setting FrameRate to maximum: " << e.what() << endl;
    //}

    return nRet;
}


// This function resets the cameras default User Set to the Default User Set.
// Note that User Set 1 will retain the settings set during ConfigureUserSet1.
void CMethod_GigECamCtrl::Configure_ResetCameraUserSetToDefault(CameraPtr pCam)
{

    // Get the camera node map
    INodeMap& nodeMap = pCam->GetNodeMap();
    // Get User Set Default from User Set Selector
    CEnumerationPtr ptrUserSetSelector = nodeMap.GetNode("UserSetSelector");
    if (!IsWritable(ptrUserSetSelector))
    {
        cout << "Unable to set User Set Selector to Default (node retrieval). Aborting..." << endl << endl;
        return;
    }
    CEnumEntryPtr ptrUserSetDefaultEntry = ptrUserSetSelector->GetEntryByName("Default");
    if (!IsReadable(ptrUserSetDefaultEntry))
    {
        cout << "Unable to set User Set Selector to Default (enum entry retrieval). Aborting..." << endl << endl;
        return;
    }
    const int64_t userSetDefault = ptrUserSetDefaultEntry->GetValue();
    // Set User Set Selector back to User Set Default
    ptrUserSetSelector->SetIntValue(userSetDefault);
    // Set User Set Default to User Set Default
    CEnumerationPtr ptrUserSetDefault = nodeMap.GetNode("UserSetDefault");
    if (!IsWritable(ptrUserSetDefault))
    {
        cout << "Unable to set User Set Default to User Set 1 (node retrieval). Aborting..." << endl << endl;
        return;
    }
    ptrUserSetDefault->SetIntValue(userSetDefault);
    // Execute User Set Load to load User Set Default
    CCommandPtr ptrUserSetLoad = nodeMap.GetNode("UserSetLoad");
    if (!ptrUserSetLoad.IsValid())
    {
        cout << "Unable to load Settings from User Set Default. Aborting..." << endl << endl;
        return;
    }
    ptrUserSetLoad->Execute();

}


int CMethod_GigECamCtrl::Configure_Get(CameraPtr pCam, LpGigECamConfig pParamOut, bool bDumpInf)
{

    if (nullptr == pParamOut)
        return -1;

    int nRet = 0;

    /* Connect to the first available camera */
    CameraPtr camera = pCam;

    LpGigECamConfig pParam = pParamOut;

    if(camera)
    {
        int cfg_Max_Width(0), cfg_Max_Height(0);

        int cfg_OffsetX(0), cfg_OffsetY(0);
        int cfg_Width(0), cfg_Height(0);

        int cfg_PixelFormat(0);
        int cfg_Exposure_modeto(0);

        int cfg_Decimation_ScaleVal(0);

        bool cfg_bIsEnbAcquisitionFrameRate(0);
        double cfg_dbAcquisitionFrameRate(0.0);




        CIntegerPtr ptrWidthMax = camera->GetNodeMap().GetNode("WidthMax");
        cfg_Max_Width = ptrWidthMax->GetValue();
        usleep(500);
        CIntegerPtr ptrHeightMax = camera->GetNodeMap().GetNode("HeightMax");
        cfg_Max_Height = ptrHeightMax->GetValue();
        usleep(500);

        CIntegerPtr ptrOffsetX = camera->GetNodeMap().GetNode("OffsetX");
        cfg_OffsetX = ptrOffsetX->GetValue();
        usleep(500);

        CIntegerPtr ptrOffsetY = camera->GetNodeMap().GetNode("OffsetY");
        cfg_OffsetY = ptrOffsetY->GetValue();
        usleep(500);

        CIntegerPtr ptrWidth = camera->GetNodeMap().GetNode("Width");
        cfg_Width = ptrWidth->GetValue();
        usleep(500);

        CIntegerPtr ptrHeight = camera->GetNodeMap().GetNode("Height");
        cfg_Height = ptrHeight->GetValue();
        usleep(500);

        CEnumerationPtr ptrPixelFormat = camera->GetNodeMap().GetNode("PixelFormat");
        CEnumEntryPtr pixelFormatValue = ptrPixelFormat->GetCurrentEntry();
        //cout << " > pixelFormat->GetCurrentEntry() = " << pixelFormat->GetCurrentEntry() << endl;
        cfg_PixelFormat = pixelFormatValue->GetValue();
        //cout << " > pixelFormatValue->GetValue() = " << pixelFormatValue->GetValue() << endl;
        usleep(500);

        CEnumerationPtr ptrEexposureAuto = camera->GetNodeMap().GetNode("ExposureAuto");
        CEnumEntryPtr exposureAutoValue = ptrEexposureAuto->GetCurrentEntry();
        //cout << " > exposureAuto->GetCurrentEntry() = " << exposureAuto->GetCurrentEntry() << endl;
        cfg_Exposure_modeto = exposureAutoValue->GetValue();
        //cout << " > exposureAutoValue->GetValue() = " << exposureAutoValue->GetValue() << endl;
        usleep(500);

        CIntegerPtr ptrDecimation_V = camera->GetNodeMap().GetNode("DecimationVertical");
        cfg_Decimation_ScaleVal = ptrDecimation_V->GetValue();
        //cout << " > decimation_V->GetValue() = " << decimation_V->GetValue() << endl;
        usleep(500);


        INodeMap& NodeMap = camera->GetNodeMap();
        // Turning AcquisitionFrameRateEnable on
        CBooleanPtr ptrFrameRateEnable = NodeMap.GetNode("AcquisitionFrameRateEnable");
        if (ptrFrameRateEnable == nullptr)
        {
            // AcquisitionFrameRateEnabled is used for Gen2 devices
            ptrFrameRateEnable = NodeMap.GetNode("AcquisitionFrameRateEnabled");
        }
        if (IsAvailable(ptrFrameRateEnable) && IsWritable(ptrFrameRateEnable))
        {
            cfg_bIsEnbAcquisitionFrameRate = ptrFrameRateEnable->GetValue();
            //cout << " > @ AcquisitionFrameRateEnable set to True" << endl;
        }
        usleep(500);

        if (cfg_bIsEnbAcquisitionFrameRate) {
            // Set AcquisitionFrameRate to maximum
            CFloatPtr AcquisitionFrameRateNode = NodeMap.GetNode("AcquisitionFrameRate");
            if (!IsAvailable(AcquisitionFrameRateNode) || !IsWritable(AcquisitionFrameRateNode))
            {
                cout << " > ! Unable to set AcquisitionFrameRate to Max. Aborting..." << endl << endl;
            }
            //cout << " > @ AcquisitionFrameRateNode->GetMax() :" << AcquisitionFrameRateNode->GetMax() << endl;
            cfg_dbAcquisitionFrameRate = AcquisitionFrameRateNode->GetValue();
            usleep(500);
        }


        pParam->bIsConnected = true;

        pParam->iSensor_Width = cfg_Max_Width;
        pParam->iSensor_Height = cfg_Max_Height;

        pParam->iOffset_X = cfg_OffsetX;
        pParam->iOffset_Y = cfg_OffsetY;

        pParam->iWidth = cfg_Width;
        pParam->iHeight = cfg_Height;

        pParam->bPixelFormat = (cfg_PixelFormat == 0x01080001) ? 0 : 1;  //0:PixelFormat_Mono8, 1:PixelFormat_RGB8Packed
        pParam->bExposureAuto = (cfg_Exposure_modeto == 2) ? 1 : 0;     //0:Auto, 1:Timed;

        pParam->iBinning_Scale = cfg_Decimation_ScaleVal;

        pParam->bIsEnbAcquisitionFrameRate = cfg_bIsEnbAcquisitionFrameRate;
        pParam->dbAcquisitionFrameRate = cfg_dbAcquisitionFrameRate;


        if (bDumpInf) {

            printf("\n\n>>> Device information === >>> === >>>\n");
            printf("\t	@ Cfg.bIsConnected---> %d\n", pParam->bIsConnected);
            printf("\t	@ Cfg.bIsEnbAcquisitionFrameRate---> %s\n", (pParam->bIsEnbAcquisitionFrameRate) ? "True" : "False");
            printf("\t	@ Cfg.dbAcquisitionFrameRate---> %5.3f\n", pParam->dbAcquisitionFrameRate);
            printf("\t	@ Cfg.bExposureAuto---> %s\n", (!pParam->bExposureAuto) ? "Auto" : "Off");
            printf("\t	@ Cfg.bPixelFormat---> %s\n", (pParam->bPixelFormat) ? "RGB8" : "Mono8");
            printf("\t	@ Cfg.iOffset_X ---> %d\n", pParam->iOffset_X);
            printf("\t	@ Cfg.iOffset_Y---> %d\n", pParam->iOffset_Y);
            printf("\t	@ Cfg.iWidth ---> %d\n", pParam->iWidth);
            printf("\t	@ Cfg.iHeight---> %d\n", pParam->iHeight);
            printf("\t	@ Cfg.iMax_Width ---> %d\n", pParam->iSensor_Width);
            printf("\t	@ Cfg.iMax_Height---> %d\n", pParam->iSensor_Height);
            printf("\t	@ Cfg.iBinning_Scale---> %d\n", pParam->iBinning_Scale);
            printf("<<< Device information === <<< === <<< === ===\n\n");

        }

    }
    else {

        *pParam = seGigECamConfig();

        return -1;

    }

    return nRet;

}



// This function acquires and saves 10 images from a device; please see
// Acquisition example for more in-depth comments on the acquisition of images.
int CMethod_GigECamCtrl::AcquireImages(CameraPtr pCam, string strFilePath)
{
    clock_t start, end;

    string strtmp;
    double elapsed = 0.0;


    int result = 0;
    cout << endl << "*** IMAGE ACQUISITION ***" << endl << endl;
    try
    {
        ////cycletime_start
        //start = clock();

        //// Set acquisition mode to continuous
        //if (!IsReadable(pCam->AcquisitionMode) || !IsWritable(pCam->AcquisitionMode))
        //{
        //    cout << " > ! Unable to set acquisition mode to continuous. Aborting..." << endl << endl;
        //    return -1;
        //}
        //pCam->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
        //cout << "Acquisition mode set to continuous..." << endl;

        // Begin acquiring images
        pCam->BeginAcquisition();
        cout << "Acquiring images..." << endl;

        //// Get device serial number for filename
        //gcstring deviceSerialNumber("");
        //if (IsReadable(pCam->TLDevice.DeviceSerialNumber))
        //{
        //    deviceSerialNumber = pCam->TLDevice.DeviceSerialNumber.GetValue();
        //    cout << "Device serial number retrieved as " << deviceSerialNumber << "..." << endl;
        //}
        //cout << endl;
        //// Get the value of exposure time to set an appropriate timeout for GetNextImage
        //if (!IsAvailable(pCam->ExposureTime) || !IsReadable(pCam->ExposureTime))
        //{
        //    cout << " > ! Unable to read exposure time. Aborting..." << endl << endl;
        //    return -1;
        //}

        ////cycletime_end
        //end = clock();
        //elapsed = double(end - start) / CLOCKS_PER_SEC;
        //strtmp = std::to_string(elapsed);
        //cout << "AcquisitionMode_Init() Cycle time :" << strtmp.c_str() << "(seconds)" << endl;


        //// The exposure time is retrieved in �gs so it needs to be converted to ms to keep consistency with the unit
        //// being used in GetNextImage
        //uint64_t timeout = static_cast<uint64_t>(pCam->ExposureTime.GetValue() / 1000 + 1000);
        //strtmp = std::to_string(timeout);
        //cout << "timeout value :" << strtmp.c_str() << endl;

        // Retrieve, convert, and save images
        int retrycnt = 0;
        const int k_numImages = 1;
        for ( int imageCnt = 0; imageCnt < k_numImages; ++imageCnt)
        {

            try
            {
                //cycletime_start
                start = clock();

                // Retrieve next received image and ensure image completion
                // By default, GetNextImage will block indefinitely until an image arrives.
                // In this example, the timeout value is set to [exposure time + 1000]ms to ensure that an image has
                // enough time to arrive under normal conditions
                //ImagePtr pResultImage = pCam->GetNextImage(timeout);
                ImagePtr pResultImage = pCam->GetNextImage(m_Timeout);
                

                //cycletime_end
                end = clock();
                elapsed = double(end - start) / CLOCKS_PER_SEC;
                strtmp = std::to_string(elapsed);
                cout << "GetNextImage() Cycle time :" << strtmp.c_str() << "(seconds)" << endl;

                //cycletime_start
                start = clock();

                if (pResultImage->IsIncomplete())
                {
                    cout << " > timestamp : " << getCurrentTime() << endl;
                    cout << "Image incomplete with image status " << pResultImage->GetImageStatus() << "..." << endl;
                    cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus()) << "..." << endl << endl;


                    cout << " > m_RetryCnt : " << m_RetryCnt << endl;

                    if (retrycnt < m_RetryCnt) {

                        imageCnt--;
                        ++retrycnt;

                        cout << " > retrycnt : " << retrycnt << endl;
                        continue;
                    }
                }
                else
                {
                    // Print image information
                    cout << "Grabbed image " << imageCnt
                        << ", width = " << pResultImage->GetWidth()
                        << ", height = " << pResultImage->GetHeight() << endl;
                    
                    // Convert image to mono 8
                    //ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8);
                    ImagePtr convertedImage = pResultImage;

                    cv::Mat mat_img;
                    int cvFormat(0);
                    if (1 == convertedImage->GetNumChannels())
                    {
                        cvFormat = CV_8UC1;
                    }
                    else
                    {
                        cvFormat = CV_8UC3;
                    }
                    unsigned int XPadding = convertedImage->GetXPadding();
                    unsigned int YPadding = convertedImage->GetYPadding();
                    unsigned int rowsize = convertedImage->GetWidth();
                    unsigned int colsize = convertedImage->GetHeight();

                    //Mat image(pResultImage->GetHeight(), pResultImage->GetWidth(), CV_8UC1, pResultImage->GetData(), pResultImage->GetStride());
                    mat_img = cv::Mat(colsize + YPadding,
                                    rowsize + XPadding,
                                    cvFormat,
                                    convertedImage->GetData(),
                                    convertedImage->GetStride());

                    cv::imwrite(strFilePath, mat_img);

                    mat_img.release();

                    //// Create a unique filename
                    ostringstream filename;
                    filename << strFilePath.c_str();
                    //convertedImage->Save(filename.str().c_str());


                    cout << "Image saved at " << filename.str() << endl;

                }

                // Release image
                pResultImage->Release();

                //cycletime_end
                end = clock();
                elapsed = double(end - start) / CLOCKS_PER_SEC;
                strtmp = std::to_string(elapsed);
                cout << "Save() Cycle time :" << strtmp.c_str() << "(seconds)" << endl;

                cout << endl;
            }
            catch (Spinnaker::Exception& e)
            {
                cout << "Error: " << e.what() << endl;
                result = -1;
            }



        }
        // End acquisition
        pCam->EndAcquisition();
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }
    return result;
}


int CMethod_GigECamCtrl::AcquireImages(CameraPtr pCam, cv::Mat& matImg)
{
    clock_t start, end;

    string strtmp;
    double elapsed = 0.0;


    int result = 0;
    cout << endl << "*** IMAGE ACQUISITION ***" << endl << endl;
    try
    {
        ////cycletime_start
        //start = clock();

        //// Set acquisition mode to continuous
        //if (!IsReadable(pCam->AcquisitionMode) || !IsWritable(pCam->AcquisitionMode))
        //{
        //    cout << " > ! Unable to set acquisition mode to continuous. Aborting..." << endl << endl;
        //    return -1;
        //}
        //pCam->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
        //cout << "Acquisition mode set to continuous..." << endl;

        // Begin acquiring images
        pCam->BeginAcquisition();
        cout << "Acquiring images..." << endl;

        //// Get device serial number for filename
        //gcstring deviceSerialNumber("");
        //if (IsReadable(pCam->TLDevice.DeviceSerialNumber))
        //{
        //    deviceSerialNumber = pCam->TLDevice.DeviceSerialNumber.GetValue();
        //    cout << "Device serial number retrieved as " << deviceSerialNumber << "..." << endl;
        //}
        //cout << endl;

        //// Get the value of exposure time to set an appropriate timeout for GetNextImage
        //if (!IsAvailable(pCam->ExposureTime) || !IsReadable(pCam->ExposureTime))
        //{
        //    cout << " > ! Unable to read exposure time. Aborting..." << endl << endl;
        //    return -1;
        //}

        ////cycletime_end
        //end = clock();
        //elapsed = double(end - start) / CLOCKS_PER_SEC;
        //strtmp = std::to_string(elapsed);
        //cout << "AcquisitionMode_Init() Cycle time :" << strtmp.c_str() << "(seconds)" << endl;


        //// The exposure time is retrieved in �gs so it needs to be converted to ms to keep consistency with the unit
        //// being used in GetNextImage
        //uint64_t timeout = static_cast<uint64_t>(pCam->ExposureTime.GetValue() / 1000 + 1000);
        //strtmp = std::to_string(timeout);
        //cout << "timeout value :" << strtmp.c_str() << endl;

        // Retrieve, convert, and save images
        int retrycnt = 0;
        const int k_numImages = 1;
        for ( int imageCnt = 0; imageCnt < k_numImages; ++imageCnt)
        {

            try
            {
                //cycletime_start
                start = clock();

                // Retrieve next received image and ensure image completion
                // By default, GetNextImage will block indefinitely until an image arrives.
                // In this example, the timeout value is set to [exposure time + 1000]ms to ensure that an image has
                // enough time to arrive under normal conditions
                //ImagePtr pResultImage = pCam->GetNextImage(timeout);
                ImagePtr pResultImage = pCam->GetNextImage(m_Timeout);

                //cycletime_end
                end = clock();
                elapsed = double(end - start) / CLOCKS_PER_SEC;
                strtmp = std::to_string(elapsed);
                cout << "GetNextImage() Cycle time :" << strtmp.c_str() << "(seconds)" << endl;

                //cycletime_start
                start = clock();

                if (pResultImage->IsIncomplete())
                {
                    cout << " > timestamp : " << getCurrentTime() << endl;
                    cout << "Image incomplete with image status " << pResultImage->GetImageStatus() << "..." << endl;
                    cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus()) << "..." << endl << endl;

                    cout << " > m_RetryCnt : " << m_RetryCnt << endl;

                    if (retrycnt < m_RetryCnt) {

                        imageCnt--;
                        ++retrycnt;

                        cout << " > retrycnt : " << retrycnt << endl;
                        continue;
                    }
                }
                else
                {
                    // Print image information
                    cout << "Grabbed image " << imageCnt
                        << ", width = " << pResultImage->GetWidth()
                        << ", height = " << pResultImage->GetHeight() << endl;

                    // Convert image to mono 8
                    //ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8);
                    ImagePtr convertedImage = pResultImage;

                    cv::Mat mat_img;
                    int cvFormat(0);
                    if (1 == convertedImage->GetNumChannels())
                    {
                        cvFormat = CV_8UC1;
                    }
                    else
                    {
                        cvFormat = CV_8UC3;
                    }
                    unsigned int XPadding = convertedImage->GetXPadding();
                    unsigned int YPadding = convertedImage->GetYPadding();
                    unsigned int rowsize = convertedImage->GetWidth();
                    unsigned int colsize = convertedImage->GetHeight();
                    mat_img = cv::Mat(colsize + YPadding,
                        rowsize + XPadding,
                        cvFormat,
                        convertedImage->GetData(),
                        convertedImage->GetStride());

                    if (!mat_img.empty()) {

                        mat_img.copyTo(matImg);

                        mat_img.release();
                    }

                }

                // Release image
                pResultImage->Release();

                //cycletime_end
                end = clock();
                elapsed = double(end - start) / CLOCKS_PER_SEC;
                strtmp = std::to_string(elapsed);
                cout << "Save() Cycle time :" << strtmp.c_str() << "(seconds)" << endl;

                cout << endl;
            }
            catch (Spinnaker::Exception& e)
            {
                cout << "Error: " << e.what() << endl;
                result = -1;
            }



        }
        // End acquisition
        pCam->EndAcquisition();
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }
    return result;
}



int CMethod_GigECamCtrl::AcquireStreaming(CameraPtr pCam, cv::Mat& matImg)
{
    clock_t start, end;
    double elapsed = 0.0;
    string strtmp;
    int result = 0;

    cv::Mat mat_img;

    try
    {

        //cycletime_start
        start = clock();

        iCnt_1++;

        //// # Begin acquiring images
        //pCam->BeginAcquisition();

        // Retrieve, convert, and save images
        const int k_numImages = 1;
        for (int imageCnt = 0; imageCnt < k_numImages; imageCnt++)
        {
            try
            {
                //cout << " > m_Timeout : " << std::to_string(m_Timeout) << endl;

                // Retrieve next received image and ensure image completion
                // By default, GetNextImage will block indefinitely until an image arrives.
                // In this example, the timeout value is set to [exposure time + 1000]ms to ensure that an image has
                // enough time to arrive under normal conditions
                ImagePtr pResultImage = pCam->GetNextImage(m_Timeout);

                if (pResultImage->IsIncomplete())
                {
                    cout << " > incompleteErr_timestamp : " << getCurrentTime() << endl;
                    //cout << "Image incomplete with image status " << pResultImage->GetImageStatus() << "..." << endl;
                    //cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus()) << "..." << endl << endl;
                }
                else
                {
                    ImagePtr convertedImage = pResultImage;

                    int cvFormat(0);
                    if (1 == convertedImage->GetNumChannels())
                    {
                        cvFormat = CV_8UC1;
                    }
                    else
                    {
                        cvFormat = CV_8UC3;
                    }
                    unsigned int XPadding = convertedImage->GetXPadding();
                    unsigned int YPadding = convertedImage->GetYPadding();
                    unsigned int rowsize = convertedImage->GetWidth();
                    unsigned int colsize = convertedImage->GetHeight();

                    //cv::Mat mat_img = cv::Mat(colsize + YPadding,
                    mat_img = cv::Mat(colsize + YPadding,
                        rowsize + XPadding,
                        cvFormat,
                        convertedImage->GetData(),
                        convertedImage->GetStride());

                    if (!mat_img.empty()) {

#ifdef ALGO_Enable_StreamingBufOpt_SpeedOptimization_DEBUG

                        cv::resize(mat_img, matImg, cv::Size(), 0.5, 0.5, cv::INTER_NEAREST);
#else

                        mat_img.copyTo(matImg);
#endif

                        mat_img.release();
                    }

                }

                // Release image
                pResultImage->Release();

                //cout << " > Capture_timestamp : " << getCurrentTime() << endl;

                //// # cycletime_end
                //end = clock();
                //elapsed = double(end - start) / CLOCKS_PER_SEC;
                //strtmp = std::to_string(elapsed);
                //cout << "1.[ " << iCnt_1 << " ]. --.--> PointGrey_AcquireImages() Cycle time : " << strtmp.c_str() << " (seconds)" << endl;

            }
            catch (Spinnaker::Exception& e)
            {
                cout << "Error: " << e.what() << endl;
                result = -1;
            }

        }

        //// # End acquisition
        //pCam->EndAcquisition();

    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;

}





//int CMethod_GigECamCtrl::AcquireStreaming(CameraPtr pCam, cv::Mat& matImg)
//{
//    clock_t start, end;
//    double elapsed = 0.0;
//    string strtmp;
//    int result = 0;
//
//    try
//    {
//
//        //cycletime_start
//        start = clock();
//
//
//        //// Set acquisition mode to continuous
//        //if (!IsReadable(pCam->AcquisitionMode) || !IsWritable(pCam->AcquisitionMode))
//        //{
//        //    cout << " > ! Unable to set acquisition mode to continuous. Aborting..." << endl << endl;
//        //    return -1;
//        //}
//        //pCam->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
//
//        // Begin acquiring images
//        pCam->BeginAcquisition();
//
//        //// Get device serial number for filename
//        //gcstring deviceSerialNumber("");
//        //if (IsReadable(pCam->TLDevice.DeviceSerialNumber))
//        //{
//        //    deviceSerialNumber = pCam->TLDevice.DeviceSerialNumber.GetValue();
//        //    cout << "Device serial number retrieved as " << deviceSerialNumber << "..." << endl;
//        //}
//
//        //// Get the value of exposure time to set an appropriate timeout for GetNextImage
//        //if (!IsAvailable(pCam->ExposureTime) || !IsReadable(pCam->ExposureTime))
//        //{
//        //    cout << " > ! Unable to read exposure time. Aborting..." << endl << endl;
//        //    return -1;
//        //}
//
//        //// The exposure time is retrieved in �gs so it needs to be converted to ms to keep consistency with the unit
//        //// being used in GetNextImage
//        //uint64_t timeout = static_cast<uint64_t>(pCam->ExposureTime.GetValue() / 1000 + 1000);
//        //strtmp = std::to_string(timeout);
//        ////cout << "timeout value :" << strtmp.c_str() << endl;
//
//        // Retrieve, convert, and save images
//        const int k_numImages = 1;
//        for (int imageCnt = 0; imageCnt < k_numImages; imageCnt++)
//        {
//            try
//            {
//                //cout << " > m_Timeout : " << std::to_string(m_Timeout) << endl;
//
//                // Retrieve next received image and ensure image completion
//                // By default, GetNextImage will block indefinitely until an image arrives.
//                // In this example, the timeout value is set to [exposure time + 1000]ms to ensure that an image has
//                // enough time to arrive under normal conditions
//                ImagePtr pResultImage = pCam->GetNextImage(m_Timeout);
//
//                if (pResultImage->IsIncomplete())
//                {
//                    cout << " > timestamp : " << getCurrentTime() << endl;
//                    cout << "Image incomplete with image status " << pResultImage->GetImageStatus() << "..." << endl;
//                    cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus()) << "..." << endl << endl;
//                }
//                else
//                {
//                    ImagePtr convertedImage = pResultImage;
//
//                    int cvFormat(0);
//                    if (1 == convertedImage->GetNumChannels())
//                    {
//                        cvFormat = CV_8UC1;
//                    }
//                    else
//                    {
//                        cvFormat = CV_8UC3;
//                    }
//                    unsigned int XPadding = convertedImage->GetXPadding();
//                    unsigned int YPadding = convertedImage->GetYPadding();
//                    unsigned int rowsize = convertedImage->GetWidth();
//                    unsigned int colsize = convertedImage->GetHeight();
//
//                    cv::Mat cvImg = cv::Mat(colsize + YPadding,
//                        rowsize + XPadding,
//                        cvFormat,
//                        convertedImage->GetData(),
//                        convertedImage->GetStride());
//
//                    if (!cvImg.empty()) {
//
//#ifdef ALGO_Enable_StreamingBufOpt_SpeedOptimization_DEBUG
//
//                        cv::resize(cvImg, matImg, cv::Size(), 0.5, 0.5, cv::INTER_NEAREST);
//#else
//
//                        cvImg.copyTo(matImg);
//#endif
//                    }
//
//                }
//
//                // Release image
//                pResultImage->Release();
//
//            }
//            catch (Spinnaker::Exception& e)
//            {
//                cout << "Error: " << e.what() << endl;
//                result = -1;
//            }
//
//
//        }
//
//        //// # cycletime_end
//        //end = clock();
//        //elapsed = double(end - start) / CLOCKS_PER_SEC;
//        //strtmp = std::to_string(elapsed);
//        //cout << "1. --.--> PointGrey_AcquireImages() Cycle time : " << strtmp.c_str() << " (seconds)" << endl;
//
//
//        // End acquisition
//        pCam->EndAcquisition();
//
//    }
//    catch (Spinnaker::Exception& e)
//    {
//        cout << "Error: " << e.what() << endl;
//        result = -1;
//    }
//
//    return result;
//
//}



int CMethod_GigECamCtrl::AcquireStreaming_Prepare()
{
    CameraPtr camera = m_pCam;

	cout << "AcquireStreaming_Prepare(...) >>>\n";
	cout << "tid = " << tid << endl;
	cout << "tid = " << tid << endl;

	if (camera) {

		if (bIsCreated) {

			cout << "thread already creatr()..." << endl;
			return 0;
		}

		seGigECamConfig pParam = seGigECamConfig();

		std::thread t1(&CMethod_GigECamCtrl::Thread_Acquire, this, camera, &pParam);

		tid = t1.get_id();
		t1.detach();

		bIsCreated = true;


		//cout << "   pCam =" << camera << endl;
		cout << "   tid =" << tid << endl;

	}

	cout << "AcquireStreaming_Prepare(...) <<<\n";

	return 0;
}


int CMethod_GigECamCtrl::AcquireStreaming_StartorStop(bool bflgEnableThd)
{
	cout << "AcquireStreaming_StartorStop(...) >>>\n";
	cout << "bflgEnableThd = "  << bflgEnableThd << endl;

	std::unique_lock<std::mutex> lock(u_mutex);

	if (bflgEnableThd) {

		cout << "AcquireStreaming_StartorStop(...) ==> START \n";
		notified_IsRun = true;

		std::cout << "notified_IsDone = " << notified_IsDone << endl;
		std::cout << "notified_IsRun = " << notified_IsRun << endl;

	}
	else {

        ////////////////////////////////////////////////////////////////////////////////
        //// >> Gige camera end.start
        ////////////////////////////////////////////////////////////////////////////////

        //// # End acquisition
        //cout << " > # End acquisition() " << endl;
        //m_pCam->EndAcquisition();

        usleep(50000);


        ////////////////////////////////////////////////////////////////////////////////
        //// >> Gige camera end.start
        ////////////////////////////////////////////////////////////////////////////////


		cout << "AcquireStreaming_StartorStop(...) ==> STOP \n";
		notified_IsRun = false;

		std::cout << "notified_IsDone = " << notified_IsDone << endl;
		std::cout << "notified_IsRun = " << notified_IsRun << endl;

	}	

	m_global_cfg_ParamInfo.bIsStreaming = notified_IsRun;

	m_iFrameRate_Cnt = 0;

	cout << " cond_var.notify_one()\n";
	cond_var.notify_one();

	cout << "AcquireStreaming_StartorStop(...) <<<\n";


	return 0;
}


int CMethod_GigECamCtrl::Thread_Acquire(CameraPtr pCam, LpGigECamConfig pParamOut)
{

	cout << "Thread_Acquire(...) >>>\n";
	//cout << "   pCam =" << pCam << endl;

	int ret = 0;
	string strInfo;
	clock_t start, end;
	double elapsed = 0.0;
	string strtmp;
	bool bIsAlreadyStart = 0;

	while (!notified_IsDone)
	{


        //cycletime_start
        start = clock();


		std::unique_lock<std::mutex> lock(u_mutex);
		while (!notified_IsRun)
		{
            ////////////////////////////////////////////////////////////////////////////////
            //// >> Gige camera init.start
            ////////////////////////////////////////////////////////////////////////////////
            
            //// # Set acquisition mode to continuous
            //if (IsReadable(pCam->AcquisitionMode) && IsWritable(pCam->AcquisitionMode))
            //{
            //    pCam->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
            //}
            //else
            //{
            //    cout << " > ! Unable to set acquisition mode to continuous. Aborting..." << endl << endl;
            //    //return -1;
            //}


            // # End acquisition
            if (bIsAlreadyStart) {
                cout << " > # Thread_Acquire -> End acquisition() " << endl;
                m_pCam->EndAcquisition();

                bIsAlreadyStart = 0;

                iCnt_0 = 0;
                iCnt_1 = 0;

                usleep(50000);
            }

            //// # Begin acquiring images
            //cout << " > # Thread_Acquire -> Begin Acquisition() " << endl;
            //pCam->BeginAcquisition();
            //bIsAlreadyStart = 1;


            ////////////////////////////////////////////////////////////////////////////////
            //// >> Gige camera init.end
            ////////////////////////////////////////////////////////////////////////////////


			std::cout << "worker_thread() wait\n";
			cond_var.wait(lock);
		}
		lock.unlock();

		// after the wait, we own the lock.
		//std::cout << "worker_thread() is processing data\n";
		//std::cout << "notified_IsDone = " << notified_IsDone << endl;
		//std::cout << "notified_IsRun = " << notified_IsRun << endl;


        if (!bIsAlreadyStart) {

            // # Begin acquiring images
            cout << " > # Thread_Acquire -> Begin Acquisition() " << endl;
            pCam->BeginAcquisition();
            bIsAlreadyStart = 1;
            usleep(50000);
        }

		cv::Mat matImg;

		ret = AcquireStreaming(pCam, matImg);

        if (matImg.empty()) {
            usleep(10);
            continue;
        }

#ifdef ALGO_Enable_StreamingBufOpt_AddTimestamp_DEBUG

		//strInfo = "No." + std::to_string(m_iFrameRate_Cnt++) + "_" + getCurrentTime();
		strInfo = "No." + std::to_string(m_iFrameRate_Cnt++);

		cv::putText(matImg, strInfo, cv::Point(20, 60), cv::FONT_HERSHEY_SIMPLEX, 2.0, cv::Scalar(255, 255, 255), 2);

#endif

		if (ret) {

			std::cout << "Error!!! AcquireStreaming() in Thread_Acquire(...)" << endl;
			AcquireStreaming_StartorStop(false);
            usleep(50000);
		}

		if (!matImg.empty()) {

			send_mat(matImg);

			matImg.release();
		}


        //// # cycletime_end
        //end = clock();
        //elapsed = double(end - start) / CLOCKS_PER_SEC;
        //strtmp = std::to_string(elapsed);
        //cout << "3.[ " << iCnt_0 << " ]. --.--> Total_AcquireImages() + send_mat(matImg)__CycleTime : " << strtmp.c_str() << " (seconds)" << endl;

		
		usleep(10);		
    }

	return 0;
}


int CMethod_GigECamCtrl::AcquireStreaming_Close()
{
	cout << "AcquireStreaming_Close(...) >>>\n";

	{
		std::lock_guard<std::mutex> lock(u_mutex);

		notified_IsDone = true;
		notified_IsRun = false;
	}

	cout << " cond_var.notify_one()\n";
	cond_var.notify_one();

	bIsCreated = false;


	m_global_cfg_ParamInfo.bIsStreaming = notified_IsRun;


	cout << "AcquireStreaming_Close(...) <<<\n";

	return 0;
}



static int CMethod_GigECamCtrl::GigECam_DebugPrint()
{
	int ret = EXIT_SUCCESS;

	cout << "GigECam_DebugPrint() --> TDB" << endl;

	return ret;
} 
