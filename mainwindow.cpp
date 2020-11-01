#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QVector>
#include<QDebug>
#include <QtAlgorithms>
#include <algorithm>


QVector<PCB> readyqu;   //就绪，阻塞队列
QVector<PCB> reservequ;   //后备队列
QVector<PCB> highqu;        //挂起队列
QVector<PCB> finishqu;     //完成队列
BiTNode l;              //指向分区表头的指针

//采用全局变量来记录cpu的状态
QString cpu1_status = "空闲";
QString cpu2_status = "空闲";


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Init_BiTree(&l);
    showpeace(&l);
    showprocess();
    showouttable();
    showcpu();
    timer = new QTimer(this);      //this 指定父对象，可自动回收
    connect(timer,&QTimer::timeout,this,&MainWindow::process_update);
    timer->start(1000);
}


MainWindow::~MainWindow()
{
    delete ui;
}


static bool cmp(PCB& a,PCB& b)
{
    return a.priority>b.priority||(a.priority==b.priority&&a.runtime-a.donetime > b.runtime-b.donetime);
}


void MainWindow::showcpu()
{
    ui->cpu_widget->clear();
    ui->cpu_widget->setWindowTitle("cpu状态");
    ui->cpu_widget->setColumnCount(2);
    ui->cpu_widget->setRowCount(1);

    /* 设置 tableWidget */

    ui->cpu_widget->setHorizontalHeaderLabels(QStringList() << "cpu1状态"<<"cpu2状态");
   // ui->cpu_widget->verticalHeader()->setVisible(); // 隐藏水平header
    ui->cpu_widget->setSelectionBehavior(QAbstractItemView::SelectItems);   // 单个选中
    ui->cpu_widget->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个

    QTableWidgetItem *item0 = new QTableWidgetItem(cpu1_status);
    QTableWidgetItem *item1 = new QTableWidgetItem(cpu2_status);

    ui->cpu_widget->setItem(0,0,item0);
    ui->cpu_widget->setItem(0,1,item1);

    ui->cpu_widget->show();
    cpu1_status = "空闲";
    cpu2_status = "空闲";
}

bool MainWindow::Insert_PreOrder(pBiTree p, int length, QString homework)  //插入新作业
{
    if (p != nullptr)
    {
        if ((p->state == 0) && (length <= p->length) && (p->lchild==nullptr) && (p->rchild==nullptr))//判断节点是否有左右孩子，是否小于空闲区长度，是否状态为0（表示区域无作业运行）
        {

            pBiTree  x1 = new BiTNode;//进行插入，动态申请两个空间 x1,x2
            pBiTree  x2 = new BiTNode;
            x1->homework = homework; //x1为作业
            x2->homework = ""; //x2为剩余的空闲区
            x1->address = p->address; //把原空闲区的首地址给作业的首地址
            x1->state = 1;  //作业的状态改为1
            x1->length = length; //作业的长度为传入的长度
            x2->address = x1->address + length;//新空闲区的首地址为 x1首地址与x1长度之和
            x2->length = p->length - x1->length;
            x2->state = 0;
            p->lchild = x1;
            p->rchild = x2;
            x1->lchild = nullptr;
            x1->rchild = nullptr;
            x2->rchild = nullptr;
            x2->lchild = nullptr;
            p->state = 1;
            return true;

        }
        else
        {
            return Insert_PreOrder(p->lchild, length, homework)||Insert_PreOrder(p->rchild, length, homework);
        }
     }
        return false;
}


void MainWindow::on_pushButton_clicked()
{

    PCB pcb;
    pcb.pid = ui->name->text();
    pcb.runtime = ui->runtime->text().toInt();
    pcb.donetime = 0;
    pcb.priority = ui->priority->text().toInt();
    pcb.status = 0;
    pcb.attributes = ui->attritube->text();
    pcb.peace = ui->size->text().toInt();
    ui->name->clear();
    ui->runtime->clear();
    ui->attritube->clear();
    ui->priority->clear();
    ui->size->clear();
    qDebug()<<"添加进程："<<pcb.pid;
    reservequ.append(pcb);                  //入外存的后备队列
    showouttable();
    showpeace(&l);
}


void MainWindow::on_pushButton_2_clicked()
{
    ui->name->clear();
    ui->runtime->clear();
    ui->attritube->clear();
    ui->priority->clear();
    ui->size->clear();
}

void MainWindow::showouttable()
{
    ui->outtablewidget->clear();
    ui->outtablewidget->setWindowTitle("外存队列");
    int n = readyqu.size();
    ui->outtablewidget->setColumnCount(5);
    ui->outtablewidget->setRowCount(n+5);

    /* 设置 tableWidget */

    ui->outtablewidget->setHorizontalHeaderLabels(QStringList() << "pid名称" << "需运行时间" << "已运行时间" << "优先权"<<"状态");
    ui->outtablewidget->verticalHeader()->setVisible(false); // 隐藏水平header
    ui->outtablewidget->setSelectionBehavior(QAbstractItemView::SelectItems);   // 单个选中
    ui->outtablewidget->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个

    int i = 0;
    for(;i<reservequ.size();i++)
    {
        auto pcb = reservequ[i];

        QTableWidgetItem *item0 = new QTableWidgetItem(pcb.pid);
        QTableWidgetItem *item2 = new QTableWidgetItem(QString::number(pcb.runtime - pcb.donetime, 10));
        QTableWidgetItem *item3 = new QTableWidgetItem(QString::number(pcb.donetime, 10));
        QTableWidgetItem *item4 = new QTableWidgetItem(QString::number(pcb.priority, 10));
        QString status = "新建/就绪";
        QTableWidgetItem *item5 = new QTableWidgetItem(status);

        ui->outtablewidget->setItem(i,0,item0);
        ui->outtablewidget->setItem(i,1,item2);
        ui->outtablewidget->setItem(i,2,item3);
        ui->outtablewidget->setItem(i,3,item4);
        ui->outtablewidget->setItem(i,4,item5);

    }

    int w = 0;
    for(int j = i;j<i + highqu.size();j++)
    {
        auto pcb = highqu[w];

        QTableWidgetItem *item0 = new QTableWidgetItem(pcb.pid);
        QTableWidgetItem *item2 = new QTableWidgetItem(QString::number(pcb.runtime - pcb.donetime, 10));
        QTableWidgetItem *item3 = new QTableWidgetItem(QString::number(pcb.donetime, 10));
        QTableWidgetItem *item4 = new QTableWidgetItem(QString::number(pcb.priority, 10));
        QString status = "挂起";
        QTableWidgetItem *item5 = new QTableWidgetItem(status);

        ui->outtablewidget->setItem(j,0,item0);
        ui->outtablewidget->setItem(j,1,item2);
        ui->outtablewidget->setItem(j,2,item3);
        ui->outtablewidget->setItem(j,3,item4);
        ui->outtablewidget->setItem(j,4,item5);

        w++;
    }
    ui->outtablewidget->show();
}

void MainWindow::showprocess()
{
    ui->tableWidget->clear();
    ui->tableWidget->setWindowTitle("就绪队列");
    int n = readyqu.size();
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setRowCount(n+5);

    /* 设置 tableWidget */

    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "pid名称" << "需运行时间" << "已运行时间" << "优先权"<<"状态");
    ui->tableWidget->verticalHeader()->setVisible(false); // 隐藏水平header
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);   // 单个选中
    ui->tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个
    for(int i = 0;i<readyqu.size();i++)
    {
        auto pcb = readyqu[i];

        QTableWidgetItem *item0 = new QTableWidgetItem(pcb.pid);
        QTableWidgetItem *item2 = new QTableWidgetItem(QString::number(pcb.runtime - pcb.donetime, 10));
        QTableWidgetItem *item3 = new QTableWidgetItem(QString::number(pcb.donetime, 10));
        QTableWidgetItem *item4 = new QTableWidgetItem(QString::number(pcb.priority, 10));
        QString status;
        if(pcb.status==0) status = "就绪";
        else if(pcb.status==1) status = "运行";
        else status = "阻塞";
        QTableWidgetItem *item5 = new QTableWidgetItem(status);

        ui->tableWidget->setItem(i,0,item0);
        ui->tableWidget->setItem(i,1,item2);
        ui->tableWidget->setItem(i,2,item3);
        ui->tableWidget->setItem(i,3,item4);
        ui->tableWidget->setItem(i,4,item5);

        readyqu[i].status = 0;               //将进程状态重置放在此处

    }
    ui->tableWidget->show();
}


void MainWindow::Init_BiTree(pBiTree l)  //分区表初始化
{
    l->address = 0;
    l->length = 500;
    l->state = 0;
    l->homework = "";
    l->lchild = nullptr;
    l->rchild = nullptr;
    qDebug()<<"初始化完成！";
}


void MainWindow::haveshowqu(pBiTree l,QVector<pBiTree>& bit)
{
    if(l!=nullptr)
    {
        if(l->lchild==nullptr&&l->rchild==nullptr)
            bit.append(l);
        if(l->lchild) haveshowqu(l->lchild,bit);
        if(l->rchild) haveshowqu(l->rchild,bit);
    }
}


void MainWindow::showpeace(pBiTree l)
{
    QVector<pBiTree> bit;
    haveshowqu(l,bit);
    ui->peacetable->clear();
    ui->peacetable->setWindowTitle("主存分区表");
    int n = bit.size();
    ui->peacetable->setColumnCount(5);
    ui->peacetable->setRowCount(n+5);
    /* 设置 tableWidget */
    ui->peacetable->setHorizontalHeaderLabels(QStringList() << "序号" <<"开始地址" << "结束地址" << "长度" <<"状态");
    ui->peacetable->verticalHeader()->setVisible(false); // 隐藏水平header
    ui->peacetable->setSelectionBehavior(QAbstractItemView::SelectItems);   // 单个选中
    ui->peacetable->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 可以选中多个

    for(int i = 0;i<bit.size();i++)
    {

        auto p = bit[i];
        QTableWidgetItem *item0 = new QTableWidgetItem(QString::number(i, 10));
        QTableWidgetItem *item1 = new QTableWidgetItem(QString::number(p->address, 10));
        QTableWidgetItem *item2 = new QTableWidgetItem(QString::number(p->address+p->length, 10));
        QTableWidgetItem *item3 = new QTableWidgetItem((QString::number(p->length, 10)));
        QString state;
        if(p->state==0) state = "空闲";
        else state = "使用";
        QTableWidgetItem *item4 = new QTableWidgetItem(state);

        ui->peacetable->setItem(i,0,item0);
        ui->peacetable->setItem(i,1,item1);
        ui->peacetable->setItem(i,2,item2);
        ui->peacetable->setItem(i,3,item3);
        ui->peacetable->setItem(i,4,item4);

    }
    ui->peacetable->show();
}


void MainWindow::recycle_1(pBiTree l)
{
    if (l!=nullptr&&l->lchild != nullptr)
    {
        //如果两个叶子节点都是空闲的，可以进行合并。
        if (l->lchild->lchild == nullptr && l->rchild->rchild == nullptr && l->lchild->state == 0 && l->rchild->state == 0)
        {
            l->state = 0;
            auto x = l->lchild;
            auto y = l->rchild;
            l->lchild = nullptr;
            l->rchild = nullptr;
            delete x;
            delete y;
        }
        recycle_1(l->lchild);
        recycle_1(l->rchild);
    }
}

void MainWindow::recycle_2(pBiTree l)
{
    if(l->lchild&&l->rchild->lchild)
    {
        if(l->lchild->state==0&&l->rchild->lchild->state==0)
        {
            l->lchild->length = l->rchild->rchild->address;
            l->rchild = l->rchild->rchild;
        }
        recycle_2(l->lchild);
        recycle_2(l->rchild);
    }
}

void MainWindow::Delect_PreOrder(pBiTree p,QString homework)
{
    if (p != nullptr)
        {
            if (p->homework == homework)//当查询到作业名相同的作业。进行修改
            {
                p->homework = "";//将作业名修改为空
                p->state = 0;//将作业名状态修改为空闲
            }
            Delect_PreOrder(p->lchild, homework);  //遍历左子树
            Delect_PreOrder(p->rchild, homework); //遍历右子树
        }
}

void MainWindow::inpeace()
{

    for(int i = 0;i<reservequ.size();i++)
    {
        if(Insert_PreOrder(&l,reservequ[i].peace,reservequ[i].pid))   //如果进程可以放入主存空间，就将就绪队列中的进程放入主存中去运行
        {
            readyqu.append(reservequ[i]);
            reservequ.remove(i,1);
            i--;
        }
    }
}


bool MainWindow::isready(PCB pcb)
{
    if(pcb.attributes=="") return true;
    for(auto x:finishqu)
    {
        if(x.pid==pcb.attributes)         //如果它所需资源已经释放，则可以进入执行状态
            return true;
    }
    return false;
}

bool MainWindow::isactive(int j)
{
    for(auto x:finishqu)
    {
        if(x.pid==highqu[j].attributes)         //如果它所需资源已经释放，则可以进入执行状态
            return true;
    }
    return false;
}


void MainWindow::process_update()
{  
    std::sort(reservequ.begin(),reservequ.end(),cmp);
    inpeace();
    showpeace(&l);
    showprocess();
    if (!readyqu.empty())  //如果就绪队列不为空
    {       
        int flag1 = 0,flag2 = 0;
        int i = 0;
        while(flag1==0&&i<readyqu.size())
        {
            while(i<readyqu.size()&&readyqu[i].status!=0)         //先跳过主存中阻塞的进程
            {
                i++;

            }
            if(i>=readyqu.size()) break;
            if(readyqu[i].status==0&&isready(readyqu[i])&&i<readyqu.size())      //可以执行的进程
            {
                auto pcb = readyqu[i];
                cpu1_status = pcb.pid;
                readyqu.remove(i,1);
                pcb.status = 1;                    //状态转为运行态
                pcb.donetime++;                   //经过一个时间片，已运行时间加1。
                pcb.priority++;                   //进程优先权加1。
                if (pcb.donetime == pcb.runtime)    //如果该进程执行结束，将该进程送入完成队列中
                {
                    QString name = pcb.pid;
                    finishqu.append(pcb);
                     qDebug()<<"回收的主存空间"<<pcb.pid;
                    Delect_PreOrder(&l,pcb.pid);
                    recycle_1(&l);
                    recycle_2(&l);
                    for(int j = 0;j<highqu.size();j++)   //如果资源释放，就自动解挂
                    {
                        if(isactive(j))
                        {
                            highqu[j].status = 0;
                            reservequ.append(highqu[j]);
                            highqu.remove(j,1);
                            break;
                        }
                        qDebug()<<"xx4";
                    }
                }
                else readyqu.append(pcb);            //如果没有执行结束，返回就绪队列

                flag1 = 1;
            }
            else                                     //阻塞的同步进程
            {
                readyqu[i].status = 2;         //将该进程状态置2，阻塞态
            }
        }

        if(flag1==0&&!readyqu.empty()&&!reservequ.empty())  //如果主存中全为阻塞态，并且后备队列存在未执行进程
        {
            //选择主存队列中的第一个进程换出
            //先将换出进程置挂起态
            readyqu[0].status = 3;
            //将主存中的阻塞队列置挂起态
            highqu.append(readyqu[0]);
            //先考虑挂起队列，再考虑后备队列
            Delect_PreOrder(&l,readyqu[0].pid);
            recycle_1(&l);
            recycle_2(&l);
            readyqu.pop_front();
            inpeace();
        }
        i = 0;
        while(flag2==0&&i<readyqu.size())
        {

            while(i<readyqu.size()&&readyqu[i].status!=0)         //先跳过主存中阻塞的进程
            {
                i++;
            }

            if(i>=readyqu.size()) break;
            if(i<readyqu.size()&&readyqu[i].status==0&&isready(readyqu[i]))      //可以执行的进程
            {
                auto pcb = readyqu[i];
                cpu2_status = pcb.pid;
                readyqu.remove(i,1);
                pcb.status = 1;                    //状态转为运行态
                pcb.donetime++;                   //经过一个时间片，已运行时间加1。
                pcb.priority++;                   //进程优先权加1。
                if (pcb.donetime == pcb.runtime)    //如果该进程执行结束，将该进程送入完成队列中
                {
                    QString name = pcb.pid;
                    finishqu.append(pcb);
                    qDebug()<<"回收的主存空间"<<pcb.pid;
                    Delect_PreOrder(&l,pcb.pid);
                    recycle_1(&l);
                    recycle_2(&l);
                    for(int j = 0;j<highqu.size();j++)       //如果资源释放，就自动解挂
                    {
                        if(isactive(j))
                        {
                            highqu[j].status = 0;
                            reservequ.append(highqu[j]);
                            highqu.remove(j,1);
                            break;
                        }
                    }
                }
                else readyqu.append(pcb);            //如果没有执行结束，返回就绪队列
                flag2 = 1;
            }
            else                                     //阻塞的同步进程
            {
                readyqu[i].status = 2;         //将该进程状态置2，阻塞态
            }
        }
        if(flag2==0&&readyqu.size()>1&&!reservequ.empty())  //如果主存中全为阻塞态，并且后备队列存在未执行进程
        {
            //选择主存队列中的第一个进程换出
            //先将换出进程置挂起态
            readyqu[0].status = 3;
            //将主存中的阻塞队列置挂起态
            highqu.append(readyqu[0]);
            //先考虑挂起队列，再考虑后备队列
            Delect_PreOrder(&l,readyqu[0].pid);
            recycle_1(&l);
            recycle_2(&l);
            readyqu.pop_front();
            inpeace();
        }
    }
    std::sort(readyqu.begin(),readyqu.end(),cmp);
    recycle_1(&l);
    recycle_2(&l);
    showpeace(&l);
    showprocess();
    showouttable();
    showcpu();
}

void MainWindow::on_pause_button_clicked()
{
    timer->stop();
    qDebug()<<"暂停成功";
}

void MainWindow::on_start_button_clicked()
{
    timer->start();
    qDebug()<<"取消暂停成功";
}

void MainWindow::on_pushButton_3_clicked()
{
    int flag = 0;
    QString name = ui->hang_pcb->text();
    for(int i = 0;i<readyqu.size();i++)
    {
        if(readyqu[i].pid==name)
        {
            flag = 1;
            readyqu[i].status = 3;
            highqu.append(readyqu[i]);
            Delect_PreOrder(&l,readyqu[i].pid);
            recycle_1(&l);
            recycle_2(&l);
            readyqu.remove(i,1);
            break;
        }
    }
    if(flag==0) qDebug()<<"就绪队列无该进程，挂起失败";
    else qDebug()<<"挂起队列成功";
     ui->hang_pcb->clear();
}

void MainWindow::on_pushButton_4_clicked()
{
    int flag = 0;
    QString name = ui->cancal_hangpcb->text();
    for(int i = 0;i<highqu.size();i++)
    {
        if(highqu[i].pid==name)
        {
            flag = 1;
            highqu[i].status = 1;
            reservequ.append(highqu[i]);
            highqu.remove(i,1);
        }
    }
    if(flag==0) qDebug()<<"挂起队列无该进程，解挂失败";
    else qDebug()<<"解挂成功";
    ui->cancal_hangpcb->clear();
}
