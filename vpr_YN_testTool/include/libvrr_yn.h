#ifndef LIBVPR_TEST_H
#define LIBVPR_TEST_H

#include"vpr_commondef.h"
#include <QtCore>

#if defined(LIBVRR_YN_LIBRARY)
#  define LIBVRR_YNSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBVRR_YNSHARED_EXPORT Q_DECL_IMPORT
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/*-----------------------------------------------------------------------------
//注册窗口句柄和自定义消息
//消息参数定义：
//PostMessage(
//			HWND,     		// 目标窗口句柄，即hHandle参数
//			WM_XXX,			// 自定义消息，即nDataMsgID参数
//			WPARAM,     	// 返回的数据序号，用来保证车牌、图像的一致性
//			LPARAM      	// 未使用，为0
//			);
-----------------------------------------------------------------------------*/
LIBVRR_YNSHARED_EXPORT int VPR_RegisterMessage (QObject* hHandle, quint32 nDataMsgID);



//注册车牌识别事件通知回调函数。
LIBVRR_YNSHARED_EXPORT int VPR_RegisterCallback (Func_VPR_NotifyFunc NotifyFunc);

/*-----------------------------------------------------------------------------
//连接车牌识别系统。
//参数：
//	lpszConnPara1：连接参数1
//	lpszConnPara2：连接参数2
//返回：
//	0-成功；
//	其它-失败；
-----------------------------------------------------------------------------*/
LIBVRR_YNSHARED_EXPORT int VPR_Connect (char * lpszConnPara1, char* lpszConnPara2);

/*-----------------------------------------------------------------------------
//断开与车牌识别系统的连接
-----------------------------------------------------------------------------*/
LIBVRR_YNSHARED_EXPORT int VPR_Disconnect (void);

/*-----------------------------------------------------------------------------
//主动触发车牌识别系统抓怕图像并识别车牌，函数为非阻塞方式，调用后立即返回，
  抓拍识别工作完成并准备好数据后通过消息或回调的方式通知上层应用。
//参数：无
//返回：
//0-成功；
//其它-失败；
-----------------------------------------------------------------------------*/
LIBVRR_YNSHARED_EXPORT int VPR_Capture (void);

/*-----------------------------------------------------------------------------
//获取指定序号车牌颜色、车牌号。
//车牌颜色定义为：0-蓝牌；1-黄牌；2-黑牌；3-白牌；-1-无颜色
//车牌格式例如：“鲁A12345”
//参数：
//nIndex：数据序号；
//nColor: 车牌颜色代码；
//lpszPlate：车牌号，如未识别出车牌或无车牌输出零长度字符串；
//nRating：车牌的可信度，为整数，例如nRating=85，表示可信度为85%。
//返回：
//0-成功；
//其它-失败；
-----------------------------------------------------------------------------*/
LIBVRR_YNSHARED_EXPORT int VPR_GetPlate (quint32 nIndex, int &nColor, char * lpszPlate, quint32 &nRating);

/*-----------------------------------------------------------------------------
//获取指定序号的车辆大图。
//参数：
//nIndex：数据序号；
//lpImgBuf：图像数据缓冲区，缓冲区大小必须≥200K;
//nSize： 输入时为上层应用分配的缓冲区大小，输出时为图像实际大小；
//如图像大小大于缓冲区大小，也通过该参数输出图像实际大小。
//返回：
//0-成功；
//1-图像大小大于缓冲区
//2-无图像
//其它-失败；
-----------------------------------------------------------------------------*/
LIBVRR_YNSHARED_EXPORT int VPR_GetBigImg (quint32 nIndex, quint8* lpImgBuf, quint32 &nSize);

/*-----------------------------------------------------------------------------
获取指定序号的车牌小图。
参数：
nIndex：数据序号；
lpImgBuf：图像数据缓冲区，缓冲区大小必须≥20K;
nSize： 输入时为上层应用分配的缓冲区大小，输出时为图像实际大小；
如图像大小大于缓冲区大小，也通过该参数输出图像实际大小。
返回：
0-成功；
1-图像大小大于缓冲区
2-无图像
其它-失败；
-----------------------------------------------------------------------------*/
LIBVRR_YNSHARED_EXPORT int VPR_GetSmallImg (quint32 nIndex, quint8* lpImgBuf, quint32 &nSize);

/*-----------------------------------------------------------------------------
获取指定序号的车牌二值图。
参数：
nIndex：数据序号；
lpImgBuf：图像数据缓冲区，缓冲区大小必须≥280字节;
nWeidth：输出的二值图像宽度；
nHeight：输出的二值图像高度；
nSize： 输入时为上层应用分配的缓冲区大小，输出时为图像实际大小；
如图像大小大于缓冲区大小，也通过该参数输出图像实际大小。
返回：
0-成功；
1-图像大小大于缓冲区
2-无图像
其它-失败；
-----------------------------------------------------------------------------*/
LIBVRR_YNSHARED_EXPORT int VPR_GetBinImg (quint32 nIndex, quint8* lpImgBuf, quint32 &nSize, quint32 &nWidth, quint32 &nHeight);


/*-----------------------------------------------------------------------------
获取车牌、大图、车牌小图、二值化图等数据。
参数：
nIndex：数据序号；
PAutoDetectedVehInfo: 车牌识别结果结构
返回：
0-成功；
其它-失败；
-----------------------------------------------------------------------------*/
LIBVRR_YNSHARED_EXPORT int VPR_GetAllVehInfo (quint32 nIndex, CAutoDetectedVehInfo *pVehInfo);

/*-----------------------------------------------------------------------------
获取动态链接库的版本号和厂家信息。
参数：
lpszVer：动态链接库版本号，至少分配20个字节；
lpszVerder：厂商名称，至少分配50字节；
lpszDevType：设备型号，如果非空指针，可以写入50字节；
返回：
0-成功；
其它-失败；
-----------------------------------------------------------------------------*/
LIBVRR_YNSHARED_EXPORT int VPR_GetVer (char* lpszVer, char* lpszVender, char* lpszDevType);

#ifdef __cplusplus
}
#endif

#endif // LIBVPR_TEST_H
