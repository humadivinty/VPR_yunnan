#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QMessageBox>
#include<QDebug>
#include<QDir>

#include"libvrr_yn.h"
#include"vprevent.h"

#define MAX_IMG_SIZE 5*1024*1024
#define CUS_EVENT_TYPE (1000 + 2)
void Func_NotifyFunc(quint32 uIndex);


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_iMsgID(0),
    m_iCurIndex(-1)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_Connect_clicked()
{
    QString qstrConPara1, qstrConPara2;
    qstrConPara1 = ui->lineEdit->text();
    qstrConPara2 = ui->lineEdit_2->text();
    QMessageBox msgbox;
    if(qstrConPara1.isEmpty())
    {
       msgbox.setText(tr("ConPara1 can not be null, please input the valid value."));
    }
    else
    {
        //int iRet = 0;
        int iRet = VPR_Connect((char*)(qstrConPara1.toStdString().c_str()), (char*)(qstrConPara2.toStdString().c_str()) );
        QString qstrLog = QString("VPR_Connect (%1, %2) = %3").arg(qstrConPara1).arg(qstrConPara2).arg(iRet);
        msgbox.setText(qstrLog);
    }

    msgbox.exec();
}

void MainWindow::on_pushButton_DisConnect_clicked()
{
    QMessageBox msgbox;
    int iRet = VPR_Disconnect();
    QString qstrLog = QString("VPR_Disconnect () = %1").arg(iRet);
    msgbox.setText(qstrLog);

    msgbox.exec();
}

void MainWindow::on_pushButton_RegisterMessage_clicked()
{
    //QMessageBox msgbox;
    int iRet = VPR_RegisterMessage(this, m_iMsgID);
    QString qstrLog;
    qstrLog.sprintf("RegisterMessage (%p, %d) = %d",this,  m_iMsgID, iRet);
    //msgbox.setText(qstrLog);

    qDebug()<<qstrLog;
    //msgbox.exec();
}

void MainWindow::on_pushButton_VPR_RegisterCallback_clicked()
{
    QMessageBox msgbox;
    int iRet = VPR_RegisterCallback(Func_NotifyFunc);
    QString qstrLog;
    qstrLog.sprintf("VPR_RegisterCallback (%p) = %d",Func_NotifyFunc, iRet);
    msgbox.setText(qstrLog);

    msgbox.exec();
}

void MainWindow::on_pushButton_Capture_clicked()
{
    QMessageBox msgbox;
    int iRet = VPR_Capture();
    QString qstrLog;
    qstrLog.sprintf("VPR_Capture () = %d",iRet);
    msgbox.setText(qstrLog);

    msgbox.exec();
}

void MainWindow::on_pushButton_GetPlate_clicked()
{
    //QMessageBox msgbox;

    QString qstrPlateNo;
    quint32 iIndex = 0, iRating = 0;
    int iPlateColor = 0;

    iIndex = ui->lineEdit_Index->text().toInt();
    char chPlateNo[64] = {0};
    int iRet = VPR_GetPlate(iIndex, iPlateColor, chPlateNo, iRating);
    QString qstrLog;
    qstrPlateNo = QString::fromLocal8Bit(chPlateNo);
    //qstrLog.sprintf("VPR_GetPlate (%u, %d, %s, %u) = %d",iIndex,iPlateColor, qstrPlateNo, iRating, iRet) ;
    qstrLog = QString("VPR_GetPlate (%1, %2, %3, %4) = %5").arg(iIndex).arg(iPlateColor).arg(qstrPlateNo).arg(iRating).arg(iRet);
    //msgbox.setText(qstrLog);
    qDebug()<<qstrLog;

    QMessageBox msgbox;
    msgbox.setText(qstrLog);
    msgbox.exec();
}

void MainWindow::on_pushButton_GetBigImg_clicked()
{
    QString qstrLog;
    quint32 iIndex = 0;

    int iRet = 1;
    iIndex = ui->lineEdit_Index->text().toInt();
    unsigned char* pImg = new unsigned char[MAX_IMG_SIZE];
    quint32 iBuferSize = MAX_IMG_SIZE;
    if(pImg)
    {
        memset(pImg, 0, MAX_IMG_SIZE);
        iRet = VPR_GetBigImg(iIndex, pImg, iBuferSize);
        qstrLog = QString("VPR_GetBigImg(%1, buffer , %2) = %3").arg(iIndex).arg(iBuferSize).arg(iRet);

        if(iRet == 0)
        {
            QString imgPath = QString("%1/result/%2_%3_big.jpg")
                    .arg(QDir::currentPath())
                    .arg(QDate::currentDate().toString("yyyyMMdd"))
                    .arg(QTime::currentTime().toString("hh_mm_ss_zzz"));
            SaveBuferToFile(imgPath, pImg, iBuferSize );
        }
    }
    else
    {
        qstrLog = QString("alloc memory failed.");
    }

    if(pImg)
    {
        delete[] pImg;
        pImg = NULL;
    }
    qDebug()<<qstrLog;
    QMessageBox msgbox;
    msgbox.setText(qstrLog);
    msgbox.exec();
}

void MainWindow::on_pushButton_GetSmallImg_clicked()
{
    QString qstrLog;
    quint32 iIndex = 0;

    int iRet = 1;
    iIndex = ui->lineEdit_Index->text().toInt();
    unsigned char* pImg = new unsigned char[MAX_IMG_SIZE];
    quint32 iBuferSize = MAX_IMG_SIZE;
    if(pImg)
    {
        memset(pImg, 0, MAX_IMG_SIZE);
        iRet = VPR_GetSmallImg(iIndex, pImg, iBuferSize);
        qstrLog = QString("VPR_GetSmallImg(%1, buffer , %2) = %3").arg(iIndex).arg(iBuferSize).arg(iRet);

        if(iRet == 0)
        {
            QString imgPath = QString("%1/result/%2_%3_small.bmp")
                    .arg(QDir::currentPath())
                    .arg(QDate::currentDate().toString("yyyyMMdd"))
                    .arg(QTime::currentTime().toString("hh_mm_ss_zzz"));
            SaveBuferToFile(imgPath, pImg, iBuferSize );
        }
    }
    else
    {
        qstrLog = QString("alloc memory failed.");
    }

    if(pImg)
    {
        delete[] pImg;
        pImg = NULL;
    }
    qDebug()<<qstrLog;
    QMessageBox msgbox;
    msgbox.setText(qstrLog);
    msgbox.exec();
}

void MainWindow::on_pushButton_GetBinImg_clicked()
{
    QString qstrLog;
    quint32 iIndex = 0;
    quint32 iwidth = 0;
    quint32 iHeight = 0;

    int iRet = 1;
    iIndex = ui->lineEdit_Index->text().toInt();
    unsigned char* pImg = new unsigned char[MAX_IMG_SIZE];
    quint32 iBuferSize = MAX_IMG_SIZE;
    if(pImg)
    {
        memset(pImg, 0, MAX_IMG_SIZE);
        iRet = VPR_GetBinImg(iIndex, pImg, iBuferSize, iwidth, iHeight);
        qstrLog = QString("VPR_GetBinImg(%1, buffer , %2, %3, %4) = %5").arg(iIndex).arg(iBuferSize).arg(iwidth).arg(iHeight).arg(iRet);

        if(iRet == 0)
        {
            QString imgPath = QString("%1/result/%2_%3_bin.bin")
                    .arg(QDir::currentPath())
                    .arg(QDate::currentDate().toString("yyyyMMdd"))
                    .arg(QTime::currentTime().toString("hh_mm_ss_zzz"));
            SaveBuferToFile(imgPath, pImg, iBuferSize );
        }
    }
    else
    {
        qstrLog = QString("alloc memory failed.");
    }

    if(pImg)
    {
        delete[] pImg;
        pImg = NULL;
    }
    qDebug()<<qstrLog;
    QMessageBox msgbox;
    msgbox.setText(qstrLog);
    msgbox.exec();
}

void MainWindow::on_pushButton_GetAllInfo_clicked()
{
    char chPlateNo[64] = {0};
    CAutoDetectedVehInfo VehInfo;
    VehInfo.nColor = 0;
    VehInfo.lpszPlate = chPlateNo;
    VehInfo.nRating = 0;
    VehInfo.lpBigImgBuf = new unsigned char[MAX_IMG_SIZE];			//	车辆大图图像数据缓冲区
    if(VehInfo.lpBigImgBuf)
    {
        memset(VehInfo.lpBigImgBuf, 0, MAX_IMG_SIZE);
        VehInfo.nBigImgSize = MAX_IMG_SIZE;
    }

    VehInfo.lpSmallImgBuf = new unsigned char[MAX_IMG_SIZE];			//	车辆小图图像数据缓冲区
    if(VehInfo.lpSmallImgBuf)
    {
        memset(VehInfo.lpSmallImgBuf, 0, MAX_IMG_SIZE);
        VehInfo.nSmallImgSize = MAX_IMG_SIZE;
    }

    VehInfo.lpBinImgBuf = new unsigned char[MAX_IMG_SIZE];			//	车辆二值化图图像数据缓冲区
    if(VehInfo.lpBinImgBuf)
    {
        memset(VehInfo.lpBinImgBuf, 0, MAX_IMG_SIZE);
        VehInfo.nBinImgSize = MAX_IMG_SIZE;
    }
    VehInfo.nBinImgWidth = 0;
    VehInfo.nBinImgHeight = 0;

    QString qstrLog;
    quint32 iIndex = 0;
    iIndex = ui->lineEdit_Index->text().toInt();
    int iRet = VPR_GetAllVehInfo(iIndex, &VehInfo);
    if(iRet == 0)
    {
        QString imgPath = QString("%1/result_GetAllInfo/%2_%3_")
                .arg(QDir::currentPath())
                .arg(QDate::currentDate().toString("yyyyMMdd"))
                .arg(QTime::currentTime().toString("hh_mm_ss_zzz"));

        QString strBigImgPath = QString(imgPath).append("big.jpg");
        QString strSmallImgPath = QString(imgPath).append("small.bmp");
        QString strBinImgPath = QString(imgPath).append("bin.bin");
        QString strPlateInfoPath = QString(imgPath).append("plate.txt");

        if(VehInfo.nBigImgSize > 0)
        {
            SaveBuferToFile(strBigImgPath, VehInfo.lpBigImgBuf , VehInfo.nBigImgSize );
        }

        if(VehInfo.nSmallImgSize > 0)
        {
            SaveBuferToFile(strSmallImgPath, VehInfo.lpSmallImgBuf , VehInfo.nSmallImgSize );
        }

        if(VehInfo.nBinImgSize > 0)
        {
            SaveBuferToFile(strBinImgPath, VehInfo.lpBinImgBuf , VehInfo.nBinImgSize );
        }

        QString qstrPlateInfo = QString("PlateNo: %1\n"
                                        "PlateColor: %2 \n"
                                        "Rating: %3\n"
                                        "Big ImageSize: %4\n"
                                        "Small ImageSize: %5\n"
                                        "Bin ImageSize: %6\n"
                                        "Bin image width: %7\n"
                                        "Bin image height: %8\n").arg(QString::fromLocal8Bit(VehInfo.lpszPlate))
                .arg(VehInfo.nColor )
                .arg(VehInfo.nRating)
                .arg(VehInfo.nBigImgSize)
                .arg(VehInfo.nSmallImgSize)
                .arg(VehInfo.nBinImgSize)
                .arg(VehInfo.nBinImgWidth)
                .arg(VehInfo.nBinImgHeight);
        //SaveBuferToFile(strPlateInfoPath, (void*)(qstrPlateInfo.toStdString().c_str()), qstrPlateInfo.toStdString().length() );
        SaveTextToFile(strPlateInfoPath, qstrPlateInfo);
    }
    qstrLog = QString("VPR_GetAllVehInfo return code = %1").arg(iRet);

    if(VehInfo.lpBigImgBuf)
    {
        delete[] VehInfo.lpBigImgBuf;
        VehInfo.lpBigImgBuf = NULL;
    }

    if(VehInfo.lpSmallImgBuf)
    {
        delete[] VehInfo.lpSmallImgBuf;
        VehInfo.lpSmallImgBuf = NULL;
    }

    if(VehInfo.lpBinImgBuf)
    {
        delete[] VehInfo.lpBinImgBuf;
        VehInfo.lpBinImgBuf = NULL;
    }

    qstrLog = QString("VPR_GetAllVehInfo (%1) = %2").arg(iIndex).arg(iRet);
    //msgbox.setText(qstrLog);
    qDebug()<<qstrLog;
    QMessageBox msgbox;
    msgbox.setText(qstrLog);
    msgbox.exec();
}

void MainWindow::on_pushButton_GetVer_clicked()
{
    char chVer[256] = {0};
    char chVender[256] = {0};
    char chDevType[256] = {0};

    int iRet = VPR_GetVer(chVer, chVender, chDevType);
    QString qstrLog;

    qstrLog = QString("VPR_GetVer (%1, %2, %3) = %4").arg(chVer).arg(chVender).arg(chDevType).arg(iRet);
    //msgbox.setText(qstrLog);
    qDebug()<<qstrLog;
    QMessageBox msgbox;
    msgbox.setText(qstrLog);
    msgbox.exec();
}

void Func_NotifyFunc(quint32 uIndex)
{
    qDebug()<<"Func_NotifyFunc, index = "<<uIndex;
}

void MainWindow::SaveBuferToFile(QString FilePath, void *bufer, int size)
{
    QString qstrPath(FilePath);
    QDir tempDir;
    //QString qstrCurrentPath = QDir::currentPath();
    int iLastcharIndex = qstrPath.lastIndexOf("\\");
    if(iLastcharIndex == -1)
    {
        iLastcharIndex = qstrPath.lastIndexOf("/");
    }
    iLastcharIndex = (iLastcharIndex == -1) ? qstrPath.length() : iLastcharIndex;
    QString qstrCurrentPath = qstrPath.mid(0, iLastcharIndex);
    //qstrCurrentPath.append("//SNW_log");
    if(!tempDir.exists(qstrCurrentPath))
    {
        tempDir.mkdir(qstrCurrentPath);
    }
//    QDate currentDate = QDate::currentDate();
//    QTime currentTime = QTime::currentTime();

    FILE* pFile = fopen(FilePath.toStdString().c_str(), "wb+");
    if(pFile)
    {
        fwrite(bufer, size, 1, pFile);
        fclose(pFile);
        pFile = NULL;
    }
}

void MainWindow::SaveTextToFile(QString path, QString text)
{
    QString qstrPath(path);
    QDir tempDir;
    //QString qstrCurrentPath = QDir::currentPath();
    int iLastcharIndex = qstrPath.lastIndexOf("\\");
    if(iLastcharIndex == -1)
    {
        iLastcharIndex = qstrPath.lastIndexOf("/");
    }
    iLastcharIndex = (iLastcharIndex == -1) ? qstrPath.length() : iLastcharIndex;
    QString qstrCurrentPath = qstrPath.mid(0, iLastcharIndex);
    //qstrCurrentPath.append("//SNW_log");
    if(!tempDir.exists(qstrCurrentPath))
    {
        tempDir.mkdir(qstrCurrentPath);
    }

    QFile InfFile(path);
    if(!InfFile.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug()<<"open log File failed";
        return ;
    }

    QTextStream qStream(&InfFile);
    qStream<<text<<endl;

    InfFile.close();
}

void MainWindow::ProcessResultByIndex(int index)
{
    qDebug()<<"Receive msg index = "<<index;    

    ui->lineEdit_Index->setText(QString("%1").arg(index));
    quint32  iRating = 0;
    int iPlateColor = 0;
    int iIndex = index;

    QString imgPath = QString("%1/result/%2_%3_")
            .arg(QDir::currentPath())
            .arg(QDate::currentDate().toString("yyyyMMdd"))
            .arg(QTime::currentTime().toString("hh_mm_ss_zzz"));

    QString strBigImgPath = QString(imgPath).append("big.jpg");
    QString strSmallImgPath = QString(imgPath).append("small.bmp");
    QString strBinImgPath = QString(imgPath).append("bin.bin");
    QString strPlateInfoPath = QString(imgPath).append("plate.txt");

    //GePlateNo
    qDebug()<<"GePlateNo";
    QString qstrPlateNo;
    char chPlateNo[64] = {0};
    int iRet = VPR_GetPlate(iIndex, iPlateColor, chPlateNo, iRating);
    QString qstrLog;
    qstrPlateNo = QString::fromLocal8Bit(chPlateNo);
    //qstrLog.sprintf("VPR_GetPlate (%u, %d, %s, %u) = %d",iIndex,iPlateColor, qstrPlateNo, iRating, iRet) ;
    qstrLog = QString("VPR_GetPlate (%1, %2, %3, %4) = %5").arg(iIndex).arg(iPlateColor).arg(qstrPlateNo).arg(iRating).arg(iRet);
    //msgbox.setText(qstrLog);
    qDebug()<<qstrLog;
    //SaveBuferToFile(strPlateInfoPath, (void*)(qstrLog.toStdString().c_str()), qstrLog.toStdString().length() );
    SaveTextToFile(strPlateInfoPath, qstrLog);    


    //GetBigImg
    qDebug()<<"GetBigImg";
    unsigned char* pImg = new unsigned char[MAX_IMG_SIZE];
    quint32 iBuferSize = MAX_IMG_SIZE;
    if(pImg)
    {
        memset(pImg, 0, MAX_IMG_SIZE);
        iRet = VPR_GetBigImg(iIndex, pImg, iBuferSize);
        qstrLog = QString("VPR_GetBigImg(%1, buffer , %2) = %3").arg(iIndex).arg(iBuferSize).arg(iRet);

        if(iRet == 0 && iBuferSize > 0)
        {
            QString imgPath = QString("%1/result/%2_%3_big.jpg")
                    .arg(QDir::currentPath())
                    .arg(QDate::currentDate().toString("yyyyMMdd"))
                    .arg(QTime::currentTime().toString("hh_mm_ss_zzz"));
            SaveBuferToFile(strBigImgPath, pImg, iBuferSize );
        }
    }
    else
    {
        qstrLog = QString("alloc memory failed.");
    }
    qDebug()<<qstrLog;

    //GetSmallImg
    qDebug()<<"GetSmallImg";
    quint32 iwidth = 0;
    quint32 iHeight = 0;
    iBuferSize = MAX_IMG_SIZE;
    if(pImg)
    {
        memset(pImg, 0, MAX_IMG_SIZE);
        iRet = VPR_GetSmallImg(iIndex, pImg, iBuferSize);
        qstrLog = QString("VPR_GetSmallImg(%1, buffer , %2) = %3").arg(iIndex).arg(iBuferSize).arg(iRet);

        if(iRet == 0 && iBuferSize > 0)
        {
            SaveBuferToFile(strSmallImgPath, pImg, iBuferSize );
        }
    }
    else
    {
        qstrLog = QString("alloc memory failed.");
    }
    qDebug()<<qstrLog;

    //GetBinImage
    qDebug()<<"GetBinImage";
    iBuferSize = MAX_IMG_SIZE;
    if(pImg)
    {
        memset(pImg, 0, MAX_IMG_SIZE);
        iRet = VPR_GetBinImg(iIndex, pImg, iBuferSize, iwidth, iHeight);
        qstrLog = QString("VPR_GetBinImg(%1, buffer , %2, %3, %4) = %5").arg(iIndex).arg(iBuferSize).arg(iwidth).arg(iHeight).arg(iRet);

        if(iRet == 0 && iBuferSize > 0)
        {
            SaveBuferToFile(strBinImgPath, pImg, iBuferSize );
        }
    }
    else
    {
        qstrLog = QString("alloc memory failed.");
    }
    qDebug()<<qstrLog;

    if(pImg)
    {
        delete[] pImg;
        pImg = NULL;
    }
}

bool MainWindow::event(QEvent *event)
{
    if(event->type() == CUS_EVENT_TYPE)
    {
        VPREVENT* vprEvent = static_cast<VPREVENT *>(event);
        if(m_iMsgID == vprEvent->m_nDataMsgId)
        {
            ProcessResultByIndex(vprEvent->m_nIndex);
            return true;
        }
    }
    return QMainWindow::event(event);
}
