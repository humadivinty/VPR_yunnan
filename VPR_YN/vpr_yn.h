#ifndef VPR_YN_H
#define VPR_YN_H

#include "vpr_yn_global.h"
#include"vpr_commondef_yn.h"
#include<windows.h>

// 云南省高速公路联网收费系统
// 车牌识别设备软件接口 V1.0

/*
附录
车牌颜色定义：
0-蓝牌；1-黄牌；2-黑牌；3-白牌；-1-无颜色

车牌回调方法定义：
TNotifyFunc= procedure(nIndex: LongWord); stdcall;

 TAutoDetectedVehInfo定义：
    TAutoDetectedVehInfo = packed record
     nColor: Integer ;	         	车牌颜色代码
     lpszPlate: PChar; 		     	车牌号
     nRating: LongWord ;		    车牌的可信度
     lpBigImgBuf: PBYTE ;	     	车辆大图图像数据缓冲区
     nBigImgSize: LongWord;     	车辆大图图像大小
     lpSmallImgBuf: PBYTE ;     	车牌小图图像数据缓冲区
     nSmallImgSize: LongWord;   	车牌小图图像大小
     lpBinImgBuf: PBYTE ;		   	二值化图像数据缓冲区
     nBinImgSize: LongWord ;		二值化图像数据大小
     nBinImgWidth: LongWord ;   	二值化图像宽度
     nBinImgHeight: LongWord ;	 	二值化图像高度
  end;
 */
#ifdef __cplusplus
extern "C"{
#endif

/* 说明：注册窗口句柄和自定义消息；
参数：hHandle 目标窗口句柄，即hHandle参数
nDataMsgID 自定义消息参数ID
返回： 0 注册消息成功；
      -1 注册消息失败。 */
VPR_YNSHARED_EXPORT int YN_DECL VPR_RegisterMessage(HANDLE hHandle,int  nDataMsgID);

/*
说明：注册车牌识别事件回调函数；
参数：TNotifyFunc notifyFunc消息回调函数，详见[附录]；
    返回：
0  注册消息回调成功；
-1 注册消息回调失败。
 */
 VPR_YNSHARED_EXPORT int  YN_DECL  VPR_RegisterCallback (TNotifyFunc notifyFunc);

/*  说明：连接车牌识别系统；
参数：char * lpszConnPara1 连接参数，一般为车牌识别设备的IP；
char * lpszConnPara2 备用参数
返回：0  连接车牌识别设备成功；
-1 连接车牌识别设备失败； */
  VPR_YNSHARED_EXPORT int   YN_DECL  VPR_Connect (char * lpszConnPara1, char * lpszConnPara2);

/* 说明：断开与车牌识别系统的连接；
参数：无；
返回： 0  成功；
       -1 失败。 */
  VPR_YNSHARED_EXPORT int   YN_DECL  VPR_Disconnect (void);


/*  说明：主动触发车牌识别系统抓怕图像并识别车牌，函数为非阻塞方式，调用后立即返回，抓拍识别工作完成并准备好数据后通过消息或回调的方式通知上层应用；
参数：无；
返回： 0 主动抓拍成功；
      -1 主动抓拍失败。 */
  VPR_YNSHARED_EXPORT int   YN_DECL VPR_Capture (void);

/*
 说明：获取指定序号车牌颜色、车牌号；
参数： nIndex：数据序号；
nColor: 车牌颜色代码，详见[附录]；
lpszPlate：车牌号，如未识别出车牌或无车牌输出零长度字符串；
nRating：车牌的可信度，为整数，例如nRating=85，表示可信度为85%。
返回： 0 获取车牌成功；
      -1 获取车牌失败。
*/
  VPR_YNSHARED_EXPORT int   YN_DECL  VPR_GetPlate (int nIndex, int * nColor, char *  lpszPlate, int * nRating);

/*  说明：获取指定序号的车辆大图；
参数：nIndex：数据序号；
lpImgBuf：图像数据缓冲区，缓冲区大小必须≥200K;
nSize： 输入时为上层应用分配的缓冲区大小，输出时为图像实际大小；如图像大小大于缓冲区大小，也通过该参数输出图像实际大小。
返回：
0-成功；
1-图像大小大于缓冲区
2-无图像
其它-失败；
 */
  VPR_YNSHARED_EXPORT int  YN_DECL VPR_GetBigImg (int nIndex, BYTE * lpImgBuf, int * nSize);

/*
 说明：获取指定序号的车牌小图；
参数：nIndex：数据序号；
lpImgBuf：图像数据缓冲区，缓冲区大小必须≥20K;
nSize： 输入时为上层应用分配的缓冲区大小，输出时为图像实际大小；如图像大小大于缓冲区大小，也通过该参数输出图像实际大小。
返回：0-成功；
1-图像大小大于缓冲区
2-无图像
其它-失败；
 */
  VPR_YNSHARED_EXPORT int  YN_DECL VPR_GetSmallImg (int nIndex, BYTE * lpImgBuf, int * nSize);

 /*
 说明：获取指定序号的车牌二值图；
参数： nIndex：数据序号；
lpImgBuf：图像数据缓冲区，缓冲区大小必须≥280字节;
nWeidth：输出的二值图像宽度；
nHeight：输出的二值图像高度；
nSize： 输入时为上层应用分配的缓冲区大小，输出时为图像实际大小；如图像大小大于缓冲区大小，也通过该参数输出图像实际大小。
返回：0-成功；
1-图像大小大于缓冲区
2-无图像
其它-失败；
 */
  VPR_YNSHARED_EXPORT int  YN_DECL VPR_GetBinImg (int nIndex, BYTE * lpImgBuf, int *  nSize, int * nWidth, int * nHeight);

/*
 说明：获取车牌、大图、车牌小图、二值化图等数据；
参数：int nIndex：数据序号；
TAutoDetectedVehInfo * VehInfo: 车牌识别结果结构体，详见[附录]
返回：0   成功；
-1  失败；
*/
 VPR_YNSHARED_EXPORT int  YN_DECL VPR_GetAllVehInfo(int nIndex, TAutoDetectedVehInfo * VehInfo);

/*
说明：获取动态链接库的版本号和厂家信息；
参数：lpszVer：动态链接库版本号，至少分配20个字节；
lpszVerder：厂商名称，至少分配50字节；
lpszDevType：设备型号，如果非空指针，可以写入50字节；
返回：0  成功；
      -1 失败；
       */
  VPR_YNSHARED_EXPORT int   YN_DECL VPR_GetVer (char * lpszVer, char * lpszVender, char * lpszDevType);

#ifdef __cplusplus
}
#endif

#endif // VPR_YN_H
