#ifndef VPR_COMMONDEF_YN_H
#define VPR_COMMONDEF_YN_H

#define YN_DECL _stdcall

typedef  void (YN_DECL *TNotifyFunc)(unsigned long);

//#pragma pack(push)
//#pragma pack(1)
typedef struct
{
    int nColor ;                                            //车牌颜色代码
    char* lpszPlate;                                    //车牌号
    unsigned long nRating ;                      //车牌的可信度
    unsigned char* lpBigImgBuf ;            //车辆大图图像数据缓冲区
    unsigned long nBigImgSize;              //车辆大图图像大小
    unsigned char* lpSmallImgBuf ;        //车牌小图图像数据缓冲区
    unsigned long nSmallImgSize;          //车牌小图图像大小
    unsigned char* lpBinImgBuf ;            //二值化图像数据缓冲区
    unsigned long nBinImgSize ;             //二值化图像数据大小
    unsigned long nBinImgWidth ;         //二值化图像宽度
    unsigned long nBinImgHeight ;       //二值化图像高度
}TAutoDetectedVehInfo;
//#pragma pack(pop)


#endif // VPR_COMMONDEF_YN_H
