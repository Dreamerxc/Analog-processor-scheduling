#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QString>
#include<QTimer>
#include<QVector>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef struct PCB
{
    QString pid;   //进程名
    int runtime;    //要求运行时间
    int donetime;   //已运行时间
    int priority;   //优先权
    int status;   //状态
    QString attributes;  //属性,如果属性为“”，则为独立进程，否则为同步进程，有前驱
    int peace;   //所需主存大小
    bool operator<(const PCB& a)const
    {
        return (priority > a.priority) || (priority == a.priority && runtime-donetime > a.runtime-a.donetime);
    }
}PCB;

typedef struct _BiTNode  //定义表的结构体
{
    QString pid;
    int address;	//首地址
    int length;		//空闲区长度
    bool state;		//区域状态 “1”表示已分配， “0”表示未分配
    QString homework;		//作业名
    struct _BiTNode* lchild;	//指向左孩子的指针
    struct _BiTNode* rchild;	//右孩子

    _BiTNode()
    {
        pid = "";
        address = 0;
        length = 0;
        state = 0;
        homework = "";
        lchild = nullptr;
        rchild = nullptr;
    }
}BiTNode, * pBiTree;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void showprocess();                      //刷新进程表
    void showpeace(pBiTree);             //刷新主存分区表
    void Init_BiTree(pBiTree);               //初始化分区表程序
    void process_update();                   //每隔1s执行处理机调度程序
    void Delect_PreOrder(pBiTree,QString);   //改变结束进程在主存分区表的状态
    void recycle_1(pBiTree);                   //将可以合并的空闲分区合并
    void recycle_2(pBiTree);                   //将可以合并的空闲分区合并
    bool Insert_PreOrder(pBiTree, int, QString); //主存空间分配空间
    void inpeace();                            //从后备队列中选取进程进入就绪队列
    void haveshowqu(pBiTree,QVector<pBiTree>&);             //获得分区树上的分区空间
    void showouttable();                       //刷新外存进程表
    bool isready(PCB);
    bool isactive(int);                                //判断挂起态是否激活
    void showcpu();                                 //展示cpu状态
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pause_button_clicked();

    void on_start_button_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;
    QTimer *timer;

};
#endif // MAINWINDOW_H
