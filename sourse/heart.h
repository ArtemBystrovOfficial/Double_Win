#ifndef HEART_H
#define HEART_H

#include "Bridge_Gui_Server.h"
#include <QMainWindow>
#include <QLabel>
#include <memory>
#include <QHBoxLayout>
#include <vector>
#include <QPushButton>
#include <QTabBar>
#include "tasks.h"
#include <QTextBrowser>
#include <QPainterPath>
#include <QPainter>
#include <QTimer>
#include <QTime>
#include <fstream>
#include <string>
#include <QMessageBox>
#include <deque>

//#include
QT_BEGIN_NAMESPACE
namespace Ui { class Heart; }
QT_END_NAMESPACE

class Heart : public QMainWindow
{
    Q_OBJECT
    struct Time
    {
        int h,m,s;
        bool time_over=false;
        std::string timer();
        Time & operator+=(int min)
        {
            this->h+=min/60;
            this->m+=min%60;
        }
        void stop()
        {
            h=0, m=0, s=0;
        }

    };

signals:

    void s_Check();
    void s_Dismiss();
    void s_More_time();
    void s_Work_In();

public:

   // void set_server_connect(Double_Exam_Client *);

    std::string get_text_from_area();

    Heart(QWidget *parent = nullptr);

    void re_colors();

    void CentralWindow();

    int get_curret_status();

    void set_online_group(const std::vector<QWidget*> &);

    void set_status_tab(int index_tab,const Tab_mode &mode);

    void setFullTimer(int h,int m,int s);

    void setTaskTimer(int h,int m,int s);

    void set_other_info_page(int h,int m,int score,int index);

    ~Heart();

public:
    // objects with client-server
    int time_completition;
    int this_id=-1;
    Time timer_task;
    bool is_ask_minutes_send=false;

public slots :

     void disconnected();

     void no_extra_time();

     void set_logo_and_nickname(std::string path, std::string nick);

     void add_pages(std::vector<Info_page>);

     void add_extra_minutes();

     void start_work();

     void set_online(std::vector<std::string>);

     void refresh_tabs(std::vector<int>);

private slots:

     void slotTimerAlarm();

     void slotTimerTaskAlarm();

    void zoom_in()
    {
        size_text+=2;
            for(int i=0;i< tasks.size();i++)
            {
                auto * page = tasks[i].task_page.get();
                QFont f = page->font();
                f.setPointSize(size_text);
                page->setFont(f);
            }
    }

    void zoom_out()
    {
        size_text-=2;
        for(int i=0;i< tasks.size();i++)
        {
            auto * page = tasks[i].task_page.get();
            QFont f = page->font();
            f.setPointSize(size_text);
            page->setFont(f);
        }
    }

    void tabSelected();

    void on_Tasks_clicked();

    void on_WaitRoom_clicked();

    void on_Talk_clicked();

    void on_History_clicked();

    void on_Help_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_BTake_clicked();

    void on_BSend_clicked();

    void on_BRefuse_clicked();

    void on_AddMinutes_clicked();

private:


    int x_live_room=0,y_live_room=0;
    int size_text=16;
    std::vector <Tasks> tasks;
    std::unique_ptr<QPushButton> logout;
    std::vector <QWidget*> list_online;
    bool is_me_in_work=false;
    bool is_app_close=false;

    Ui::Heart *ui;

    std::unique_ptr<QTimer> timer;

    std::unique_ptr<QTimer> timertask;

    Time timer_start;

};




#endif // HEART_H



