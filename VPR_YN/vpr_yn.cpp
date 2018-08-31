#include "vpr_yn.h"
#include"coredump/MiniDumper.h"
#include"utilityTool/log4z.h"
#include"utilityTool/ToolFunction.h"
#include"Camera/Camera6467_plate.h"
#include<memory>

CMiniDumper dumper(true);

class _tagLoger
{
public:
    _tagLoger()
    {
        zsummer::log4z::ILog4zManager::getRef().setLoggerPath(LOG4Z_MAIN_LOGGER_ID,"./XLWLog/");
        zsummer::log4z::ILog4zManager::getRef().start();
        zsummer::log4z::ILog4zManager::getRef().setLoggerLevel(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_TRACE);
    }
};

_tagLoger loger;
static char g_szlogerBuffer[1024] = {0};

std::shared_ptr<Camera6467_plate> g_plateCamer;

std::shared_ptr<Camera6467_plate> GetCameraByIP(const char* ipAddress, bool& bFind)
{
    bFind = false;
    if(g_plateCamer == NULL)
    {
        memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
        sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_RegisterMessage:: create camera first.");
        g_plateCamer = std::make_shared<Camera6467_plate>();
    }
    else
    {
        if(NULL != ipAddress && strlen(ipAddress) > 0 )
        {
            if(0 != strcmp(g_plateCamer->GetCameraIP(),ipAddress ))
            {
                g_plateCamer = std::make_shared<Camera6467_plate>();
            }
            else
            {
                bFind = true;
            }
        }
    }
    return g_plateCamer;
}

int VPR_RegisterMessage(HANDLE hHandle, int nDataMsgID)
{
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_RegisterMessage:: begin, hHandle = %p, nDataMsgID = %d", hHandle, nDataMsgID);
    LOGT(g_szlogerBuffer);
    bool bFind = false;
    auto pCamer = GetCameraByIP(NULL, bFind);
    pCamer->SetWindowsWndForResultComming((HWND)hHandle, nDataMsgID);

    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_RegisterMessage:: finish.");
    LOGT(g_szlogerBuffer);
    return 0;
}

int VPR_RegisterCallback(TNotifyFunc notifyFunc)
{
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_RegisterCallback:: begin, TNotifyFunc = %p", notifyFunc);
    LOGT(g_szlogerBuffer);

    bool bFind = false;
    auto pCamer = GetCameraByIP(NULL, bFind);
    pCamer->SetResultCallbackFunc((void*)notifyFunc);

    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_RegisterCallback:: finish.");
    LOGT(g_szlogerBuffer);
    return 0;
}

int VPR_Connect(char *lpszConnPara1, char *lpszConnPara2)
{
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_Connect:: begin, lpszConnPara1 = %s, lpszConnPara2 = %s", lpszConnPara1, lpszConnPara2);
    LOGT(g_szlogerBuffer);
    int iRet = -1;
    bool bFind = false;
    auto pCamer = GetCameraByIP(NULL, bFind);
    if(!bFind)
    {
        pCamer->SetCameraIP(lpszConnPara1);
        if(0 != pCamer->ConnectToCamera())
        {
            LOGE("VPR_Connect:: ConnectToCamera failed.");
            iRet = -1;
        }
        else
        {
            memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
            sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_Connect:: ConnectToCamera success.");
            LOGT(g_szlogerBuffer);
            iRet = 0;
        }
    }
    else
    {
        memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
        sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_Connect:: camera is already exist.");
        LOGT(g_szlogerBuffer);
        if(0 == pCamer->GetCamStatus())
        {
            iRet = 0;
        }
        else
        {
            LOGE("VPR_Connect:: camera is disconnect.");
            iRet = -1;
        }
    }
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_Connect:: finish.");
    LOGT(g_szlogerBuffer);
    return iRet;
}

int VPR_Disconnect()
{
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_Disconnect:: begin .");
    LOGT(g_szlogerBuffer);
    g_plateCamer = NULL;
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_Disconnect:: finish.");
    LOGT(g_szlogerBuffer);
    return 0;
}

int VPR_Capture()
{
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_Capture:: begin.");
    LOGT(g_szlogerBuffer);

    int iRet = -1;
    bool bFind = false;
    auto pCamer = GetCameraByIP(NULL, bFind);
    if(pCamer->TakeCapture())
    {
        memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
        sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"capture command send success.");
        LOGT(g_szlogerBuffer);
        iRet = 0;
    }
    else
    {
        LOGE("capture command send failed.");
    }
    return iRet;
}

int VPR_GetPlate(int nIndex, int *nColor, char *lpszPlate, int *nRating)
{
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetPlate:: begin. index = %d, nColor = %p, lpszPlate = %p.nRating = %p", nIndex,nColor, lpszPlate , nRating);
    LOGT(g_szlogerBuffer);
    int iRet = -1;
    bool bFind = false;
    auto pCamer = GetCameraByIP(NULL, bFind);
    auto tempResult = pCamer->GetOneResultByCarId(nIndex);
    if(!tempResult)
    {
        LOGE("VPR_GetPlate:: the result is not ready");
        iRet = -1;
    }
    else
    {
        memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
        sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetPlate:: get the result , plate no = %s",tempResult->chPlateNO);
        LOGT(g_szlogerBuffer);

        *nColor = tempResult->iPlateColor;
        *nRating = tempResult->fConfidenceLevel * 100;
        strcpy(lpszPlate,tempResult->chPlateNO );
        iRet = 0;
    }
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetPlate:: finish. index = %d, nColor = %d, lpszPlate = %s. nRating = %d", nIndex, *nColor, lpszPlate , *nRating);
    LOGT(g_szlogerBuffer);
    return iRet;
}

int VPR_GetBigImg(int nIndex, BYTE *lpImgBuf, int *nSize)
{
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetBigImg:: begin. index = %d, lpImgBuf = %p, nSize = %p", nIndex, lpImgBuf, nSize);
    LOGT(g_szlogerBuffer);

    int iRet = -1;
    bool bFind = false;
    auto pCamer = GetCameraByIP(NULL, bFind);
    auto tempResult = pCamer->GetOneResultByCarId(nIndex);
    if(!tempResult)
    {
        LOGE("VPR_GetBigImg:: the result is not ready");
        iRet = -1;
    }
    else
    {
        memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
        if(tempResult->CIMG_BestSnapshot.dwImgSize > 0 )
        {
            sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetBigImg:: get the result , plate no = %s, BigImg data = %p, size = %lu",
                    tempResult->chPlateNO, tempResult->CIMG_BestSnapshot.pbImgData, tempResult->CIMG_BestSnapshot.dwImgSize);
            LOGT(g_szlogerBuffer);

            if(lpImgBuf != NULL)
            {
                memcpy(lpImgBuf, tempResult->CIMG_BestSnapshot.pbImgData,  tempResult->CIMG_BestSnapshot.dwImgSize);
                *nSize =  tempResult->CIMG_BestSnapshot.dwImgSize;
                iRet = 0;
            }
            else
            {
                LOGE("VPR_GetBigImg:: the lpImgBuf is NULL.");
                *nSize =  0;
                iRet = -1;
            }
        }
        else if(tempResult->CIMG_LastSnapshot.dwImgSize > 0 )
        {
            memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
            if(tempResult->CIMG_LastSnapshot.dwImgSize > 0 )
            {
                sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetBigImg:: get the result , plate no = %s, BigImg data = %p, size = %lu",
                        tempResult->chPlateNO, tempResult->CIMG_LastSnapshot.pbImgData, tempResult->CIMG_LastSnapshot.dwImgSize);
                LOGT(g_szlogerBuffer);

                if(lpImgBuf != NULL)
                {
                    memcpy(lpImgBuf, tempResult->CIMG_LastSnapshot.pbImgData,  tempResult->CIMG_LastSnapshot.dwImgSize);
                    *nSize =  tempResult->CIMG_LastSnapshot.dwImgSize;
                    iRet = 0;
                }
                else
                {
                    LOGE("VPR_GetBigImg:: the lpImgBuf is NULL.");
                    *nSize =  0;
                    iRet = -1;
                }
            }
        }
        else
        {
            LOGE("VPR_GetBigImg:: the CIMG is NULL.");
            *nSize =  0;
            iRet = -1;
        }
    }
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetBigImg:: begin. index = %d,  nSize = %p", nIndex, *nSize);
    LOGT(g_szlogerBuffer);
    return iRet;
}

int VPR_GetSmallImg(int nIndex, BYTE *lpImgBuf, int *nSize)
{
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetSmallImg:: begin. index = %d, lpImgBuf = %p, nSize = %p", nIndex, lpImgBuf, nSize);
    LOGT(g_szlogerBuffer);

    int iRet = -1;
    bool bFind = false;
    auto pCamer = GetCameraByIP(NULL, bFind);
    auto tempResult = pCamer->GetOneResultByCarId(nIndex);
    if(!tempResult)
    {
        LOGE("VPR_GetSmallImg:: the result is not ready");
        iRet = -1;
    }
    else
    {
        sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetSmallImg:: get the result , plate no = %s, BigImg data = %p, size = %lu",
                tempResult->chPlateNO, tempResult->CIMG_PlateImage.pbImgData, tempResult->CIMG_PlateImage.dwImgSize);
        LOGT(g_szlogerBuffer);

        if(lpImgBuf != NULL)
        {
            memcpy(lpImgBuf, tempResult->CIMG_PlateImage.pbImgData,  tempResult->CIMG_PlateImage.dwImgSize);
            *nSize =  tempResult->CIMG_PlateImage.dwImgSize;
            iRet = 0;
        }
        else
        {
            LOGE("VPR_GetSmallImg:: the lpImgBuf is NULL.");
            *nSize =  0;
            iRet = -1;
        }
    }
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetSmallImg:: begin. index = %d,  nSize = %p", nIndex, *nSize);
    LOGT(g_szlogerBuffer);
    return iRet;
}

int VPR_GetBinImg(int nIndex, BYTE *lpImgBuf, int *nSize, int *nWidth, int *nHeight)
{
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetBinImg:: begin. index = %d, lpImgBuf = %p, nSize = %p", nIndex, lpImgBuf, nSize);
    LOGT(g_szlogerBuffer);

    int iRet = -1;
    bool bFind = false;
    auto pCamer = GetCameraByIP(NULL, bFind);
    auto tempResult = pCamer->GetOneResultByCarId(nIndex);
    if(!tempResult)
    {
        LOGE("VPR_GetBinImg:: the result is not ready");
        iRet = -1;
    }
    else
    {
        memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
        sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetBinImg:: get the result , plate no = %s, BigImg data = %p, size = %lu, width = %d, height = %d",
                tempResult->chPlateNO, tempResult->CIMG_BinImage.pbImgData, tempResult->CIMG_BinImage.dwImgSize,
                tempResult->CIMG_BinImage.wImgWidth,
                tempResult->CIMG_BinImage.wImgHeight);
        LOGT(g_szlogerBuffer);

        if(lpImgBuf != NULL)
        {
            memcpy(lpImgBuf, tempResult->CIMG_BinImage.pbImgData,  tempResult->CIMG_BinImage.dwImgSize);
            *nSize =  tempResult->CIMG_BinImage.dwImgSize;
            *nWidth = tempResult->CIMG_BinImage.wImgWidth;
            *nHeight = tempResult->CIMG_BinImage.wImgHeight;
            iRet = 0;
        }
        else
        {
            LOGE("VPR_GetBinImg:: the lpImgBuf is NULL.");
            *nSize =  0;
            iRet = -1;
        }
    }
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetBinImg:: begin. index = %d,  nSize = %p", nIndex, *nSize);
    LOGT(g_szlogerBuffer);

    return iRet;
}

int VPR_GetAllVehInfo(int nIndex, TAutoDetectedVehInfo *VehInfo)
{
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetAllVehInfo:: begin. index = %d, TAutoDetectedVehInfo = %p", nIndex, VehInfo);
    LOGT(g_szlogerBuffer);

    int iRet = -1;
    bool bFind = false;
    auto pCamer = GetCameraByIP(NULL, bFind);
    auto tempResult = pCamer->GetOneResultByCarId(nIndex);
    if(!tempResult)
    {
        LOGE("VPR_GetAllVehInfo:: the result is not ready");
        iRet = -1;
    }
    else
    {
        memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
        sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetAllVehInfo:: get the result , plate no = %s",tempResult->chPlateNO);
        LOGT(g_szlogerBuffer);
        if(VehInfo != NULL)
        {
            VehInfo->nColor = tempResult->iPlateColor;
            strcpy(VehInfo->lpszPlate, tempResult->chPlateNO);
            VehInfo->nRating = tempResult->fConfidenceLevel * 100;

            //车辆大图图像
            memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
            sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetAllVehInfo:: copy big image data.");
            LOGT(g_szlogerBuffer);
            if(tempResult->CIMG_BestSnapshot.dwImgSize > 0
                    && NULL != tempResult->CIMG_BestSnapshot.pbImgData)
            {
                memcpy(VehInfo->lpBigImgBuf, tempResult->CIMG_BestSnapshot.pbImgData,  tempResult->CIMG_BestSnapshot.dwImgSize);
                VehInfo->nBigImgSize =  tempResult->CIMG_BestSnapshot.dwImgSize;
            }
            else if(tempResult->CIMG_LastSnapshot.dwImgSize > 0
                    && NULL != tempResult->CIMG_LastSnapshot.pbImgData)
            {
                memcpy(VehInfo->lpBigImgBuf, tempResult->CIMG_LastSnapshot.pbImgData,  tempResult->CIMG_LastSnapshot.dwImgSize);
                VehInfo->nBigImgSize =  tempResult->CIMG_LastSnapshot.dwImgSize;
            }

            //车牌小图图像
            memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
            sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetAllVehInfo:: copy small image data.");
            LOGT(g_szlogerBuffer);
            memcpy(VehInfo->lpSmallImgBuf, tempResult->CIMG_PlateImage.pbImgData,  tempResult->CIMG_PlateImage.dwImgSize);
            VehInfo->nSmallImgSize =  tempResult->CIMG_PlateImage.dwImgSize;

            //二值化图像
            memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
            sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetAllVehInfo:: copy binary image data.");
            LOGT(g_szlogerBuffer);
            memcpy(VehInfo->lpBinImgBuf, tempResult->CIMG_BinImage.pbImgData,  tempResult->CIMG_BinImage.dwImgSize);
            VehInfo->nBinImgSize =  tempResult->CIMG_BinImage.dwImgSize;
            VehInfo->nBinImgWidth =  tempResult->CIMG_BinImage.wImgWidth;
            VehInfo->nBinImgHeight =  tempResult->CIMG_BinImage.wImgHeight;
        }
        else
        {
            LOGE("VPR_GetAllVehInfo:: the VehInfo is NULL.");
            iRet = -1;
        }
    }
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetAllVehInfo:: begin. index = %d", nIndex);
    LOGT(g_szlogerBuffer);
    return iRet;
}

int VPR_GetVer(char *lpszVer, char *lpszVender, char *lpszDevType)
{
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetVer:: begin. lpszVer = %p, lpszVender = %p, lpszDevType = %p",
            lpszVer, lpszVender, lpszDevType);
    LOGT(g_szlogerBuffer);

    strcpy(lpszVender, "Signalway");

    //Get device version
    BasicInfo info;
    if( g_plateCamer != NULL
            && 0 == g_plateCamer->GetCamStatus()
            && g_plateCamer->GetHardWareInfo(info)
            )
    {
        char szDevType[128] = { 0 };
        memcpy(szDevType, info.szDevType, strlen(info.szDevType));
        szDevType[strlen(info.szDevType)] = '\0';
        if(strlen(szDevType) >= 50)
        {
            memcpy(lpszDevType, szDevType, 49);
            lpszDevType[49] = '\0';
        }
        else
        {
            memcpy(lpszDevType, szDevType, strlen(szDevType)+1);
        }
    }
    else
    {
        strcpy(lpszDevType, "PCC");
    }

    //get DLL version
    char szDLLFullPsth[256] = {0};
    sprintf_s(szDLLFullPsth, sizeof(szDLLFullPsth), "%s/VPR_YN.dll");
    if(!Tool_IsFileExist(szDLLFullPsth))
    {
        memset(szDLLFullPsth, '\0', sizeof(szDLLFullPsth));
        sprintf_s(szDLLFullPsth, sizeof(szDLLFullPsth), "%s/VPRCtrl.dll");

        memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
        sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"can`t find %s. ", szDLLFullPsth );
        LOGE(g_szlogerBuffer);
    }

    if(!Tool_IsFileExist(szDLLFullPsth))
    {
        strcpy(lpszVer, "1.0.0.1");

        memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
        sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer), "can`t find %s, use %s instead. ", szDLLFullPsth , lpszVer);
        LOGE(g_szlogerBuffer);
    }
    else
    {
        std::string strDllVer = Tool_GetSoftVersion(szDLLFullPsth);
        if(!strDllVer.empty())
        {
            strcpy(lpszVer, strDllVer.c_str());
        }
        else
        {
            strcpy(lpszVer, "1.0.0.1");
        }
    }
    memset(g_szlogerBuffer, '\0', sizeof(g_szlogerBuffer));
    sprintf_s(g_szlogerBuffer, sizeof(g_szlogerBuffer),"VPR_GetVer:: finish. lpszVer = %s, lpszVender = %s, lpszDevType = %s",
            lpszVer, lpszVender, lpszDevType);
    LOGT(g_szlogerBuffer);
    return 0;
}
