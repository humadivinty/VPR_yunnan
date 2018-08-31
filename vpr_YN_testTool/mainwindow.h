#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_Connect_clicked();

    void on_pushButton_DisConnect_clicked();

    void on_pushButton_RegisterMessage_clicked();

    void on_pushButton_VPR_RegisterCallback_clicked();

    void on_pushButton_Capture_clicked();

    void on_pushButton_GetPlate_clicked();

    void on_pushButton_GetBigImg_clicked();

    void on_pushButton_GetSmallImg_clicked();

    void on_pushButton_GetBinImg_clicked();

    void on_pushButton_GetAllInfo_clicked();

    void on_pushButton_GetVer_clicked();

public:
   // void Func_NotifyFunc(quint32 uIndex);
    void SaveBuferToFile(QString FilePath, void* bufer, int size);
    void SaveTextToFile(QString path, QString text);
    void ProcessResultByIndex(int index);
private:
    Ui::MainWindow *ui;    
    quint32 m_iMsgID;
    qint32 m_iCurIndex;

private:
    bool event(QEvent *event);
};

#endif // MAINWINDOW_H
