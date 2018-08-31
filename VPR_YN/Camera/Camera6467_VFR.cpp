#include "stdafx.h"
#include "Camera6467_VFR.h"
#include "stdafx.h"
#include "HvDevice/HvDeviceBaseType.h"
#include "HvDevice/HvDeviceCommDef.h"
#include "HvDevice/HvDeviceNew.h"
#include "HvDevice/HvCamera.h"
#include "utilityTool/ToolFunction.h"
#include "utilityTool/log4z.h"
#include <process.h>
#include <exception>
#include <new>

#define CHECK_ARG(arg)\
    if (arg == NULL) \
    {\
        WriteFormatLog("%s is NULL", #arg); \
        return 0;\
    }

Camera6467_VFR::Camera6467_VFR() :
BaseCamera(),
m_dwLastCarID(-1),
m_iTimeInvl(10),
m_iSuperLenth(13),
m_iResultTimeOut(1500),
m_iResultMsg(WM_USER + 1),
g_pUser(NULL),
g_func_ReconnectCallback(NULL),
g_ConnectStatusCallback(NULL),
g_func_DisconnectCallback(NULL),
m_hMsgHanldle(NULL),
m_pResult(NULL),
m_bStatusCheckThreadExit(false),
m_bJpegComplete(false),
m_hStatusCheckThread(NULL)
{
    InitializeCriticalSection(&m_csResult);
    ReadConfig();

    //m_hStatusCheckThread = (HANDLE)_beginthreadex(NULL, 0, Camera_StatusCheckThread, this, 0, NULL);
}


Camera6467_VFR::Camera6467_VFR(const char* chIP, HWND hWnd, int Msg):
BaseCamera(chIP, NULL, 0),
m_dwLastCarID(-1),
m_iTimeInvl(10),
m_iSuperLenth(13),
m_iResultTimeOut(1500),
m_iResultMsg(Msg),
g_pUser(NULL),
g_func_ReconnectCallback(NULL),
g_ConnectStatusCallback(NULL),
g_func_DisconnectCallback(NULL),
m_hMsgHanldle(hWnd),
m_pResult(NULL),
m_bStatusCheckThreadExit(false),
m_bJpegComplete(false),
m_hStatusCheckThread(NULL)
{
    InitializeCriticalSection(&m_csResult);
    ReadConfig();

    //m_hStatusCheckThread = (HANDLE)_beginthreadex(NULL, 0, Camera_StatusCheckThread, this, 0, NULL);
}

Camera6467_VFR::~Camera6467_VFR()
{
    m_Camera_Plate = nullptr;
    SetCheckThreadExit(true);
    Tool_SafeCloseThread(m_hStatusCheckThread);
    InterruptionConnection();
    DisConnectCamera();

    SAFE_DELETE_OBJ(m_pResult);
    DeleteCriticalSection(&m_csResult);
}

void Camera6467_VFR::AnalysisAppendXML(CameraResult* CamResult)
{
    if (NULL == CamResult)
        return;
    if (0 != CamResult->dw64TimeMS)
    {
        CTime tm(CamResult->dw64TimeMS / 1000);
        //sprintf_s(record->chPlateTime, "%04d%02d%02d%02d%02d%02d%03d", tm.GetYear(), tm.GetMonth(), tm.GetDay(), tm.GetHour(), tm.GetMinute(), tm.GetSecond(), record->dw64TimeMS%1000);
        sprintf_s(CamResult->chPlateTime, sizeof(CamResult->chPlateTime), "%04d-%02d-%02d %02d:%02d:%02d",
            tm.GetYear(),
            tm.GetMonth(),
            tm.GetDay(),
            tm.GetHour(),
            tm.GetMinute(),
            tm.GetSecond());
    }
    else
    {
        SYSTEMTIME st;
        GetLocalTime(&st);
        //sprintf_s(record->chPlateTime, "%04d%02d%02d%02d%02d%02d%03d", st.wYear, st.wMonth, st.wDay	,st.wHour, st.wMinute,st.wSecond, st.wMilliseconds);
        sprintf_s(CamResult->chPlateTime, sizeof(CamResult->chPlateTime), "%04d-%02d-%02d %02d:%02d:%02d:%03d",
            st.wYear,
            st.wMonth,
            st.wDay,
            st.wHour,
            st.wMinute,
            st.wSecond);
    }

    char chTemp[BUFFERLENTH] = { 0 };
    int iLenth = BUFFERLENTH;

    if (Tool_GetDataFromAppenedInfo(CamResult->pcAppendInfo, "VehicleType", chTemp, &iLenth))
    {
        CamResult->iVehTypeNo = AnalysisVelchType(chTemp);
    }
    memset(chTemp, 0, sizeof(chTemp));
    iLenth = BUFFERLENTH;
    if (Tool_GetDataFromAppenedInfo(CamResult->pcAppendInfo, "AxleCnt", chTemp, &iLenth))
    {
        int iAxleCount = 0;
        sscanf_s(chTemp, "%d", &iAxleCount);
        CamResult->iAxletreeCount = iAxleCount;
        //printf("the Axletree count is %d.\n", iAxleCount);
    }
    memset(chTemp, 0, sizeof(chTemp));
    iLenth = BUFFERLENTH;
    if (Tool_GetDataFromAppenedInfo(CamResult->pcAppendInfo, "Wheelbase", chTemp, &iLenth))
    {
        float fWheelbase = 0;
        sscanf_s(chTemp, "%f", &fWheelbase);
        CamResult->fDistanceBetweenAxles = fWheelbase;
        //printf("the Wheelbase  is %f.\n", fWheelbase);
    }
    memset(chTemp, 0, sizeof(chTemp));
    iLenth = BUFFERLENTH;
    if (Tool_GetDataFromAppenedInfo(CamResult->pcAppendInfo, "CarLength", chTemp, &iLenth))
    {
        float fCarLength = 0;
        sscanf_s(chTemp, "%f", &fCarLength);
        CamResult->fVehLenth = fCarLength;
        //printf("the CarLength  is %f.\n", fCarLength);
    }
    memset(chTemp, 0, sizeof(chTemp));
    iLenth = BUFFERLENTH;
    if (Tool_GetDataFromAppenedInfo(CamResult->pcAppendInfo, "CarHeight", chTemp, &iLenth))
    {
        float fCarHeight = 0;
        sscanf_s(chTemp, "%f", &fCarHeight);
        CamResult->fVehHeight = fCarHeight;
        //printf("the CarHeight  is %f.\n", fCarHeight);
    }
    memset(chTemp, 0, sizeof(chTemp));
    iLenth = BUFFERLENTH;
    if (Tool_GetDataFromAppenedInfo(CamResult->pcAppendInfo, "BackUp", chTemp, &iLenth))
    {
        CamResult->bBackUpVeh = true;
    }
    //iLenth = BUFFERLENTH;
    //if (Tool_GetDataFromAppenedInfo(CamResult->pcAppendInfo, "Confidence", chTemp, &iLenth))
    //{
    //    float fConfidence = 0;
    //    sscanf_s(chTemp, "%f", &fConfidence);
    //    CamResult->fConfidenceLevel = fConfidence;
    //    //printf("the CarHeight  is %f.\n", fCarHeight);
    //}

    //TiXmlElement element = Tool_SelectElementByName(CamResult->pcAppendInfo, "PlateName", 2);
    //if (strlen(element.GetText()) > 0)
    //{
    //    memset(CamResult->chPlateNO, 0, sizeof(CamResult->chPlateNO));
    //    strcpy_s(CamResult->chPlateNO, sizeof(CamResult->chPlateNO), element.GetText());

    //    memset(chTemp, 0, sizeof(chTemp));
    //    strcpy_s(chTemp, sizeof(chTemp), element.GetText());

    //    iLenth = strlen(chTemp);
    //    printf("find the plate number = %s, plate length = %d\n", chTemp, iLenth);
    //    if (strlen(chTemp) > 0)
    //    {
    //        if (NULL != strstr(chTemp, "蓝") && NULL == strstr(chTemp, "白"))
    //        {
    //            CamResult->iPlateColor = COLOR_BLUE;
    //            printf("find plate color COLOR_BLUE.\n");
    //        }
    //        else if (NULL != strstr(chTemp, "黄"))
    //        {
    //            CamResult->iPlateColor = COLOR_YELLOW;
    //            printf("find plate color COLOR_YELLOW.\n");
    //        }
    //        else if (NULL != strstr(chTemp, "黑"))
    //        {
    //            CamResult->iPlateColor = COLOR_BLACK;
    //            printf("find plate color COLOR_BLACK.\n");
    //        }
    //        else if (NULL != strstr(chTemp, "白") && NULL == strstr(chTemp, "蓝"))
    //        {
    //            CamResult->iPlateColor = COLOR_WHITE;
    //            printf("find plate color COLOR_WHITE.\n");
    //        }
    //        else if (NULL != strstr(chTemp, "绿"))
    //        {
    //            size_t ilen = strlen(chTemp);
    //            if (chTemp[ilen - 1] == 'D' || chTemp[ilen - 1] == 'F')
    //            {
    //                CamResult->iPlateColor = COLOR_YELLOW_GREEN;
    //                printf("find plate color COLOR_YELLOW_GREEN.\n");
    //            }
    //            else
    //            {
    //                CamResult->iPlateColor = COLOR_GRADIENT_CREEN;
    //                printf("find plate color COLOR_GRADIENT_CREEN.\n");
    //            }
    //        }
    //        else if (NULL != strstr(chTemp, "蓝") && NULL != strstr(chTemp, "白"))
    //        {
    //            CamResult->iPlateColor = COLOR_BLUE_WHIETE;
    //            printf("find plate color COLOR_BLUE_WHIETE.\n");
    //        }
    //        else
    //        {
    //            CamResult->iPlateColor = COLOR_UNKNOW;
    //            printf("find plate color COLOR_UNKNOW.\n");
    //        }
    //    }
    //    else
    //    {
    //        CamResult->iPlateColor = COLOR_UNKNOW;
    //    }

    //}
    //else
    //{
    //    sprintf_s(CamResult->chPlateNO, sizeof(CamResult->chPlateNO), "无车牌");
    //    CamResult->iPlateColor = COLOR_UNKNOW;
    //}

}

int Camera6467_VFR::AnalysisVelchType(const char* vehType)
{
    if (vehType == NULL)
    {
        return UNKOWN_TYPE;
    }
    if (strstr(vehType, "客1"))
    {
        return BUS_TYPE_1;
    }
    else if (strstr(vehType, "客2"))
    {
        return BUS_TYPE_2;
        //printf("the Vehicle type code is 2.\n");
    }
    else if (strstr(vehType, "客3"))
    {
        return BUS_TYPE_3;
    }
    else if (strstr(vehType, "客4"))
    {
        return BUS_TYPE_4;
    }
    else if (strstr(vehType, "客5"))
    {
        return BUS_TYPE_5;
    }
    else if (strstr(vehType, "货1"))
    {
        return TRUCK_TYPE_1;
    }
    else if (strstr(vehType, "货2"))
    {
        return TRUCK_TYPE_2;
    }
    else if (strstr(vehType, "货3"))
    {
        return TRUCK_TYPE_3;
    }
    else if (strstr(vehType, "货4"))
    {
        return TRUCK_TYPE_4;
    }
    else if (strstr(vehType, "货5"))
    {
        return TRUCK_TYPE_5;
    }
    else
    {
        return UNKOWN_TYPE;
    }
}

bool Camera6467_VFR::CheckIfSuperLength(CameraResult* CamResult)
{
    CHECK_ARG(CamResult);
    if (CamResult->fVehLenth > m_iSuperLenth)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Camera6467_VFR::CheckIfBackUpVehicle(CameraResult* CamResult)
{
    CHECK_ARG(CamResult);
    if (CamResult->bBackUpVeh)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Camera6467_VFR::ReadConfig()
{
    char chTemp[MAX_PATH] = { 0 };
    int iTempValue = 0;

    iTempValue = 15;
    Tool_ReadIntValueFromConfigFile(INI_FILE_NAME, "Filter", "SuperLongVehicleLenth", iTempValue);
    m_iSuperLenth = iTempValue > 0 ? iTempValue : 15;

    //iTempValue = 1500;
    //Tool_ReadIntValueFromConfigFile(INI_FILE_NAME, "Filter", "ResultTimeOut", iTempValue);
    //m_iResultTimeOut = iTempValue > 0 ? iTempValue : 1500;    

    BaseCamera::ReadConfig();
}

int Camera6467_VFR::GetTimeInterval()
{
    int iTimeInterval = 1;
    EnterCriticalSection(&m_csFuncCallback);
    iTimeInterval = m_iTimeInvl;
    LeaveCriticalSection(&m_csFuncCallback);
    return iTimeInterval;
}

void Camera6467_VFR::SetDisConnectCallback(void* funcDisc, void* pUser)
{
    EnterCriticalSection(&m_csFuncCallback);
    g_func_DisconnectCallback = funcDisc;
    g_pUser = pUser;
    LeaveCriticalSection(&m_csFuncCallback);
}

void Camera6467_VFR::SetReConnectCallback(void* funcReco, void* pUser)
{
    EnterCriticalSection(&m_csFuncCallback);
    g_func_ReconnectCallback = funcReco;
    g_pUser = pUser;
    LeaveCriticalSection(&m_csFuncCallback);
}

void Camera6467_VFR::SetConnectStatus_Callback(void* func, void* pUser, int TimeInterval)
{
    EnterCriticalSection(&m_csFuncCallback);
    g_pUser = pUser;
    m_iTimeInvl = TimeInterval;
    LeaveCriticalSection(&m_csFuncCallback);
}

void Camera6467_VFR::SendConnetStateMsg(bool isConnect)
{
    if (isConnect)
    {
        //EnterCriticalSection(&m_csFuncCallback);
        //if (g_ConnectStatusCallback)
        //{
        //    LeaveCriticalSection(&m_csFuncCallback);
        //    //char chIP[32] = { 0 };
        //    //sprintf_s(chIP, "%s", m_strIP.c_str());
        //    //g_ConnectStatusCallback(m_iIndex, 0, g_pUser);
        //}
        //else
        //{
        //    LeaveCriticalSection(&m_csFuncCallback);
        //}

        EnterCriticalSection(&m_csFuncCallback);
        if (m_hWnd != NULL)
        {
            ::PostMessage(m_hWnd, m_iConnectMsg, NULL, NULL);
        }        
        LeaveCriticalSection(&m_csFuncCallback);
    }
    else
    {
        //EnterCriticalSection(&m_csFuncCallback);
        //if (g_ConnectStatusCallback)
        //{
        //    LeaveCriticalSection(&m_csFuncCallback);
        //    char chIP[32] = { 0 };
        //    sprintf_s(chIP, "%s", m_strIP.c_str());
        //    //g_ConnectStatusCallback(m_iIndex, -100, g_pUser);
        //}
        //else
        //{
        //    LeaveCriticalSection(&m_csFuncCallback);
        //}

        EnterCriticalSection(&m_csFuncCallback);
        if (m_hWnd != NULL)
        {
            ::PostMessage(m_hWnd, m_iDisConMsg, NULL, NULL);
        }        
        LeaveCriticalSection(&m_csFuncCallback);
    }
}

void Camera6467_VFR::SetMsgHandleAngMsg(void* handle, int msg)
{
    WriteFormatLog("SetMsgHandleAngMsg, handle = %p, msg = %d", handle, msg);
    EnterCriticalSection(&m_csFuncCallback);
    m_hMsgHanldle = handle;
    m_iMsg = msg;
    LeaveCriticalSection(&m_csFuncCallback);
}

void Camera6467_VFR::SetCheckThreadExit(bool bExit)
{
    EnterCriticalSection(&m_csFuncCallback);
    m_bStatusCheckThreadExit = bExit;
    LeaveCriticalSection(&m_csFuncCallback);
}

bool Camera6467_VFR::GetCheckThreadExit()
{
    bool bExit = false;
    EnterCriticalSection(&m_csFuncCallback);
    bExit = m_bStatusCheckThreadExit;
    LeaveCriticalSection(&m_csFuncCallback);
    return bExit;
}

bool Camera6467_VFR::OpenPlateCamera(const char* ipAddress)
{
    CHECK_ARG(ipAddress);

    WriteFormatLog("OpenPlateCamera %s begin.", ipAddress);
    m_Camera_Plate = std::make_shared<Camera6467_plate>();
    m_Camera_Plate->SetCameraIP(ipAddress);
    if (0 == m_Camera_Plate->ConnectToCamera())
    {
        WriteFormatLog("OpenPlateCamera %s success.", ipAddress);
        return true;
    }
    else
    {
        WriteFormatLog("OpenPlateCamera %s failed.", ipAddress);
        return false;
    }
}

std::shared_ptr<CameraResult> Camera6467_VFR::GetFrontResult()
{
    try
    {
        std::shared_ptr<CameraResult> pResultPlate;
        std::shared_ptr<CameraResult> pResultVFR;
        bool bGetPlateNo = false;
        if (m_Camera_Plate)
        {
            CameraResult* pTemp = m_Camera_Plate->GetOneResult();
            if (pTemp != NULL)
            {
                pResultPlate = std::shared_ptr<CameraResult>(pTemp);
                bGetPlateNo = true;
                pTemp = NULL;
            }
            else
            {
                bGetPlateNo = false;
            }
        }
        else
        {
            if (!m_resultList.empty())
            {
                pResultPlate = m_resultList.front();
            }
        }
        if (pResultPlate != nullptr)
        {
            if (bGetPlateNo)
            {
                WriteFormatLog("Get one result from plate camera , plate : %s.", pResultPlate->chPlateNO);
                WriteFormatLog("GetFrontResult, plate No list:\n");
                BaseCamera::WriteLog(m_resultList.GetAllPlateString().c_str());
                int iIndex = -1;
                if (strstr(pResultPlate->chPlateNO, "无"))
                {
                    WriteFormatLog("GetFrontResult, current plate no is 无车牌, use first result .");
                    iIndex = -1;
                }
                else
                {
                    iIndex = m_resultList.GetPositionByPlateNo(pResultPlate->chPlateNO);
                }
                if (-1 != iIndex)
                {
                    WriteFormatLog("find the result from list, index = %d.", iIndex);
                    pResultVFR = m_resultList.GetOneByIndex(iIndex);
                }
                else
                {
                    if (!m_resultList.empty())
                    {
                        WriteFormatLog("can not find result from list, Get first result.");
                        pResultVFR = m_resultList.front();
                    }
                    else
                    {
                        WriteFormatLog("can not find result from list, the list is empty.");
                    }
                }
            }
            else
            {
                WriteFormatLog("can not get result from plate camera, get from VFR list.");
                if (!m_resultList.empty())
                {
                    WriteFormatLog("Get first result.");
                    pResultVFR = m_resultList.front();
                }
                else
                {
                    WriteFormatLog("The list is empty.");
                }
            }
        }
        else
        {
            WriteFormatLog("Can not get result from  camera .");
        }
        return pResultVFR;
    }
    catch (bad_exception& e)
    {
        LOGFMTE("GetFrontResult, bad_exception, error msg = %s", e.what());

        return std::make_shared<CameraResult>();
    }
    catch (bad_alloc& e)
    {
        LOGFMTE("GetFrontResult, bad_alloc, error msg = %s", e.what());

        return std::make_shared<CameraResult>();
    }
    catch (exception& e)
    {
        LOGFMTE("GetFrontResult, exception, error msg = %s.", e.what());

        return std::make_shared<CameraResult>();
    }
    catch (void*)
    {
        LOGFMTE("GetFrontResult,  void* exception");
        return std::make_shared<CameraResult>();
    }
    catch (...)
    {
        LOGFMTE("GetFrontResult,  unknown exception");
        return std::make_shared<CameraResult>();
    }
}

std::shared_ptr<CameraResult> Camera6467_VFR::GetFrontResultByPosition(int position)
{
    try
    {
        WriteFormatLog("GetFrontResultByPosition , position = %d", position);
        std::shared_ptr<CameraResult> tempResult;
        if (m_resultList.empty())
        {
            WriteFormatLog("GetFrontResultByPosition , resultList is empty, return null.");
            return tempResult;
        }
        if (position <= 0)
        {
            tempResult = GetFrontResult();
        }
        else
        {
            if (position >= m_resultList.size())
            {
                WriteFormatLog("GetFrontResultByPosition , position : %d is larger than  resultList size %d, get the last one.",
                    position,
                    m_resultList.size());
                tempResult = m_resultList.GetOneByIndex(m_resultList.size() - 1);
            }
            else
            {
                tempResult = m_resultList.GetOneByIndex(position);
            }
        }
        WriteFormatLog("GetFrontResultByPosition ,finish");
        return tempResult;
    }
    catch (bad_exception& e)
    {
        LOGFMTE("GetFrontResultByPosition, bad_exception, error msg = %s", e.what());

        return std::make_shared<CameraResult>();
    }
    catch (bad_alloc& e)
    {
        LOGFMTE("GetFrontResultByPosition, bad_alloc, error msg = %s", e.what());

        return std::make_shared<CameraResult>();
    }
    catch (exception& e)
    {
        LOGFMTE("GetFrontResultByPosition, exception, error msg = %s.", e.what());

        return std::make_shared<CameraResult>();
    }
    catch (void*)
    {
        LOGFMTE("GetFrontResultByPosition,  void* exception");
        return std::make_shared<CameraResult>();
    }
    catch (...)
    {
        LOGFMTE("GetFrontResultByPosition,  unknown exception");
        return std::make_shared<CameraResult>();
    }
}

void Camera6467_VFR::DeleteFrontResult(const char* plateNo)
{
    try
    {
        WriteFormatLog("DeleteFrontResult, plate no = %p", plateNo);
        bool bHasPlateNo = false;
        if (NULL != plateNo && strlen(plateNo) > 0)
        {
            bHasPlateNo = true;
        }
        std::string strPlateNo;
        if (bHasPlateNo)
        {
            strPlateNo = plateNo;
            WriteFormatLog("DeleteFrontResult, has plate no : %s", plateNo);
        }
        else
        {            
            if (  m_Camera_Plate != nullptr  )
            {
                WriteFormatLog("DeleteFrontResult, get from plate camera:");
                CameraResult* pResult = m_Camera_Plate->GetOneResult();
                if (NULL != pResult)
                {
                    strPlateNo = pResult->chPlateNO;
                    SAFE_DELETE_OBJ(pResult);
                    WriteFormatLog("DeleteFrontResult, plate no : %s", strPlateNo.c_str());
                }
                else
                {
                    WriteFormatLog("DeleteFrontResult,can not get from plate camera.");
                }
            }
            else
            {
                WriteFormatLog("DeleteFrontResult, cant not get plate number from plate camera.");
            }
        }

        if (strPlateNo.empty())
        {
            WriteFormatLog("DeleteFrontResult, cant not get plate number , so delete front result.");
            m_resultList.pop_front();
        }
        else
        {
            if (std::string::npos == strPlateNo.find("无"))
            {
                int iPosition = m_resultList.GetPositionByPlateNo(strPlateNo.c_str());
                WriteFormatLog("DeleteFrontResult, GetPositionByPlateNo %d.", iPosition);
                m_resultList.DeleteToPosition(iPosition);
            }
            else
            {
                WriteFormatLog("DeleteFrontResult, current plate  %s == ‘无车牌’, do nothing.", strPlateNo.c_str());
            }
        }
        WriteFormatLog("DeleteFrontResult, final list:");
        BaseCamera::WriteLog(m_resultList.GetAllPlateString().c_str());

        WriteFormatLog("DeleteFrontResult, finish");
    }
    catch (bad_exception& e)
    {
        LOGFMTE("DeleteFrontResult, bad_exception, error msg = %s", e.what());
    }
    catch (bad_alloc& e)
    {
        LOGFMTE("DeleteFrontResult, bad_alloc, error msg = %s", e.what());
    }
    catch (exception& e)
    {
        LOGFMTE("DeleteFrontResult, exception, error msg = %s.", e.what());
    }
    catch (void*)
    {
        LOGFMTE("DeleteFrontResult,  void* exception");
    }
    catch (...)
    {
        LOGFMTE("DeleteFrontResult,  unknown exception");
    }
}

void Camera6467_VFR::ClearALLResult()
{
    WriteFormatLog("ClearALLResult begin.");
    m_resultList.ClearALL();
    WriteFormatLog("ClearALLResult finish.");
}

size_t Camera6467_VFR::GetResultListSize()
{
    WriteFormatLog("GetResultListSize begin.");
    size_t iSize = m_resultList.size();
    WriteFormatLog("GetResultListSize finish, size = %d.", iSize);
    return iSize;
}

int Camera6467_VFR::RecordInfoBegin(DWORD dwCarID)
{
    try
    {
        WriteFormatLog("RecordInfoBegin, dwCarID = %lu", dwCarID);
        SAFE_DELETE_OBJ(m_pResult);
        //m_Result = std::make_shared<CameraResult>();
        if (NULL == m_pResult)
        {
            m_pResult = new CameraResult();
        }
        CHECK_ARG(m_pResult);

        m_pResult->dwCarID = dwCarID;

        WriteFormatLog("RecordInfoBegin, finish.");
        return 0;
    }
    catch (bad_exception& e)
    {
        LOGFMTE("RecordInfoBegin, bad_exception, error msg = %s", e.what());
        return 0;
    }
    catch (bad_alloc& e)
    {
        LOGFMTE("RecordInfoBegin, bad_alloc, error msg = %s", e.what());
        return 0;
    }
    catch (exception& e)
    {
        LOGFMTE("RecordInfoBegin, exception, error msg = %s.", e.what());
        return 0;
    }
    catch (void*)
    {
        LOGFMTE("Camera6467_VFR::RecordInfoBegin,  void* exception");
        return 0;
    }
    catch (...)
    {
        LOGFMTE("Camera6467_VFR::DeleteFrontResult,  unknown exception");
        return 0;
    }
}

int Camera6467_VFR::RecordInfoEnd(DWORD dwCarID)
{
    try
    {
        WriteFormatLog("RecordInfoEnd, dwCarID = %lu", dwCarID);
        CHECK_ARG(m_pResult);

        if (dwCarID == m_pResult->dwCarID)
        {
            if (CheckIfBackUpVehicle(m_pResult))
            {
                WriteFormatLog("current result is reversing car, drop this result.");
            }
            else
            {
                if (CheckIfSuperLength(m_pResult))
                {
                    WriteFormatLog("current length %f is larger than max length %d, clear list first.", m_pResult->fVehLenth, m_iSuperLenth);
                    m_resultList.ClearALL();
                }
                WriteFormatLog("push one result to list, current list plate NO:\n");
                if (!m_resultList.empty())
                {
                    BaseCamera::WriteLog(m_resultList.GetAllPlateString().c_str());
                }
                else
                {
                    WriteFormatLog("list is empty.");
                }
                std::shared_ptr<CameraResult> pResult(m_pResult);
                if (m_dwLastCarID == dwCarID)
                {
                    WriteFormatLog("current car ID  %lu is  same wit last carID %lu, replace the last one.", dwCarID, m_dwLastCarID);
                    m_resultList.pop_back();
                }
                else
                {
                    m_dwLastCarID = dwCarID;
                }
                m_resultList.push_back(pResult);
                WriteFormatLog("after push, list plate NO:\n");
                BaseCamera::WriteLog(m_resultList.GetAllPlateString().c_str());
                m_pResult = NULL;
            }

            if (NULL != m_hMsgHanldle)
            {
                WriteLog("PostMessage");
                //::PostMessage(*((HWND*)m_hWnd),m_iMsg, 1, 0);
                ::PostMessage((HWND)m_hMsgHanldle, m_iResultMsg, (WPARAM)1, 0);
            }
        }
        else
        {
            WriteFormatLog("current car ID  %lu is not same wit result carID %lu.", dwCarID, m_pResult->dwCarID);
        }
        WriteFormatLog("RecordInfoEnd, finish");
        return 0;
    }
    catch (bad_exception& e)
    {
        LOGFMTE("RecordInfoEnd, bad_exception, error msg = %s", e.what());
        return 0;
    }
    catch (bad_alloc& e)
    {
        LOGFMTE("RecordInfoEnd, bad_alloc, error msg = %s", e.what());
        return 0;
    }
    catch (exception& e)
    {
        LOGFMTE("RecordInfoEnd, exception, error msg = %s.", e.what());
        return 0;
    }
    catch (void*)
    {
        LOGFMTE("RecordInfoEnd,  void* exception");
        return 0;
    }
    catch (...)
    {
        LOGFMTE("RecordInfoEnd, unknown exception");
        return 0;
    }
}

int Camera6467_VFR::RecordInfoPlate(DWORD dwCarID,
    LPCSTR pcPlateNo, 
    LPCSTR pcAppendInfo,
    DWORD dwRecordType,
    DWORD64 dw64TimeMS)
{
    try
    {
        WriteFormatLog("RecordInfoPlate, dwCarID = %lu, plateNo = %s, dwRecordType= %x, dw64TimeMS= %I64u",
            dwCarID,
            pcPlateNo,
            dwRecordType,
            dw64TimeMS);
        BaseCamera::WriteLog(pcAppendInfo);
        CHECK_ARG(m_pResult);

        if (dwCarID == m_pResult->dwCarID)
        {
            m_pResult->dw64TimeMS = dw64TimeMS;
            strcpy_s(m_pResult->chPlateNO, sizeof(m_pResult->chPlateNO), pcPlateNo);
            if (strlen(pcAppendInfo) < sizeof(m_pResult->pcAppendInfo))
            {
                strcpy_s(m_pResult->pcAppendInfo, sizeof(m_pResult->pcAppendInfo), pcAppendInfo);
                AnalysisAppendXML(m_pResult);
            }
            else
            {
                WriteFormatLog("strlen(pcAppendInfo)< sizeof(m_pResult->pcAppendInfo), can not AnalysisAppendXML.");
            }
        }
        else
        {
            WriteFormatLog("current car ID  %lu is not same wit result carID %lu.", dwCarID, m_pResult->dwCarID);
        }

        WriteFormatLog("RecordInfoPlate, finish.");
        return 0;
    }
    catch (bad_exception& e)
    {
        LOGFMTE("RecordInfoPlate, bad_exception, error msg = %s", e.what());
        return 0;
    }
    catch (bad_alloc& e)
    {
        LOGFMTE("RecordInfoPlate, bad_alloc, error msg = %s", e.what());
        return 0;
    }
    catch (exception& e)
    {
        LOGFMTE("RecordInfoPlate, exception, error msg = %s.", e.what());
        return 0;
    }
    catch (void*)
    {
        LOGFMTE("RecordInfoPlate,  void* exception");
        return 0;
    }
    catch (...)
    {
        LOGFMTE("RecordInfoPlate, unknown exception");
        return 0;
    }
}

int Camera6467_VFR::RecordInfoBigImage(DWORD dwCarID, 
    WORD wImgType,
    WORD wWidth, 
    WORD wHeight, 
    PBYTE pbPicData,
    DWORD dwImgDataLen,
    DWORD dwRecordType, 
    DWORD64 dw64TimeMS)
{
    WriteFormatLog("RecordInfoBigImage, dwCarID = %lu, wImgType = %u, wWidth= %u, wHeight= %u, \
        dwImgDataLen= %lu, dwRecordType = %x, dw64TimeMS = %I64u.",
        dwCarID,
        wImgType,
        wWidth,
        wHeight,
        dwImgDataLen,
        dwRecordType,
        dw64TimeMS);

    CHECK_ARG(m_pResult);
    
    if (dwCarID == m_pResult->dwCarID)
    {
        if (wImgType == RECORD_BIGIMG_BEST_SNAPSHOT)
        {
            WriteFormatLog("RecordInfoBigImage BEST_SNAPSHO  ");

            CopyDataToIMG(m_pResult->CIMG_BestSnapshot, pbPicData, wWidth, wHeight, dwImgDataLen, wImgType);
        }
        else if (wImgType == RECORD_BIGIMG_LAST_SNAPSHOT)
        {
            WriteFormatLog("RecordInfoBigImage LAST_SNAPSHOT  ");

            CopyDataToIMG(m_pResult->CIMG_LastSnapshot, pbPicData, wWidth, wHeight, dwImgDataLen, wImgType);
        }
        else if (wImgType == RECORD_BIGIMG_BEGIN_CAPTURE)
        {
            WriteFormatLog("RecordInfoBigImage BEGIN_CAPTURE  ");

            CopyDataToIMG(m_pResult->CIMG_BeginCapture, pbPicData, wWidth, wHeight, dwImgDataLen, wImgType);
        }
        else if (wImgType == RECORD_BIGIMG_BEST_CAPTURE)
        {
            WriteFormatLog("RecordInfoBigImage BEST_CAPTURE  ");

            CopyDataToIMG(m_pResult->CIMG_BestCapture, pbPicData, wWidth, wHeight, dwImgDataLen, wImgType);
        }
        else if (wImgType == RECORD_BIGIMG_LAST_CAPTURE)
        {
            WriteFormatLog("RecordInfoBigImage LAST_CAPTURE  ");

            CopyDataToIMG(m_pResult->CIMG_LastCapture, pbPicData, wWidth, wHeight, dwImgDataLen, wImgType);
        }
        else
        {
            WriteFormatLog("RecordInfoBigImage other Image, put it to  LAST_CAPTURE .");
            CopyDataToIMG(m_pResult->CIMG_LastCapture, pbPicData, wWidth, wHeight, dwImgDataLen, wImgType);
        }
    }
    else
    {
        WriteFormatLog("current car ID  %lu is not same wit result carID %lu.", dwCarID, m_pResult->dwCarID);
    }

    WriteFormatLog("RecordInfoBigImage , finish.");
    return 0;
}

int Camera6467_VFR::RecordInfoSmallImage(DWORD dwCarID,
    WORD wWidth,
    WORD wHeight,
    PBYTE pbPicData, 
    DWORD dwImgDataLen, 
    DWORD dwRecordType,
    DWORD64 dw64TimeMS)
{
    WriteFormatLog("RecordInfoSmallImage, dwCarID = %lu, wWidth= %u, wHeight= %u, \
                                              dwImgDataLen= %lu, dwRecordType = %x, dw64TimeMS = %I64u.",
                                              dwCarID,
                                              wWidth,
                                              wHeight,
                                              dwImgDataLen,
                                              dwRecordType,
                                              dw64TimeMS);

    CHECK_ARG(m_pResult);

    int iBuffLen = MAX_IMG_SIZE;
    if (m_pResult->dwCarID == dwCarID)
    {
        if (NULL != m_pResult->CIMG_PlateImage.pbImgData)
        {
            delete[] m_pResult->CIMG_PlateImage.pbImgData;
            m_pResult->CIMG_PlateImage.pbImgData = NULL;
        }
        m_pResult->CIMG_PlateImage.pbImgData = new BYTE[iBuffLen];
        WriteFormatLog("RecordInfoSmallImage 内存申请.");
        if (m_pResult->CIMG_PlateImage.pbImgData != NULL)
        {
            WriteFormatLog("RecordInfoSmallImage 内存申请成功.");
            memset(m_pResult->CIMG_PlateImage.pbImgData, 0, iBuffLen);
            HRESULT Hr = HVAPIUTILS_SmallImageToBitmapEx(pbPicData,
                wWidth,
                wHeight,
                m_pResult->CIMG_PlateImage.pbImgData,
                &iBuffLen);
            if (Hr == S_OK)
            {
                m_pResult->CIMG_PlateImage.wImgWidth = wWidth;
                m_pResult->CIMG_PlateImage.wImgHeight = wHeight;
                m_pResult->CIMG_PlateImage.dwImgSize = iBuffLen;
                if (m_Small_IMG_Temp.pbImgData == NULL)
                {
                    m_Small_IMG_Temp.pbImgData = new BYTE[MAX_IMG_SIZE];
                    memset(m_Small_IMG_Temp.pbImgData, 0, MAX_IMG_SIZE);
                }
                if (m_Small_IMG_Temp.pbImgData)
                {
                    size_t iDestLenth = MAX_IMG_SIZE;
                    memset(m_Small_IMG_Temp.pbImgData, 0, MAX_IMG_SIZE);
                    WriteFormatLog("convert bmp to jpeg , begin .");
                    bool bScale = Tool_Img_ScaleJpg(m_pResult->CIMG_PlateImage.pbImgData,
                        m_pResult->CIMG_PlateImage.dwImgSize,
                        m_Small_IMG_Temp.pbImgData,
                        &iDestLenth,
                        m_pResult->CIMG_PlateImage.wImgWidth,
                        m_pResult->CIMG_PlateImage.wImgHeight,
                        80);
                    if (bScale)
                    {
                        WriteFormatLog("convert bmp to jpeg success, begin copy.");
                        memset(m_pResult->CIMG_PlateImage.pbImgData, 0, m_pResult->CIMG_PlateImage.dwImgSize);
                        memcpy(m_pResult->CIMG_PlateImage.pbImgData, m_Small_IMG_Temp.pbImgData, iDestLenth);
                        m_pResult->CIMG_PlateImage.dwImgSize = iDestLenth;
                        WriteFormatLog("convert bmp to jpeg success, finish copy.");
                    }
                    else
                    {
                        WriteFormatLog("convert bmp to jpeg failed, use default.");
                    }
                }
            }
            else
            {
                WriteFormatLog("HVAPIUTILS_SmallImageToBitmapEx 失败.");
            }
        }
        else
        {
            WriteFormatLog("RecordInfoSmallImage 内存申请失败.");
        }
    }

    WriteFormatLog("RecordInfoSmallImage, finish.");
    return 0;
}

int Camera6467_VFR::RecordInfoBinaryImage(DWORD dwCarID, 
    WORD wWidth, 
    WORD wHeight, 
    PBYTE pbPicData, 
    DWORD dwImgDataLen,
    DWORD dwRecordType, 
    DWORD64 dw64TimeMS)
{
    WriteFormatLog("RecordInfoBinaryImage, dwCarID = %lu, wWidth= %u, wHeight= %u, \
                                                                                    dwImgDataLen= %lu, dwRecordType = %x, dw64TimeMS = %I64u.",
                                                                                    dwCarID,
                                                                                    wWidth,
                                                                                    wHeight,
                                                                                    dwImgDataLen,
                                                                                    dwRecordType,
                                                                                    dw64TimeMS);
    CHECK_ARG(m_pResult);
    if (dwCarID == m_pResult->dwCarID)
    {
        CopyDataToIMG(m_pResult->CIMG_BinImage, pbPicData, wWidth, wHeight, dwImgDataLen, 0);
    }
    else
    {
        WriteFormatLog("current car ID  %lu is not same wit result carID %lu.", dwCarID, m_pResult->dwCarID);
    }
    WriteFormatLog("RecordInfoBinaryImage, finish.");
    return 0;
}

int Camera6467_VFR::DeviceJPEGStream(PBYTE pbImageData, DWORD dwImageDataLen, DWORD dwImageType, LPCSTR szImageExtInfo)
{
    static int iCout = 0;
    if (iCout++ > 100)
    {
        WriteFormatLog("DeviceJPEGStream, pbImageData = %p, plateNo = %s, dwImageDataLen= %lu, dwImageType= %lu",
            pbImageData,
            dwImageDataLen,
            dwImageType);

        iCout = 0;
    }

    EnterCriticalSection(&m_csResult);
    m_bJpegComplete = false;

    m_CIMG_StreamJPEG.dwImgSize = dwImageDataLen;
    m_CIMG_StreamJPEG.wImgWidth = 1920;
    m_CIMG_StreamJPEG.wImgHeight = 1080;
    if (NULL == m_CIMG_StreamJPEG.pbImgData)
    {
        m_CIMG_StreamJPEG.pbImgData = new unsigned char[MAX_IMG_SIZE];
        memset(m_CIMG_StreamJPEG.pbImgData, 0, MAX_IMG_SIZE);
    }
    if (m_CIMG_StreamJPEG.pbImgData)
    {
        memset(m_CIMG_StreamJPEG.pbImgData, 0, MAX_IMG_SIZE);
        memcpy(m_CIMG_StreamJPEG.pbImgData, pbImageData, dwImageDataLen);
        m_bJpegComplete = true;
    }
    LeaveCriticalSection(&m_csResult);

    return 0;
}

void Camera6467_VFR::CheckStatus()
{
    int iLastStatus = -1;
    INT64 iLastTick = 0, iCurrentTick = 0;
    int iFirstConnctSuccess = -1;

    while (!GetCheckThreadExit())
    {
        Sleep(50);
        iCurrentTick = GetTickCount();
        int iTimeInterval = GetTimeInterval();
        if ((iCurrentTick - iLastTick) >= (iTimeInterval * 1000))
        {
            int iStatus = GetCamStatus();
            if (iStatus != iLastStatus)
            {
                if (iStatus == 0)
                {
                    //if (iStatus != iLastStatus)
                    //{
                    //	pThis->SendConnetStateMsg(true);
                    //}
                    //SendConnetStateMsg(true);
                    WriteLog("设备连接正常.");
                    iFirstConnctSuccess = 0;
                }
                else
                {
                    //SendConnetStateMsg(false);
                    WriteLog("设备连接失败, 尝试重连");

                    if (iFirstConnctSuccess == -1)
                    {
                        //pThis->ConnectToCamera();
                    }
                }
            }
            iLastStatus = iStatus;

            iLastTick = iCurrentTick;
        }
    }
}

