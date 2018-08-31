//#include "stdafx.h"
#include <WinSock2.h>
#include "BaseCamera.h"
//#include "HvDevice/HvDeviceBaseType.h"
//#include "HvDevice/HvDeviceCommDef.h"
//#include "HvDevice/HvDeviceNew.h"
#include "tinyxml/tinyxml.h"
#include<math.h>
#include<shellapi.h>
#include <Dbghelp.h>
//#pragma comment(lib, "./lib/hvdevice/HvDevice.lib")
//注意：这里要把HvDeviceDLL.h 里面的 #include "HvDeviceUtils.h" 注释掉,否则无法编译通过
#include "HvDevice/HvDeviceBaseType.h"
#include "HvDevice/HvDeviceCommDef.h"
#include "HvDevice/HvDeviceNew.h"
#include "HvDevice/HvCamera.h"
//#pragma comment(lib, "HvDevice/HvDevice.lib")
#define  VEHICLE_LISTEN_PORT 99999

//#pragma comment(lib,"WS2_32.lib")
//#pragma comment(lib, "H264API/H264.lib")
#include "utilityTool/ToolFunction.h"
#include<ctime>
//#include<atltime.h>

#define  BUFFERLENTH 256

BaseCamera::BaseCamera() :
m_hHvHandle(NULL),
m_hWnd(NULL),
m_iMsg(0),
m_iConnectMsg(0x402),
m_iDisConMsg(0x403),
m_iConnectStatus(0),
m_iLoginID(0),
m_iCompressQuality(20),
m_iDirection(0),
m_iIndex(0),
m_bLogEnable(true),
m_bSynTime(true),
m_strIP("")
{
    //合成图片初始化
//    Tool_GetEncoderClsid(L"image/jpeg", &m_jpgClsid);
//    Tool_GetEncoderClsid(L"image/bmp", &m_bmpClsid);

    ReadConfig();
    InitializeCriticalSection(&m_csLog);
    InitializeCriticalSection(&m_csFuncCallback);
}

BaseCamera::BaseCamera(const char* chIP, HWND  hWnd, int Msg) :
m_hHvHandle(NULL),
m_hWnd(hWnd),
m_iMsg(Msg),
m_iConnectMsg(0x402),
m_iDisConMsg(0x403),
m_iConnectStatus(0),
m_iLoginID(0),
m_iCompressQuality(20),
m_iDirection(0),
m_iIndex(0),
m_bLogEnable(true),
m_bSynTime(true),
m_strIP(chIP)
{
    //合成图片初始化
//    Tool_GetEncoderClsid(L"image/jpeg", &m_jpgClsid);
//    Tool_GetEncoderClsid(L"image/bmp", &m_bmpClsid);

    ReadConfig();
    InitializeCriticalSection(&m_csLog);
    InitializeCriticalSection(&m_csFuncCallback);
}

BaseCamera::~BaseCamera()
{
    InterruptionConnection();
    DisConnectCamera();
    //m_strIP.clear();

    m_hWnd = NULL;
    WriteLog("finish delete Camera");
    DeleteCriticalSection(&m_csLog);
    DeleteCriticalSection(&m_csFuncCallback);
}

void BaseCamera::ReadHistoryInfo()
{
    char iniFileName[MAX_PATH] = { 0 };
    strcat_s(iniFileName, Tool_GetCurrentPath());
    strcat_s(iniFileName, "\\SafeModeConfig.ini");

    //读取可靠性配置文件
    m_SaveModelInfo.iSafeModeEnable = GetPrivateProfileIntA(m_strIP.c_str(), "SafeModeEnable", 0, iniFileName);
    GetPrivateProfileStringA(m_strIP.c_str(), "BeginTime", "0", m_SaveModelInfo.chBeginTime, 256, iniFileName);
    GetPrivateProfileStringA(m_strIP.c_str(), "EndTime", "0", m_SaveModelInfo.chEndTime, 256, iniFileName);
    m_SaveModelInfo.iIndex = GetPrivateProfileIntA(m_strIP.c_str(), "Index", 0, iniFileName);
    m_SaveModelInfo.iDataType = GetPrivateProfileIntA(m_strIP.c_str(), "DataType", 0, iniFileName);
}

void BaseCamera::WriteHistoryInfo(SaveModeInfo& SaveInfo)
{
    char iniFileName[MAX_PATH] = { 0 };

    strcat_s(iniFileName, Tool_GetCurrentPath());
    strcat_s(iniFileName, "\\SafeModeConfig.ini");

    //读取配置文件
    char chTemp[256] = { 0 };
    //sprintf_s(chTemp, sizeof(chTemp), "%d", m_SaveModelInfo.iSafeModeEnable);
    sprintf_s(chTemp, sizeof(chTemp), "%d", m_SaveModelInfo.iSafeModeEnable);

    //if(m_SaveModelInfo.iSafeModeEnable == 0)
    //{
    //	SYSTEMTIME st;	
    //	GetLocalTime(&st);
    //	sprintf_s(m_SaveModelInfo.chBeginTime, "%d.%d.%d_%d", st.wYear, st.wMonth, st.wDay, st.wHour);
    //}
    WritePrivateProfileStringA(m_strIP.c_str(), "SafeModeEnable", chTemp, iniFileName);
    WritePrivateProfileStringA(m_strIP.c_str(), "BeginTime", SaveInfo.chBeginTime, iniFileName);
    WritePrivateProfileStringA(m_strIP.c_str(), "EndTime", SaveInfo.chEndTime, iniFileName);
    memset(chTemp, 0, sizeof(chTemp));
    //sprintf_s(chTemp, sizeof(chTemp), "%d", SaveInfo.iIndex);
    sprintf_s(chTemp, sizeof(chTemp), "%d", SaveInfo.iIndex);
    WritePrivateProfileStringA(m_strIP.c_str(), "Index", chTemp, iniFileName);
}

bool BaseCamera::SaveImgToDisk(char* chImgPath, BYTE* pImgData, DWORD dwImgSize)
{
    WriteLog("begin SaveImgToDisk");
    if (NULL == pImgData || NULL == chImgPath)
    {
        WriteLog("SaveImgToDisk, failed.NULL == pImgData || NULL == chImgPath");
        return false;
    }
    char chLogBuff[MAX_PATH] = { 0 };
    bool bRet = false;

    if (NULL != strstr(chImgPath, "\\") || NULL != strstr(chImgPath, "/"))
    {
        std::string tempFile(chImgPath);
        size_t iPosition = std::string::npos;
        if (NULL != strstr(chImgPath, "\\"))
        {
            iPosition = tempFile.rfind("\\");
        }
        else
        {
            iPosition = tempFile.rfind("/");
        }
        std::string tempDir = tempFile.substr(0, iPosition + 1);
        if (!MakeSureDirectoryPathExists(tempDir.c_str()))
        {
            memset(chLogBuff, '\0', sizeof(chLogBuff));
            //sprintf_s(chLogBuff, "%s save failed", chImgPath);
            sprintf_s(chLogBuff, sizeof(chLogBuff), "%s save failed, create path failed.", chImgPath);
            WriteLog(chLogBuff);
            return false;
        }
    }

    size_t iWritedSpecialSize = 0;
    FILE* fp = NULL;
    //fp = fopen(chImgPath, "wb+");
    errno_t errCode;
    _set_errno(0);
    errCode = fopen_s(&fp, chImgPath, "wb+");
    if (fp)
    {
        //iWritedSpecialSize = fwrite(pImgData, dwImgSize , 1, fp);
        iWritedSpecialSize = fwrite(pImgData, sizeof(BYTE), dwImgSize, fp);
        fflush(fp);
        fclose(fp);
        fp = NULL;
        bRet = true;
    }
    else
    {
        memset(chLogBuff, '\0', sizeof(chLogBuff));
        //sprintf_s(chLogBuff, "%s save failed", chImgPath);
        sprintf_s(chLogBuff, sizeof(chLogBuff), "%s open failed, error code = %d", chImgPath, errCode);
        WriteLog(chLogBuff);
    }
    if (iWritedSpecialSize == dwImgSize)
    {
        memset(chLogBuff, '\0', sizeof(chLogBuff));
        //sprintf_s(chLogBuff, "%s save success", chImgPath);
        sprintf_s(chLogBuff, sizeof(chLogBuff), "%s save success", chImgPath);
        WriteLog(chLogBuff);
    }
    else
    {
        memset(chLogBuff, '\0', sizeof(chLogBuff));
        //sprintf_s(chLogBuff, "%s save success", chImgPath);
        _get_errno(&errCode);
        sprintf_s(chLogBuff, sizeof(chLogBuff), "%s write no match, size = %lu, write size = %lu, error code = %d.",
            chImgPath,
            dwImgSize,
            iWritedSpecialSize,
            errCode);
        WriteLog(chLogBuff);
    }

    WriteLog("end SaveImgToDisk");
    return bRet;
}

#ifdef USMSVC
bool BaseCamera::SaveImgToDisk(char* chImgPath, BYTE* pImgData, DWORD dwImgSize, int iWidth, int iHeight, int iType /*= 0*/)
{
    //iType 为0时压缩图像，1时不压缩
    if (pImgData == NULL || dwImgSize < 0 || iWidth < 0 || iHeight < 0)
    {
        return false;
    }
    IStream* pStream = NULL;
    CreateStreamOnHGlobal(NULL, TRUE, &pStream);
    if (NULL == pStream)
    {
        WriteLog("SaveImgToDisk:: Stream 流创建失败. reture false");
        return false;		//流创建失败
    }
    LARGE_INTEGER LiTemp = { 0 };
    ULARGE_INTEGER ULiZero = { 0 };
    ULONG ulRealSize = 0;
    pStream->Seek(LiTemp, STREAM_SEEK_SET, NULL);
    pStream->SetSize(ULiZero);

    //将图片写入流中
    pStream->Write(pImgData, dwImgSize, &ulRealSize);
    //创建位图
    Bitmap bmpSrc(pStream);

    Bitmap bmpDest(iWidth, iHeight);
    Graphics grCompress(&bmpDest);
    Rect RCompress(0, 0, iWidth, iHeight);
    Status statuDraw = grCompress.DrawImage(&bmpSrc, RCompress, 0, 0, bmpSrc.GetWidth(), bmpSrc.GetHeight(), UnitPixel);
    if (statuDraw != Ok)
    {
        char chLog[260] = { 0 };
        sprintf_s(chLog, sizeof(chLog), "SaveImgToDisk:: DrawImage failed, the error code = %d", statuDraw);
        WriteLog(chLog);

        if (pStream)
        {
            pStream->Release();
            pStream = NULL;
        }
        return false;
    }
    Status statusDest;
    bool bRet = false;
    if (iType == 0)
    {
        ULONG quality = 50;
        EncoderParameters encoderParameters;
        encoderParameters.Count = 1;
        encoderParameters.Parameter[0].Guid = EncoderQuality;
        encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
        encoderParameters.Parameter[0].NumberOfValues = 1;

        pStream->Seek(LiTemp, STREAM_SEEK_SET, NULL);
        pStream->SetSize(ULiZero);
        encoderParameters.Parameter[0].Value = &quality;
        statusDest = bmpDest.Save(pStream, &m_jpgClsid, &encoderParameters);
        if (statusDest != Ok)
        {
            char chLog[260] = { 0 };
            //sprintf_s(chLog, "SaveImgToDisk:: failed, the error code = %d", statusDest);
            sprintf_s(chLog, sizeof(chLog), "SaveImgToDisk:: failed, the error code = %d", statusDest);
            WriteLog(chLog);

            if (pStream)
            {
                pStream->Release();
                pStream = NULL;
            }
            return false;
        }

        ULARGE_INTEGER uiLength;
        ULONG iLastSize = 0;
        if (GetStreamLength(pStream, &uiLength))
        {
            iLastSize = (int)uiLength.QuadPart;
        }
        BYTE* pDestImg = NULL;
        if (iLastSize > 0)
        {
            pDestImg = new BYTE[iLastSize];
        }
        pStream->Seek(LiTemp, STREAM_SEEK_SET, NULL);
        if (S_OK != pStream->Read(pDestImg, iLastSize, &iLastSize))
        {
            WriteLog("压缩图片保存失败");

            if (NULL != pDestImg)
            {
                delete[] pDestImg;
                pDestImg = NULL;
            }
            return false;
        }
        bRet = SaveImgToDisk(chImgPath, pDestImg, iLastSize);
        if (NULL != pDestImg)
        {
            delete[] pDestImg;
            pDestImg = NULL;
        }

        if (pStream)
        {
            pStream->Release();
            pStream = NULL;
        }
        return bRet;
    }
    else
    {
        wchar_t tempPath[260];
        MultiBYTEToWideChar(CP_ACP, NULL, chImgPath, 260, tempPath, 260);
        statusDest = bmpDest.Save(tempPath, &m_bmpClsid);
        if (statusDest == Ok)
        {
            bRet = true;
        }
        else
        {
            char chLog[260] = { 0 };
            //sprintf_s(chLog, "SaveImgToDisk:: Save failed, the error code = %d", statusDest);
            sprintf_s(chLog, sizeof(chLog), "SaveImgToDisk:: Save failed, the error code = %d", statusDest);
            WriteLog(chLog);
        }

        if (pStream)
        {
            pStream->Release();
            pStream = NULL;
        }
    }
    return bRet;
}
#endif

bool BaseCamera::SetCameraInfo(CameraInfo& camInfo)
{
    m_strIP = std::string(camInfo.chIP);
    m_strDeviceID = std::string(camInfo.chDeviceID);
    //sprintf_s(m_chDeviceID, "%s", camInfo.chDeviceID);
    //sprintf_s(m_chLaneID, "%s", camInfo.chLaneID);
    //sprintf_s(m_chStationID, "%s", camInfo.chStationID);
    sprintf_s(m_chDeviceID, sizeof(m_chDeviceID), "%s", camInfo.chDeviceID);
    sprintf_s(m_chLaneID, sizeof(m_chLaneID), "%s", camInfo.chLaneID);
    sprintf_s(m_chStationID, sizeof(m_chStationID), "%s", camInfo.chStationID);
    m_bLogEnable = camInfo.bLogEnable;
    m_bSynTime = camInfo.bSynTimeEnable;
    m_iDirection = camInfo.iDirection;

    return true;
}

int BaseCamera::GetCamStatus()
{
    //int iStatus = 1;
    //CDevState pState;
    //if (HVAPI_GetDevState(m_hHvHandle, &pState) != S_OK)
    //{
    //	iStatus = 1;
    //	char chCaptureLog3[MAX_PATH] = {0};
    //	//sprintf_s(chCaptureLog3, "Camera: %s SoftTriggerCapture failed", m_strIP.c_str());
    //	sprintf_s(chCaptureLog3, "Camera: %s GetDevState failed", m_strIP.c_str());
    //	WriteLog(chCaptureLog3);
    //}
    //else
    //{
    //	iStatus = 0;
    //	char chCaptureLog4[MAX_PATH] = {0};
    //	//sprintf_s(chCaptureLog4, "Camera: %s SoftTriggerCapture success", m_strIP.c_str());
    //	sprintf_s(chCaptureLog4, "Camera: %s GetDevState success", m_strIP.c_str());
    //	WriteLog(chCaptureLog4);
    //}
    //return iStatus;

    if (NULL == m_hHvHandle)
        return 1;
    DWORD dwStatus = 1;

    if (HVAPI_GetConnStatusEx((HVAPI_HANDLE_EX)m_hHvHandle, CONN_TYPE_RECORD, &dwStatus) == S_OK)
    {
        if (dwStatus == CONN_STATUS_NORMAL
            /*|| dwStatus == CONN_STATUS_RECVDONE*/)
        {
            m_iConnectStatus = 0;
        }
        else if (dwStatus == CONN_STATUS_RECONN)
        {
            m_iConnectStatus = 1;
        }
        else
        {
            m_iConnectStatus = 1;
        }
    }
    else
    {
        m_iConnectStatus = 1;
    }
    return m_iConnectStatus;
}

int BaseCamera::GetNetSatus()
{
    if (!m_strIP.empty() && Tool_PingIPaddress(m_strIP.c_str()))
    {
        return 0;
    }
    return 1;
}

char* BaseCamera::GetStationID()
{
    return m_chStationID;
}

char* BaseCamera::GetDeviceID()
{
    return m_chDeviceID;
}

char* BaseCamera::GetLaneID()
{
    return m_chLaneID;
}

const char* BaseCamera::GetCameraIP()
{
    return m_strIP.c_str();
}

int BaseCamera::ConnectToCamera()
{
    if (m_strIP.empty())
    {
        WriteLog("ConnectToCamera:: please finish the camera ip address");
        return -1;
    }
    if (NULL != m_hHvHandle)
    {
        InterruptionConnection();
        HVAPI_CloseEx((HVAPI_HANDLE_EX)m_hHvHandle);
        m_hHvHandle = NULL;
    }
    m_hHvHandle = HVAPI_OpenEx(m_strIP.c_str(), NULL);
    //m_hHvHandle = HVAPI_OpenChannel(m_strIP.c_str(), NULL, 0);
    if (NULL == m_hHvHandle)
    {
        WriteLog("ConnectToCamera:: Open CameraHandle failed!");
        return -2;
    }

    ReadHistoryInfo();
    char chCommand[1024] = { 0 };
    sprintf_s(chCommand, sizeof(chCommand), "DownloadRecord,BeginTime[%s],Index[%d],Enable[%d],EndTime[%s],DataInfo[%d]",
        m_SaveModelInfo.chBeginTime,
        m_SaveModelInfo.iIndex,
        m_SaveModelInfo.iSafeModeEnable,
        m_SaveModelInfo.chEndTime,
        m_SaveModelInfo.iDataType);
    //sprintf_s(chCommand, "DownloadRecord,BeginTime[%s],Index[%d],Enable[%d],EndTime[%s],DataInfo[%d]", m_SaveModelInfo.chBeginTime, m_SaveModelInfo.iIndex, m_SaveModelInfo.iSafeModeEnable, m_SaveModelInfo.chEndTime, m_SaveModelInfo.iDataType);

    WriteLog(chCommand);

    if ((HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)RecordInfoBeginCallBack, this, 0, CALLBACK_TYPE_RECORD_INFOBEGIN, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)RecordInfoEndCallBack, this, 0, CALLBACK_TYPE_RECORD_INFOEND, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)RecordInfoPlateCallBack, this, 0, CALLBACK_TYPE_RECORD_PLATE, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)RecordInfoBigImageCallBack, this, 0, CALLBACK_TYPE_RECORD_BIGIMAGE, chCommand) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)RecordInfoSmallImageCallBack, this, 0, CALLBACK_TYPE_RECORD_SMALLIMAGE, chCommand) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)RecordInfoBinaryImageCallBack, this, 0, CALLBACK_TYPE_RECORD_BINARYIMAGE, chCommand) != S_OK)/* ||
        (HVAPI_SetCallBackEx(m_hHvHandle, (PVOID)JPEGStreamCallBack, this, 0, CALLBACK_TYPE_JPEG_FRAME, NULL) != S_OK)*/
        )
    {
        WriteLog("ConnectToCamera:: SetCallBack failed.");
        HVAPI_CloseEx(m_hHvHandle);
        m_hHvHandle = NULL;
        return -3;
    }
    else
    {
        WriteLog("ConnectToCamera:: SetCallBack success.");
    }

    return 0;
}

void BaseCamera::ReadConfig()
{
    char iniFileName[MAX_PATH] = { 0 };
#ifdef GUANGXI_DLL
    sprintf_s(iniFileName, "..\\DevInterfaces\\HVCR_Signalway_V%d_%d\\HVCR_Config\\HVCR_Signalway_V%d_%d.ini", PROTOCAL_VERSION, DLL_VERSION, PROTOCAL_VERSION, DLL_VERSION);
#else
    strcat_s(iniFileName, Tool_GetCurrentPath());
    strcat_s(iniFileName, INI_FILE_NAME);
#endif

    //读取可靠性配置文件
    int iLog = GetPrivateProfileIntA("Log", "Enable", 1, iniFileName);
    m_bLogEnable = (iLog == 1) ? true : false;

    char chTemp[256] = { 0 };
    //sprintf_s(chTemp, sizeof(chTemp), "%d", iLog);
    sprintf_s(chTemp, sizeof(chTemp), "%d", iLog);
    WritePrivateProfileStringA("Log", "Enable", chTemp, iniFileName);
}

void BaseCamera::WriteFormatLog(const char* szfmt, ...)
{
    static char g_szPbString[10240] = { 0 };
    memset(g_szPbString, 0, sizeof(g_szPbString));

    va_list arg_ptr;
    va_start(arg_ptr, szfmt);
    vsnprintf_s(g_szPbString, sizeof(g_szPbString), szfmt, arg_ptr);

    WriteLog(g_szPbString);

    va_end(arg_ptr);
}

bool BaseCamera::WriteLog(const char* chlog)
{
    ReadConfig();
    if (!m_bLogEnable || NULL == chlog)
        return false;

    //取得当前的精确毫秒的时间
    SYSTEMTIME systime;
    GetLocalTime(&systime);//本地时间

    char chLogPath[512] = { 0 };
#ifdef GUANGXI_DLL
    sprintf_s(chLogPath, "..\\DevInterfaces\\HVCR_Signalway_V%d_%d\\HVCR_Logs\\XLWLog\\%04d-%02d-%02d\\%s\\", PROTOCAL_VERSION, DLL_VERSION, pTM.tm_year + 1900, pTM.tm_mon + 1, pTM.tm_mday, m_strIP.c_str());
#else
    char chLogRoot[256] = { 0 };
    Tool_ReadKeyValueFromConfigFile(INI_FILE_NAME, "Log", "Path", chLogRoot, sizeof(chLogRoot));
    if (strlen(chLogRoot) > 0)
    {
        sprintf_s(chLogPath, sizeof(chLogPath), "%s\\%04d-%02d-%02d\\%s\\",
            chLogRoot,
            systime.wYear,
            systime.wMonth,
            systime.wDay,
            m_strIP.c_str());
    }
    else
    {
        sprintf_s(chLogPath, sizeof(chLogPath), "%s\\XLWLog\\%04d-%02d-%02d\\%s\\",
            Tool_GetCurrentPath(),
            systime.wYear,
            systime.wMonth,
            systime.wDay,
            m_strIP.c_str());
    }
#endif

    MakeSureDirectoryPathExists(chLogPath);

    //每次只保留10天以内的日志文件
//    CTime tmCurrentTime = CTime::GetCurrentTime();
//    CTime tmLastMonthTime = tmCurrentTime - CTimeSpan(10, 0, 0, 0);
//    int Last_Year = tmLastMonthTime.GetYear();
//    int Last_Month = tmLastMonthTime.GetMonth();
//    int Last_Day = tmLastMonthTime.GetDay();

    time_t now = time(NULL);
    tm* ts = localtime(&now);
    ts->tm_mday = ts->tm_mday-10;
    mktime(ts); /* Normalise ts */
    int Last_Year = ts->tm_year +1900;
    int Last_Month = ts->tm_mon +1;
    int Last_Day = ts->tm_wday;

    char chOldLogFileName[MAX_PATH] = { 0 };
    //sprintf_s(chOldLogFileName, "%s\\XLWLog\\%04d-%02d-%02d\\",szFileName, Last_Year, Last_Month, Last_Day);
    sprintf_s(chOldLogFileName, sizeof(chOldLogFileName), "%s\\XLWLog\\%04d-%02d-%02d\\",
        Tool_GetCurrentPath(),
        Last_Year,
        Last_Month,
        Last_Day);

    //if (PathFileExists(chOldLogFileName))
    if(Tool_IsDirExist(chOldLogFileName))
    {
        char chCommand[512] = { 0 };
        //sprintf_s(chCommand, "/c rd /s/q %s", chOldLogFileName);
        sprintf_s(chCommand, sizeof(chCommand), "/c rd /s/q %s", chOldLogFileName);
        Tool_ExcuteCMD(chCommand);
    }

    char chLogFileName[512] = { 0 };
    //sprintf_s(chLogFileName, "%s\\CameraLog-%d-%02d_%02d.log",chLogPath, pTM->tm_year + 1900, pTM->tm_mon+1, pTM->tm_mday);
    sprintf_s(chLogFileName, sizeof(chLogFileName), "%s\\CameraLog-%d-%02d_%02d.log",
        chLogPath,
        systime.wYear,
        systime.wMonth,
        systime.wDay);

    EnterCriticalSection(&m_csLog);

    FILE *file = NULL;
    //file = fopen(chLogFileName, "a+");
    fopen_s(&file, chLogFileName, "a+");
    if (file)
    {
        fprintf(file, "%04d-%02d-%02d %02d:%02d:%02d:%03d : %s\n",
            systime.wYear,
            systime.wMonth,
            systime.wDay,
            systime.wHour,
            systime.wMinute,
            systime.wSecond,
            systime.wMilliseconds,
            chlog);
        fclose(file);
        file = NULL;
    }

    LeaveCriticalSection(&m_csLog);

    return true;
}

bool BaseCamera::TakeCapture()
{
    if (NULL == m_hHvHandle)
        return false;

    bool bRet = true;
    char chRetBuf[1024 * 10] = { 0 };
    int nRetLen = 0;
    char chCaptureLog3[MAX_PATH] = { 0 };

    if (HVAPI_ExecCmdEx(m_hHvHandle, "SoftTriggerCapture", chRetBuf, sizeof(chRetBuf), &nRetLen) != S_OK)
    {
        bRet = false;

        sprintf_s(chCaptureLog3, sizeof(chCaptureLog3), "Camera: %s SoftTriggerCapture failed", m_strIP.c_str());
        WriteLog(chCaptureLog3);
    }
    else
    {
        sprintf_s(chCaptureLog3, sizeof(chCaptureLog3), "Camera: %s SoftTriggerCapture success", m_strIP.c_str());
        WriteLog(chCaptureLog3);
    }
    return bRet;
}

bool BaseCamera::SynTime()
{
    if (NULL == m_hHvHandle)
        return false;
    //if (!m_bSynTime)
    //{
    //	return false;
    //}
    WriteLog("SynTime begin");

    SYSTEMTIME st_localTime;
    GetLocalTime(&st_localTime);

    char chTemp[256] = { 0 };
    //sprintf_s(chTemp, sizeof(chTemp), "SetTime,Date[%d.%02d.%02d],Time[%02d:%02d:%02d]",
    //	pTM->tm_year + 1900,  pTM->tm_mon, pTM->tm_mday,
    //	pTM->tm_hour, pTM->tm_min, pTM->tm_sec);
    sprintf_s(chTemp, sizeof(chTemp), "SetTime,Date[%d.%02d.%02d],Time[%02d:%02d:%02d]",
        st_localTime.wYear,
        st_localTime.wMonth,
        st_localTime.wDay,
        st_localTime.wHour,
        st_localTime.wMinute,
        st_localTime.wSecond);

    WriteLog(chTemp);
    char szRetBuf[1024] = { 0 };
    int nRetLen = 0;
    if (m_hHvHandle != NULL)
    {
        char chSynTimeLogBuf[MAX_PATH] = { 0 };
        try
        {
            if (HVAPI_ExecCmdEx(m_hHvHandle, chTemp, szRetBuf, 1024, &nRetLen) != S_OK)
            {
                memset(chSynTimeLogBuf, 0, sizeof(chSynTimeLogBuf));
                sprintf_s(chSynTimeLogBuf, sizeof(chSynTimeLogBuf), "Camera: %s SynTime failed", m_strIP.c_str());
                WriteLog(chSynTimeLogBuf);
                return false;
            }
            else
            {
                memset(chSynTimeLogBuf, 0, sizeof(chSynTimeLogBuf));
                sprintf_s(chSynTimeLogBuf, sizeof(chSynTimeLogBuf), "Camera: %s SynTime success", m_strIP.c_str());
                WriteLog(chSynTimeLogBuf);
            }
        }
        catch (...)
        {
            memset(chSynTimeLogBuf, 0, sizeof(chSynTimeLogBuf));
            sprintf_s(chSynTimeLogBuf, sizeof(chSynTimeLogBuf), "Camera: %s SynTime take exception", m_strIP.c_str());
            WriteLog(chSynTimeLogBuf);
        }
    }
    WriteLog("SynTime end");
    return true;
}

bool BaseCamera::SynTime(int Year, int Month, int Day, int Hour, int Minute, int Second, int /*MilientSecond*/)
{
    if (NULL == m_hHvHandle)
    {
        WriteLog("SynTime  failed: ConnectStatus != 0.");
        return false;
    }
    //if (!m_bSynTime)
    //{
    //	WriteLog("SynTime  failed: SynTime is not open.");
    //	return false;
    //}
    if (abs(Month) > 12 || abs(Day) > 31 || abs(Hour) > 24 || abs(Minute) > 60 || abs(Second) > 60)
    {
        WriteLog("SynTime  failed: time value is invalid.");
        return false;
    }
    WriteLog("SynTime begin");


    char chTemp[256] = { 0 };
    sprintf_s(chTemp, sizeof(chTemp), "SetTime,Date[%d.%02d.%02d],Time[%02d:%02d:%02d]",
        abs(Year), abs(Month), abs(Day),
        abs(Hour), abs(Minute), abs(Second));
    WriteLog(chTemp);
    char szRetBuf[1024] = { 0 };
    int nRetLen = 1024;
    if (m_hHvHandle != NULL)
    {
        char chSynTimeLogBuf1[MAX_PATH] = { 0 };
        try
        {
            if (HVAPI_ExecCmdEx(m_hHvHandle, chTemp, szRetBuf, 1024, &nRetLen) != S_OK)
            //if (HVAPI_SetTime(m_hHvHandle, Year, Month, Day, Hour, Minute, Second, 0) != S_OK)
            {

                memset(chSynTimeLogBuf1, 0, sizeof(chSynTimeLogBuf1));
                sprintf_s(chSynTimeLogBuf1, sizeof(chSynTimeLogBuf1), "Camera: %s SynTime failed", m_strIP.c_str());
                WriteLog(chSynTimeLogBuf1);
                return false;
            }
            else
            {
                memset(chSynTimeLogBuf1, 0, sizeof(chSynTimeLogBuf1));
                sprintf_s(chSynTimeLogBuf1, sizeof(chSynTimeLogBuf1), "Camera: %s SynTime success.", m_strIP.c_str());
                WriteLog(chSynTimeLogBuf1);
            }
        }
        catch (...)
        {
            memset(chSynTimeLogBuf1, 0, sizeof(chSynTimeLogBuf1));
            sprintf_s(chSynTimeLogBuf1, sizeof(chSynTimeLogBuf1), "Camera: %s SynTime exception.", m_strIP.c_str());
            WriteLog(chSynTimeLogBuf1);
        }
    }
    WriteLog("SynTime end");
    return true;
}
bool BaseCamera::GetDeviceTime(DeviceTime& deviceTime)
{
    if (NULL == m_hHvHandle)
        return false;

    char chRetBuf[1024] = { 0 };
    int nRetLen = 0;

    if (HVAPI_ExecCmdEx(m_hHvHandle, "DateTime", chRetBuf, sizeof(chRetBuf), &nRetLen) != S_OK)
    {
        WriteLog("GetDeviceTime:: failed.");
        return false;
    }
    WriteLog(chRetBuf);
    bool bRet = false;
    const char* chFileName = "./DateTime.xml";
    DeleteFileA(chFileName);

    //FILE* file_L = fopen(chFileName, "wb");
    FILE* file_L = NULL;
    fopen_s(&file_L, chFileName, "wb");
    if (file_L)
    {
        size_t size_Read = fwrite(chRetBuf, 1, nRetLen, file_L);
        fclose(file_L);
        file_L = NULL;
        char chFileLog[260] = { 0 };
        sprintf_s(chFileLog, sizeof(chFileLog), "GetDeviceTime:: DateTime.xml create success, size =%d ", size_Read);
        WriteLog(chFileLog);
        bRet = true;
    }
    if (!bRet)
    {
        WriteLog("GetDeviceTime:: DateTime.xml create failed.");
        return false;
    }

    const char* pDate = NULL;
    const char* pTime = NULL;
    TiXmlDocument cXmlDoc;
    //    if(cXmlDoc.Parse(chRetBuf))
    if (cXmlDoc.LoadFile(chFileName))
    {
        TiXmlElement* pSectionElement = cXmlDoc.RootElement();
        if (pSectionElement)
        {
            TiXmlElement* pChileElement = pSectionElement->FirstChildElement();
            pDate = pChileElement->Attribute("Date");
            pTime = pChileElement->Attribute("Time");
        }
        else
        {
            WriteLog("find Root element failed.");
        }
    }
    else
    {
        WriteLog("parse failed");
    }
    int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMinute = 0, iSecond = 0, iMiliSecond = 0;
    sscanf(pDate, "%04d.%02d.%02d", &iYear, &iMonth, &iDay);
    sscanf(pTime, "%02d:%02d:%02d %03d", &iHour, &iMinute, &iSecond, &iMiliSecond);

    deviceTime.iYear = iYear;
    deviceTime.iMonth = iMonth;
    deviceTime.iDay = iDay;
    deviceTime.iHour = iHour;
    deviceTime.iMinutes = iMinute;
    deviceTime.iSecond = iSecond;
    deviceTime.iMilisecond = iMiliSecond;

    return true;
}

bool BaseCamera::GetStreamLength(IStream* pStream, ULARGE_INTEGER* puliLenth)
{
    if (pStream == NULL)
        return false;

    LARGE_INTEGER liMov;
    liMov.QuadPart = 0;

    ULARGE_INTEGER uliEnd, uliBegin;

    HRESULT hr = S_FALSE;

    hr = pStream->Seek(liMov, STREAM_SEEK_END, &uliEnd);
    if (FAILED(hr))
        return false;

    hr = pStream->Seek(liMov, STREAM_SEEK_SET, &uliBegin);
    if (FAILED(hr))
        return false;

    // 差值即是流的长度
    puliLenth->QuadPart = uliEnd.QuadPart - uliBegin.QuadPart;

    return TRUE;
}

void BaseCamera::SaveResultToBufferPath(CameraResult* pResult)
{
    if (NULL == pResult)
    {
        return;
    }
    //将图片缓存到缓存目录
    //DWORD64 dwPlateTime = 0;
    //char chBigImgFileName[260] = {0};
    //char chBinImgFileName[260] = {0};
    char chPlateColor[32] = { 0 };

   // dwPlateTime = pResult->dw64TimeMS / 1000;
    if (strstr(pResult->chPlateNO, "无"))
    {
        sprintf_s(chPlateColor, sizeof(chPlateColor), "无");
    }
    else
    {
        sprintf_s(chPlateColor, sizeof(chPlateColor), "%s", pResult->chPlateColor);
    }

    //TCHAR szFileName[260] = { 0 };
    //GetModuleFileName(NULL, szFileName, 260);	//取得包括程序名的全路径
    //PathRemoveFileSpec(szFileName);				//去掉程序名	

    char chLogPath[256] = { 0 };
    Tool_ReadKeyValueFromConfigFile(INI_FILE_NAME, "Log", "Path", chLogPath, sizeof(chLogPath));


    //构造文件名称，格式： Unix时间-车牌号-车牌颜色
    //二值图
    //sprintf_s(pResult->CIMG_BinImage.chSavePath, "%s\\Buffer\\%s\\%d-%s-%s-%s-车型%d-车轴%d-%d-bin.bin", szFileName, m_strIP.c_str(), dwPlateTime, pResult->chPlateNO, chPlateColor, pResult->chPlateTime, pResult->iVehTypeNo, pResult->iAxletreeCount, pResult->dwCarID);
    ////车牌图
    //sprintf_s(pResult->CIMG_PlateImage.chSavePath, "%s\\Buffer\\%s\\%d-%s-%s-%s-车型%d-车轴%d-%d-Plate.jpg", szFileName, m_strIP.c_str(), dwPlateTime, pResult->chPlateNO, chPlateColor, pResult->chPlateTime, pResult->iVehTypeNo, pResult->iAxletreeCount, pResult->dwCarID);
    ////bestCapture
    //sprintf_s(pResult->CIMG_BestCapture.chSavePath, "%s\\Buffer\\%s\\%d-%s-%s-%s-车型%d-车轴%d-%d-BestCapture.jpg", szFileName, m_strIP.c_str(), dwPlateTime, pResult->chPlateNO, chPlateColor, pResult->chPlateTime, pResult->iVehTypeNo, pResult->iAxletreeCount, pResult->dwCarID);
    ////lastCapture
    //sprintf_s(pResult->CIMG_LastCapture.chSavePath, "%s\\Buffer\\%s\\%d-%s-%s-%s-车型%d-车轴%d-%d-LastCapture.jpg", szFileName, m_strIP.c_str(), dwPlateTime, pResult->chPlateNO, chPlateColor, pResult->chPlateTime, pResult->iVehTypeNo, pResult->iAxletreeCount, pResult->dwCarID);
    ////BEGIN_CAPTURE
    //sprintf_s(pResult->CIMG_BeginCapture.chSavePath, "%s\\Buffer\\%s\\%d-%s-%s-%s-车型%d-车轴%d-%d-BeginCapture.jpg", szFileName, m_strIP.c_str(), dwPlateTime, pResult->chPlateNO, chPlateColor, pResult->chPlateTime, pResult->iVehTypeNo, pResult->iAxletreeCount, pResult->dwCarID);
    ////BEST_SNAPSHOT
    //sprintf_s(pResult->CIMG_BestSnapshot.chSavePath, "%s\\Buffer\\%s\\%d-%s-%s-%s-车型%d-车轴%d-%d-BestSnapshot.jpg", szFileName, m_strIP.c_str(), dwPlateTime, pResult->chPlateNO, chPlateColor, pResult->chPlateTime, pResult->iVehTypeNo, pResult->iAxletreeCount, pResult->dwCarID);
    ////LAST_SNAPSHOT
    //sprintf_s(pResult->CIMG_LastSnapshot.chSavePath, "%s\\Buffer\\%s\\%d-%s-%s-%s-车型%d-车轴%d-%d-LastSnapshot.jpg", szFileName, m_strIP.c_str(), dwPlateTime, pResult->chPlateNO, chPlateColor, pResult->chPlateTime, pResult->iVehTypeNo, pResult->iAxletreeCount, pResult->dwCarID);

    int iYear = 0, iMonth = 0, iDay = 0/*, iHour*/;
    if (strlen(pResult->chPlateTime) > 0)
    {
        std::string strTime(pResult->chPlateTime);
        std::string strYear = strTime.substr(0, 4);
        iYear = atoi(strYear.c_str());

        std::string strMonth = strTime.substr(4, 2);
        iMonth = atoi(strMonth.c_str());

        std::string strDay = strTime.substr(6, 2);
        iDay = atoi(strDay.c_str());
    }
    else
    {
        time_t timeT = time(NULL);//这句返回的只是一个时间cuo
        tm timeNow;
        localtime_s(&timeNow, &timeT);
        iYear = timeNow.tm_year + 1900;
        iMonth = timeNow.tm_mon + 1;
        iDay = timeNow.tm_mday;
    }

    sprintf_s(pResult->CIMG_BinImage.chSavePath, sizeof(pResult->CIMG_BinImage.chSavePath), "%s\\%4d-%02d-%02d\\Result\\%s\\%s-%s-bin.bin", \
        chLogPath, iYear, iMonth, iDay, m_strIP.c_str(), pResult->chPlateTime, pResult->chPlateNO);
    //车牌图
    sprintf_s(pResult->CIMG_PlateImage.chSavePath, sizeof(pResult->CIMG_BinImage.chSavePath), "%s\\%4d-%02d-%02d\\Result\\%s\\%s-%s-Plate.jpg", \
        chLogPath, iYear, iMonth, iDay, m_strIP.c_str(), pResult->chPlateTime, pResult->chPlateNO);
    //bestCapture
    sprintf_s(pResult->CIMG_BestCapture.chSavePath, sizeof(pResult->CIMG_BinImage.chSavePath), "%s\\%4d-%02d-%02d\\Result\\%s\\%s-%s-BestCapture.jpg", \
        chLogPath, iYear, iMonth, iDay, m_strIP.c_str(), pResult->chPlateTime, pResult->chPlateNO);
    //lastCapture
    sprintf_s(pResult->CIMG_LastCapture.chSavePath, sizeof(pResult->CIMG_BinImage.chSavePath), "%s\\%4d-%02d-%02d\\Result\\%s\\%s-%s-LastCapture.jpg", \
        chLogPath, iYear, iMonth, iDay, m_strIP.c_str(), pResult->chPlateTime, pResult->chPlateNO);
    //BEGIN_CAPTURE
    sprintf_s(pResult->CIMG_BeginCapture.chSavePath, sizeof(pResult->CIMG_BinImage.chSavePath), "%s\\%4d-%02d-%02d\\Result\\%s\\%s-%s-BeginCapture.jpg", \
        chLogPath, iYear, iMonth, iDay, m_strIP.c_str(), pResult->chPlateTime, pResult->chPlateNO);
    //BEST_SNAPSHOT
    sprintf_s(pResult->CIMG_BestSnapshot.chSavePath, sizeof(pResult->CIMG_BinImage.chSavePath), "%s\\%4d-%02d-%02d\\Result\\%s\\%s-%s-BestSnapshot.jpg", \
        chLogPath, iYear, iMonth, iDay, m_strIP.c_str(), pResult->chPlateTime, pResult->chPlateNO);
    //LAST_SNAPSHOT
    sprintf_s(pResult->CIMG_LastSnapshot.chSavePath, sizeof(pResult->CIMG_BinImage.chSavePath), "%s\\%4d-%02d-%02d\\Result\\%s\\%s-%s-LastSnapshot.jpg", \
        chLogPath, iYear, iMonth, iDay, m_strIP.c_str(), pResult->chPlateTime, pResult->chPlateNO);


    if (pResult->CIMG_BinImage.pbImgData)
    {
        SaveImgToDisk(pResult->CIMG_BinImage.chSavePath,
            pResult->CIMG_BinImage.pbImgData,
            pResult->CIMG_BinImage.dwImgSize);
    }

    if (pResult->CIMG_PlateImage.pbImgData)
    {
        SaveImgToDisk(pResult->CIMG_PlateImage.chSavePath,
            pResult->CIMG_PlateImage.pbImgData,
            pResult->CIMG_PlateImage.dwImgSize);
    }

    if (pResult->CIMG_BeginCapture.pbImgData)
    {
        SaveImgToDisk(pResult->CIMG_BeginCapture.chSavePath,
            pResult->CIMG_BeginCapture.pbImgData,
            pResult->CIMG_BeginCapture.dwImgSize);
    }

    if (pResult->CIMG_BestCapture.pbImgData)
    {
        SaveImgToDisk(pResult->CIMG_BestCapture.chSavePath,
            pResult->CIMG_BestCapture.pbImgData,
            pResult->CIMG_BestCapture.dwImgSize);
        //bool bSave  = SaveImgToDisk(pResult->CIMG_BestCapture.chSavePath, pResult->CIMG_BestCapture.pbImgData, pResult->CIMG_BestCapture.dwImgSize, 768, 576, 1); 
    }

    if (pResult->CIMG_LastCapture.pbImgData)
    {
        SaveImgToDisk(pResult->CIMG_LastCapture.chSavePath,
            pResult->CIMG_LastCapture.pbImgData,
            pResult->CIMG_LastCapture.dwImgSize);
    }

    if (pResult->CIMG_BestSnapshot.pbImgData)
    {
        SaveImgToDisk(pResult->CIMG_BestSnapshot.chSavePath,
            pResult->CIMG_BestSnapshot.pbImgData,
            pResult->CIMG_BestSnapshot.dwImgSize);
    }

    if (pResult->CIMG_LastSnapshot.pbImgData)
    {
        SaveImgToDisk(pResult->CIMG_LastSnapshot.chSavePath,
            pResult->CIMG_BestSnapshot.pbImgData,
            pResult->CIMG_LastSnapshot.dwImgSize);
        //bool bSave  = SaveImgToDisk(pResult->CIMG_LastSnapshot.chSavePath, pResult->CIMG_LastSnapshot.pbImgData, pResult->CIMG_LastSnapshot.dwImgSize, 768, 576, 0); 
    }
}

void BaseCamera::InterruptionConnection()
{
    if (NULL == m_hHvHandle)
    {
        return;
    }

    if ((HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_RECORD_INFOBEGIN, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_RECORD_PLATE, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_RECORD_BIGIMAGE, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_RECORD_SMALLIMAGE, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_RECORD_BINARYIMAGE, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_RECORD_INFOEND, NULL) != S_OK) ||
        (HVAPI_SetCallBackEx(m_hHvHandle, NULL, this, 0, CALLBACK_TYPE_JPEG_FRAME, NULL) != S_OK)
        )
    {
        WriteLog("DisConnectToCamera:: SetCallBack NULL failed.");
        //return false;
    }
    else
    {
        WriteLog("DisConnectToCamera:: SetCallBack NULL success.");
    }
}

bool BaseCamera::DisConnectCamera()
{
    bool bRet = true;
    if (NULL != m_hHvHandle)
    {
        WriteFormatLog("HVAPI_CloseEx, begin.");
        HRESULT hRet = HVAPI_CloseEx((HVAPI_HANDLE_EX)m_hHvHandle);
        if (hRet == S_OK)
        {
            m_hHvHandle = NULL;
            WriteFormatLog("HVAPI_CloseEx, success.");
        }
        else
        {
            bRet = false;
            WriteFormatLog("HVAPI_CloseEx, failed.");
        }
    }
    return bRet;
}

void BaseCamera::SetConnectMsg(UINT iConMsg, UINT iDsiConMsg)
{
    EnterCriticalSection(&m_csFuncCallback);
    m_iConnectMsg = (iConMsg < 0x402) ? 0x402 : iConMsg;
    m_iDisConMsg = (iDsiConMsg < 0x403) ? 0x403 : iDsiConMsg;
    LeaveCriticalSection(&m_csFuncCallback);
}

//void BaseCamera::CompressImg(CameraIMG& camImg, DWORD requireSize)
//{
//    if (camImg.dwImgSize <= 0 || camImg.dwImgSize <= requireSize || !(camImg.pbImgData))
//    {
//        WriteLog("图片大小为0，或原图已满足要求大小，结束压缩.");
//        return;
//    }

//    size_t iImgSize = MAX_IMG_SIZE;
//    int iCompressQuality = 80;
//    PBYTE pbDestImg = new BYTE[iImgSize];
//    do
//    {
//        iImgSize = MAX_IMG_SIZE;
//        memset(pbDestImg, 0, iImgSize);
//        bool bRet = Tool_Img_ScaleJpg(camImg.pbImgData,
//            camImg.dwImgSize,
//            pbDestImg,
//            &iImgSize,
//            camImg.wImgWidth,
//            camImg.wImgHeight,
//            iCompressQuality);

//        iCompressQuality -= 10;
//    } while (iCompressQuality >= 10 && iImgSize > requireSize);

//    if (iImgSize <= requireSize)
//    {
//        delete[] camImg.pbImgData;
//        camImg.pbImgData = NULL;

//        camImg.pbImgData = new BYTE[iImgSize];
//        memcpy(camImg.pbImgData, pbDestImg, iImgSize);
//        camImg.dwImgSize = iImgSize;

//        char chaLog[MAX_PATH] = { 0 };
//        sprintf_s(chaLog, sizeof(chaLog), "图片压缩成功, 最后大小为%d", iImgSize);
//        WriteLog(chaLog);
//    }
//    else
//    {
//        WriteLog("图片压缩失败，继续使用原图.");
//    }
//    if (pbDestImg)
//    {
//        delete[] pbDestImg;
//        pbDestImg = NULL;
//    }
//}

void BaseCamera::SendMessageToPlateServer(int iMessageType /*= 1*/)
{
    char chMessage[50] = { 0 };
    if (iMessageType == 1)
    {
        sprintf_s(chMessage, sizeof(chMessage), "deleteOneResult");
    }
    else if (iMessageType == 2)
    {
        sprintf_s(chMessage, sizeof(chMessage), "deleteALLResult");
    }
    else
    {
        sprintf_s(chMessage, sizeof(chMessage), "hello.");
    }
    WriteLog("send 'deleteOneResult' to Server.");
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(1, 1);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        return;
    }

    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
    {
        WSACleanup();
        return;
    }

    SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);

    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(VEHICLE_LISTEN_PORT);
    connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
    int iSendLenth = send(sockClient, chMessage, strlen(chMessage) + 1, 0);

    char chLog[260] = { 0 };
    sprintf_s(chLog, sizeof(chLog), "send buffer =%s, length = %d", chMessage, iSendLenth);
    WriteLog(chLog);
    //char recvBuf[50] = {0};
    //recv(sockClient, recvBuf, 50, 0);
    //printf("%s\n", recvBuf);
    //WriteLog(recvBuf);

    closesocket(sockClient);
    WSACleanup();
    getchar();
}

bool BaseCamera::SenMessageToCamera(int iMessageType, int& iReturnValue, int& iErrorCode, int iArg)
{
    if (NULL == m_hHvHandle)
    {
        iErrorCode = -1;
        return false;
    }

    char chRetBuf[1024] = { 0 };
    char chSendBuf[256] = { 0 };
    char chLog[256] = { 0 };
    int nRetLen = 0;
//    const char* pAttribute1 = NULL;
//    const char* pAttribute2 = NULL;
//    const char* pAttribute3 = NULL;
//    const char* pAttribute4 = NULL;

    if (iMessageType == CMD_DEL_VEH_HEAD)
    {
        sprintf_s(chSendBuf, sizeof(chSendBuf), "DeleteHead_ZHWL");
    }
    else if (iMessageType == CMD_GET_VEH_LENGTH)
    {
        sprintf_s(chSendBuf, sizeof(chSendBuf), "GetQueueSize_ZHWL");
    }
    else if (iMessageType == CMD_DEL_ALL_VEH)
    {
        sprintf_s(chSendBuf, sizeof(chSendBuf), "DeleteAll_ZHWL");
    }
    else if (iMessageType == CMD_GET_VHE_HEAD)
    {
        int iValue = 0;
        if (iArg < 0)
        {
            iValue = 0;
        }
        else
        {
            iValue = iArg;
        }
        sprintf_s(chSendBuf, sizeof(chSendBuf), "GetResult_ZHWL, Value[%d]", iValue);
    }

    if (strlen(chSendBuf) <= 0)
    {
        WriteLog("SenMessageToCamera, please input the right command");
        return false;
    }

    if (HVAPI_ExecCmdEx(m_hHvHandle, chSendBuf, chRetBuf, 1024, &nRetLen) != S_OK)
    {
        memset(chLog, 0, sizeof(chLog));
        sprintf_s(chLog, sizeof(chLog), "%s  send failed.", chSendBuf);
        WriteLog(chLog);

        iErrorCode = -2;
        return false;
    }
    else
    {
        memset(chLog, 0, sizeof(chLog));
        sprintf_s(chLog, sizeof(chLog), "%s  send success.", chSendBuf);
        WriteLog(chLog);
    }

    if (iMessageType == CMD_GET_VEH_LENGTH)
    {
        int iLength = 0;
        sscanf(chRetBuf, "%d", &iLength);
        iReturnValue = iLength;
    }

    iErrorCode = 0;
    return true;
}

int BaseCamera::GetLoginID()
{
    int iLoginID = 0;
    EnterCriticalSection(&m_csFuncCallback);
    iLoginID = m_iLoginID;
    LeaveCriticalSection(&m_csFuncCallback);
    return iLoginID;
}

void BaseCamera::SetLoginID(int iID)
{
    EnterCriticalSection(&m_csFuncCallback);
    m_iLoginID = iID;
    LeaveCriticalSection(&m_csFuncCallback);
}

void BaseCamera::SetCameraIP(const char* ipAddress)
{
    EnterCriticalSection(&m_csFuncCallback);
    m_strIP = std::string(ipAddress);
    LeaveCriticalSection(&m_csFuncCallback);
}

void BaseCamera::SetWindowsWndForResultComming(HWND hWnd, int Msg)
{
    EnterCriticalSection(&m_csFuncCallback);
    m_hWnd = hWnd;
    m_iMsg = Msg;
    LeaveCriticalSection(&m_csFuncCallback);
}

void BaseCamera::SetCameraIndex(int iIndex)
{
    EnterCriticalSection(&m_csFuncCallback);
    m_iIndex = iIndex;
    LeaveCriticalSection(&m_csFuncCallback);
}

bool BaseCamera::SetOverlayVedioFont(int iFontSize, int iColor)
{
    char chLog[260] = { 0 };
    sprintf_s(chLog, sizeof(chLog), "SetOverlayVedioFont, size = %d, color = %d", iFontSize, iColor);
    WriteLog(chLog);
    HRESULT hRet = S_FALSE;
    HRESULT hRet2 = S_FALSE;
    int iR = 255, iG = 255, iB = 255;
    switch (iColor)
    {
    case 0:		//白
        iR = 255, iG = 255, iB = 255;
        break;
    case 1:		//红
        iR = 255, iG = 0, iB = 0;
        break;
    case 2:		//黄
        iR = 255, iG = 255, iB = 0;
        break;
    case 3:		//蓝
        iR = 0, iG = 0, iB = 255;
        break;
    case 4:		//黑
        iR = 0, iG = 0, iB = 0;
        break;
    case 5:		//绿
        iR = 0, iG = 255, iB = 0;
        break;
    case 6:		//紫
        iR = 138, iG = 43, iB = 226;
        break;
    default:
        iR = 255, iG = 255, iB = 255;
        break;
    }

    if (m_hHvHandle)
    {
        hRet = HVAPI_SetOSDFont((HVAPI_HANDLE_EX)m_hHvHandle, 0, iFontSize, iR, iG, iB);
        //hRet2 = HVAPI_SetOSDFont((HVAPI_HANDLE_EX)m_hHvHandle, 1 , iFontSize, iR, iG, iB);
        hRet2 = HVAPI_SetOSDFont((HVAPI_HANDLE_EX)m_hHvHandle, 2, iFontSize, iR, iG, iB);
        if (S_OK == hRet)
        {
            WriteLog("set H264 Font  success.");
        }
        if (S_OK == hRet2)
        {
            WriteLog("set JPEG Font  success");
        }
    }
    else
    {
        WriteLog("set time text, but the handle is invalid.");
    }
    if (hRet != S_OK && hRet2 != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::SetOverlayVideoText(int streamId, char* overlayText, int textLength)
{
    char chLog[1024] = { 0 };
    sprintf_s(chLog, sizeof(chLog), "SetOverlayVedioText, streamID= %d, string length = %d, overlayText = [%s] ", streamId, textLength, overlayText);
    WriteLog(chLog);
    HRESULT hRet = S_FALSE;

    if (m_hHvHandle)
    {
        /**
        * @brief		设置字符叠加字符串
        * @param[in]	hHandle			对应设备的有效句柄
        * @param[in]	nStreamId		视频流ID，0：H264,1:MJPEG
        * @param[in]	szText			叠加字符串 长度范围：0～255
        * @return		成功：S_OK；失败：E_FAIL  传入参数异常：S_FALSE
        */
        hRet = HVAPI_SetOSDText(m_hHvHandle, streamId, overlayText);
        if (S_OK == hRet)
        {
            WriteLog("set SetOverlayVedioText   success.");
        }
    }
    else
    {
        WriteLog("set SetOverlayVedioText, but the handle is invalid.");
    }
    if (hRet != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::SetOverlayVideoTextPos(int streamId, int posX, int posY)
{
    char chLog[256] = { 0 };
    sprintf_s(chLog, sizeof(chLog), "SetOverlayVideoTextPos,streamID= %d posX = %d ,posY=%d", streamId, posX, posY);
    WriteLog(chLog);
    HRESULT hRet = S_FALSE;

    if (m_hHvHandle)
    {
        /**
        * @brief		设置字符叠加位置（保持兼容）
        * @param[in]	hHandle			对应设备的有效句柄
        * @param[in]	nStreamId		视频流ID，0：H264,1:MJPEG
        * @param[in]	nPosX			叠加位置X坐标，范围: 0~图像宽
        * @param[in]	nPosY			叠加位置Y坐标，范围：0~图像高
        * @return		成功：S_OK；失败：E_FAIL  传入参数异常：S_FALSE
        */
        hRet = HVAPI_SetOSDPos(m_hHvHandle, streamId, posX, posY);
        if (S_OK == hRet)
        {
            WriteLog("set SetOverlayVideoTextPos  success.");
        }
    }
    else
    {
        WriteLog("set SetOverlayVideoTextPos, but the handle is invalid.");
    }
    if (hRet != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::SetOverlayVideoTextEnable(int streamId, bool enable)
{
    char chLog[256] = { 0 };
    sprintf_s(chLog, sizeof(chLog), "SetOverlayVideoTextEnable,streamID= %d enable = %d ", streamId, enable);
    WriteLog(chLog);
    HRESULT hRet = S_FALSE;

    if (m_hHvHandle)
    {
        /**
        * @brief		设置字符叠加开关
        * @param[in]	hHandle			对应设备的有效句柄
        * @param[in]	nStreamId		视频流ID，0：H264,1:MJPEG
        * @param[in]	fOSDEnable		0：关闭，1：打开
        * @return		成功：S_OK；失败：E_FAIL  传入参数异常：S_FALSE
        */
        hRet = HVAPI_SetOSDEnable(m_hHvHandle, streamId, enable);
        if (S_OK == hRet)
        {
            WriteLog("set HVAPI_SetOSDEnable  success.");
        }
    }
    else
    {
        WriteLog("set SetOverlayVideoTextEnable, but the handle is invalid.");
    }
    if (hRet != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::SetOverlayTimeEnable(int streamID, bool enable)
{
    char chLog[256] = { 0 };
    sprintf_s(chLog, sizeof(chLog), "SetOverlayTimeEnable,streamID= %d enable = %d ", streamID, enable);
    WriteLog(chLog);
    HRESULT hRet = S_FALSE;

    if (m_hHvHandle)
    {
        /**
        * @brief		设置时间叠加开关
        * @param[in]	hHandle			对应设备的有效句柄
        * @param[in]	nStreamId		视频流ID，0：H264,1:MJPEG
        * @param[in]	fEnable			字符叠加时间叠加开关，范围：0：关闭，1：打开
        * @return		成功：S_OK；失败：E_FAIL  传入参数异常：S_FALSE
        */
        hRet = HVAPI_SetOSDTimeEnable(m_hHvHandle, streamID, enable);
        if (S_OK == hRet)
        {
            WriteLog("set SetOverlayTimeEnable  success.");
        }
    }
    else
    {
        WriteLog("set SetOverlayTimeEnable, but the handle is invalid.");
    }
    if (hRet != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::SetOverlayTimeFormat(int streamId, int iformat)
{
    char chLog[256] = { 0 };
    sprintf_s(chLog, sizeof(chLog), "SetOverlayTimeFormat,streamID= %d iformat = %d ", streamId, iformat);
    WriteLog(chLog);
    HRESULT hRet = S_FALSE;

    if (m_hHvHandle)
    {
        INT nDateSeparator = 0, fShowWeekDay = 0, fTimeNewLine = 0, fShowMicroSec = 0;
        switch (iformat)
        {
        case 0:
            nDateSeparator = 0;
            fShowWeekDay = 0;
            fTimeNewLine = 0;
            fShowMicroSec = 0;
            break;
        case 1:
            nDateSeparator = 4;
            fShowWeekDay = 0;
            fTimeNewLine = 0;
            fShowMicroSec = 0;
            break;
        case 2:
            nDateSeparator = 5;
            fShowWeekDay = 0;
            fTimeNewLine = 0;
            fShowMicroSec = 0;
            break;
        default:
            nDateSeparator = 0;
            fShowWeekDay = 0;
            fTimeNewLine = 0;
            fShowMicroSec = 1;
            break;
        }
        /**
        * @brief	   设置OSD叠加时间格式
        * @param[in]	hHandle			对应设备的有效句柄
        * @param[in]	nStreamId		视频流ID，0：H264,1:MJPEG
        * @param[in]	nDateSeparator	日期分割符号 0:减号 1:斜杠 2:中文 3:点号
        * @param[in]	fShowWeekDay    显示星期几
        * @param[in]	fTimeNewLine	日期一行，时间另起一行
        * @param[in]	fShowMicroSec	显示微秒
        * @return		成功：S_OK；失败：E_FAIL  传入参数异常：S_FALSE
        */
        hRet = HVAPI_SetOSDTimeFormat(m_hHvHandle, streamId, nDateSeparator, fShowWeekDay, fTimeNewLine, fShowMicroSec);
        if (S_OK == hRet)
        {
            WriteLog("set SetOverlayTimeFormat  success.");
        }
    }
    else
    {
        WriteLog("set SetOverlayTimeFormat, but the handle is invalid.");
    }
    if (hRet != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::SetOverlayTimePos(int streamId, int posX, int posY)
{
    char chLog[256] = { 0 };
    sprintf_s(chLog, sizeof(chLog), "SetOverlayTimePos,streamID= %d posX = %d ,posY=%d", streamId, posX, posY);
    WriteLog(chLog);
    HRESULT hRet = S_FALSE;

    if (m_hHvHandle)
    {
        /**
        * @brief		设置时间叠加位置
        * @param[in]	hHandle			对应设备的有效句柄
        * @param[in]	nStreamId		视频流ID，0：H264,1:MJPEG
        * @param[in]	nPosX			叠加位置X坐标，范围: 0~图像宽
        * @param[in]	nPosY			叠加位置Y坐标，范围：0~图像高
        * @return		成功：S_OK；失败：E_FAIL  传入参数异常：S_FALSE
        */
        hRet = HVAPI_SetOSDTimePos(m_hHvHandle, streamId, posX, posY);
        if (S_OK == hRet)
        {
            WriteLog("set SetOverlayTimePos  success.");
        }
    }
    else
    {
        WriteLog("set SetOverlayTimePos, but the handle is invalid.");
    }
    if (hRet != S_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool BaseCamera::GetHardWareInfo(BasicInfo& bsinfo)
{
    if (NULL == m_hHvHandle)
    {
        return false;
    }
    CDevBasicInfo tempInfo;
    HRESULT hr = HVAPI_GetDevBasicInfo((HVAPI_HANDLE_EX)m_hHvHandle, &tempInfo);
    if (hr == S_OK)
    {
        memcpy(&bsinfo, &tempInfo, sizeof(BasicInfo));
        return true;
    }
    else
    {
        return false;
    }
}

unsigned int __stdcall Camera_StatusCheckThread(LPVOID lpParam)
{
    if (!lpParam)
    {
        return -1;
    }
    BaseCamera* pThis = (BaseCamera*)lpParam;
    pThis->CheckStatus();

    return 0;
}
