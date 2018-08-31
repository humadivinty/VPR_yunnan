#pragma once
//#include "G:\GitHub\VCR_YN\VCR\VCR\Camera\BaseCamera.h"
#include "Camera/BaseCamera.h"
#include "ResultListManager.h"
#include "Camera/Camera6467_plate.h"

class Camera6467_VFR :
    public BaseCamera
{
public:
    Camera6467_VFR();
    Camera6467_VFR(const char* chIP, HWND  hWnd, int Msg);
    virtual ~Camera6467_VFR();

private:
    void ReadConfig();
    void AnalysisAppendXML(CameraResult* CamResult);
    int AnalysisVelchType(const char* vehType);
    
    //************************************
    // Method:        CheckIfSuperLength
    // Describe:       查看当前车是否为超长车，如果是，则后续的逻辑需清理车型队列
    // FullName:      Camera6467_VFR::CheckIfSuperLength
    // Access:          private 
    // Returns:        bool
    // Returns Describe:
    // Parameter:    CameraResult * CamResult
    //************************************
    bool CheckIfSuperLength(CameraResult* CamResult);

    //************************************
    // Method:        CheckIfBackUpVehicle
    // Describe:        检查该结果是否为倒车结果
    // FullName:      Camera6467_VFR::CheckIfBackUpVehicle
    // Access:          private 
    // Returns:        bool
    // Returns Describe:
    // Parameter:    CameraResult * CamResult
    //************************************
    bool CheckIfBackUpVehicle(CameraResult* CamResult);

public:
    //获取发送状态回调的时间间隔
    int GetTimeInterval();
    void SetDisConnectCallback(void* funcDisc, void* pUser);
    void SetReConnectCallback(void* funcReco, void* pUser);
    void SetConnectStatus_Callback(void* func, void* pUser, int TimeInterval);
    void SendConnetStateMsg(bool isConnect);

    void SetMsgHandleAngMsg(void* handle, int msg);

    void SetCheckThreadExit(bool  bExit);
    bool GetCheckThreadExit();

    bool OpenPlateCamera(const char* ipAddress);
    std::shared_ptr<CameraResult> GetFrontResult();
    std::shared_ptr<CameraResult> GetFrontResultByPlateNo(const char* plateNo);
    std::shared_ptr<CameraResult> GetFrontResultByPosition(int position);
    void DeleteFrontResult(const char* plateNo);
    void ClearALLResult();
    size_t GetResultListSize();

public:

    virtual int RecordInfoBegin(DWORD dwCarID);

    virtual int RecordInfoEnd(DWORD dwCarID);

    virtual int RecordInfoPlate(DWORD dwCarID,
        LPCSTR pcPlateNo,
        LPCSTR pcAppendInfo,
        DWORD dwRecordType,
        DWORD64 dw64TimeMS);

    virtual int RecordInfoBigImage(DWORD dwCarID,
        WORD  wImgType,
        WORD  wWidth,
        WORD  wHeight,
        PBYTE pbPicData,
        DWORD dwImgDataLen,
        DWORD dwRecordType,
        DWORD64 dw64TimeMS);

    virtual int RecordInfoSmallImage(DWORD dwCarID,
        WORD wWidth,
        WORD wHeight,
        PBYTE pbPicData,
        DWORD dwImgDataLen,
        DWORD dwRecordType,
        DWORD64 dw64TimeMS);

    virtual int RecordInfoBinaryImage(DWORD dwCarID,
        WORD wWidth,
        WORD wHeight,
        PBYTE pbPicData,
        DWORD dwImgDataLen,
        DWORD dwRecordType,
        DWORD64 dw64TimeMS);

    virtual int DeviceJPEGStream(
        PBYTE pbImageData,
        DWORD dwImageDataLen,
        DWORD dwImageType,
        LPCSTR szImageExtInfo);

    virtual void CheckStatus();

private:
    DWORD m_dwLastCarID;
    int m_iTimeInvl;
    int m_iSuperLenth;
    int m_iResultTimeOut;
    int m_iResultMsg;

    void* g_pUser;
    void* g_func_ReconnectCallback;
    void* g_ConnectStatusCallback;
    void* g_func_DisconnectCallback;

    void* m_hMsgHanldle;
    
    bool m_bStatusCheckThreadExit;
    bool m_bJpegComplete;

    HANDLE m_hStatusCheckThread;			//状态检测线程

    CameraIMG m_Small_IMG_Temp;
    CameraIMG m_CIMG_StreamJPEG;

    CRITICAL_SECTION m_csResult;

    std::shared_ptr<Camera6467_plate> m_Camera_Plate;

    CameraResult* m_pResult;
    ResultListManager m_resultList;
};

