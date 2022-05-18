#include "heart.h"
#include "./ui_heart.h"
#include <filesystem>

namespace fs = std::filesystem;

Heart::Heart(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Heart)
{
    ui->setupUi(this);
    CentralWindow();
}


Heart::~Heart()
{
    std::for_each(list_online.begin(),list_online.end(),[this](QWidget * elem){
        delete elem;
    });

    is_app_close = true;

    delete ui;

    exit(0);
}

void Heart::add_extra_minutes()
{
    timer_task+=15;

    QMessageBox::warning(this, "From Server", "Admin gave your 15 min");

    is_ask_minutes_send=false;
}

void Heart::set_logo_and_nickname(std::string path, std::string nick)
{
    ui->Nickname->setText(nick.c_str());
        QPixmap pix;

        if (!pix.load( path.c_str() )) {
            qWarning("./target.png");
        }
        pix = pix.scaled(ui->Logo->size(),Qt::KeepAspectRatio);

    ui->Logo->setPixmap(pix);

}

void Heart::slotTimerAlarm()
{
    if(!timer_start.time_over)
        ui->FullTImer->setText(QString::fromStdString(timer_start.timer()));
    else
    {
        ui->FullTImer->setText(QString::fromStdString("Comp-on end"));

        QPixmap pix_lock;

        if (!pix_lock.load(QString(std::string(fs::current_path().string()+std::string("\\images\\lock.png")).c_str()))) {
             qWarning("Failed to load images/lock.png");
        }
        pix_lock = pix_lock.scaled(ui->Logo->size(),Qt::KeepAspectRatio);

        ui->Tasks->setIcon(pix_lock);

        ui->Tasks->setStyleSheet("background:gray;");

        ui->Tasks->setDisabled(true);

        ui->MenuBar->setTabEnabled(0,false);

        ui->BRefuse->setHidden(true);
        ui->BSend->setHidden(true);
        ui->BTake->setHidden(true);
        timer_task.time_over=true;
        ui->TaskTimer->setText(QString::fromStdString(std::string("-:-:-")));
    }
}

void Heart::slotTimerTaskAlarm()
{
    static bool is=true;
    if(!timer_task.time_over)
    {
        ui->TaskTimer->setText(QString::fromStdString(timer_task.timer()));
        is=true;
    }
    else
    {
        if(is)
        {
            if(is_me_in_work)
            {
                if(ui->WorkSpace->toPlainText().isEmpty())
                {
                    on_BRefuse_clicked();
                }
                else
                {
                    on_BSend_clicked();
                }
            }
          is=false;
        }
        ui->TaskTimer->setText(QString::fromStdString(std::string("times end")));
    }
}

void Heart::setTaskTimer(int h,int m,int s)
{

    timer_task.time_over=false;
    timer_task.h=h;
    timer_task.m=m;
    timer_task.s=s;

    timertask = std::move(std::unique_ptr<QTimer>(new QTimer()));
    connect(timertask.get(), SIGNAL(timeout()), this, SLOT(slotTimerTaskAlarm()));
    timertask.get()->start(1000);

}
void Heart::set_other_info_page(int h,int m,int score,int index)
{
    if(tasks.size()<=index){

        return;
    }

    tasks[index].h=h;
    tasks[index].m=m;
    tasks[index].score=score;
}
void Heart::CentralWindow()
{

       ui->AddMinutes->setHidden(true);
       ui->BRefuse->setHidden(true);
       ui->BSend->setHidden(true);
       ui->BTake->setHidden(true);
       QPixmap pix_lock;

       if (!pix_lock.load(QString(std::string(fs::current_path().string()+std::string("\\images\\lock.png")).c_str()))) {
            qWarning("./lock.png");
       }
       pix_lock = pix_lock.scaled(ui->Logo->size(),Qt::KeepAspectRatio);

       ui->Tasks->setIcon(pix_lock);

       if (!pix_lock.load(QString(std::string(fs::current_path().string()+std::string("\\images\\diamond.png")).c_str()))) {
            qWarning("/diamond.png");
       }
       pix_lock = pix_lock.scaled(ui->Score->size(),Qt::KeepAspectRatio);

       ui->Score->setIcon(pix_lock);

       if (!pix_lock.load(QString(std::string(fs::current_path().string()+std::string("\\images\\help.png")).c_str()))) {
            qWarning("/help.png");
       }
       pix_lock = pix_lock.scaled(QSize(890,501),Qt::KeepAspectRatio, Qt::SmoothTransformation);

       ui->Help_img->setBackgroundRole(QPalette::Dark);
       ui->Help_img->setPixmap(pix_lock);


       logout = std::move(std::unique_ptr<QPushButton>(new QPushButton("Logout",this)));

       ui->MenuBar->tabBar()->hide();

       connect(ui->TasksBar, SIGNAL(currentChanged(int)), this, SLOT(tabSelected()));

}

void Heart::re_colors()
{
    int id=0;
    for_each(tasks.cbegin(),tasks.cend(),[this,&id](const Tasks & task){
        set_status_tab(id,task.mode);
        id++;
    });
}

void Heart::refresh_tabs(std::vector<int> list)
{
    if(list.size()!=tasks.size())
    {
         QMessageBox::warning(this, "Error","Different count tabs and recive, write to help");
    }
    for(int i=0;i<list.size();i++)
    {
        if(list[i]>=1000 && list[i]<1256)
        {
           if(this_id==list[i]-1000)
               tasks[i].mode = Tab_mode::Accept;
           else
               tasks[i].mode = Tab_mode::AcceptOther;
        }
        if(list[i]==0)
        {
            tasks[i].mode = Tab_mode::Common;
        }
        if(list[i]>0 && list[i]<1000)
        {
            if(this_id==list[i])
            {

              if(is_me_in_work)
                tasks[i].mode = Tab_mode::InWork;
              else
                tasks[i].mode = Tab_mode::WaitAnswer;
            }
            else
                tasks[i].mode = Tab_mode::InOtherWork;
        }
        if(list[i]<0 && list[i]>-256)
        {
            if(this_id==abs(list[i]))
            {
                tasks[i].mode = Tab_mode::Bad;
            }
            else
            {
                tasks[i].mode = Tab_mode::Common;
            }
        }
    }
    re_colors();
    tabSelected();
}

void Heart::set_online( std::vector<std::string> list)
{
    auto color = QPixmap(50,50);
    color.fill(QColor::fromRgb(rand()%50+150,rand()%50+150,rand()%50+150));

    std::vector <QWidget *> v;
    for(auto i=0;i<list.size();i++)
    {
    auto * elem = new QPushButton(list[i].c_str());
    v.push_back(elem);
    elem->setMinimumHeight(60);
    elem->setIcon(color);
    }
    set_online_group(v);

}

void Heart::tabSelected(){

    if(!is_app_close)
        {

    int index = ui->TasksBar->currentIndex();


    ui->Score->setText(QString::number(tasks[index].score));

    if(is_me_in_work && tasks[index].mode!=Tab_mode::InWork)
    {
        ui->AddMinutes->setHidden(true);
        ui->BRefuse->setHidden(true);
        ui->BSend->setHidden(true);
        ui->BTake->setHidden(true);
        return;
    }

    switch(tasks[index].mode)
    {

    case Tab_mode::Accept:
    case Tab_mode::Bad:
    {
        ui->AddMinutes->setHidden(true);
        ui->BRefuse->setHidden(true);
        ui->BSend->setHidden(true);
        ui->BTake->setHidden(true);
    }
    break;
    case Tab_mode::Common:
    {
        ui->AddMinutes->setHidden(true);
        ui->BRefuse->setHidden(true);
        ui->BSend->setHidden(true);
        ui->BTake->setHidden(false);
    }
    break;
    case Tab_mode::InWork:
    {
        ui->AddMinutes->setHidden(false);
        ui->BRefuse->setHidden(false);
        ui->BSend->setHidden(false);
        ui->BTake->setHidden(true);
    }
    break;
    case Tab_mode::InOtherWork:
    case Tab_mode::WaitAnswer:
    case Tab_mode::AcceptOther:
     {
        ui->AddMinutes->setHidden(true);
        ui->BRefuse->setHidden(true);
        ui->BSend->setHidden(true);
        ui->BTake->setHidden(true);
     }
    break;
    }
    }
}

void Heart::add_pages(std::vector<Info_page> pages)
{
    for_each(pages.cbegin(),pages.cend(),[&](Info_page page){
        tasks.push_back(Tasks(page.data));
    });

        for(int i=0;i<tasks.size();i++)
      {
                set_other_info_page(pages[i].minutes/60,pages[i].minutes%60,pages[i].score,i);
      }
}

void Heart::setFullTimer(int h,int m,int s)
{
    timer_start.h=h;
    timer_start.m=m;
    timer_start.s=s;

    timer = std::move(std::unique_ptr<QTimer>(new QTimer()));
    connect(timer.get(), SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));
    timer->start(1000);
}

void Heart::start_work()
{

     setFullTimer(time_completition/60,time_completition%60,0);

     ui->TasksBar->removeTab(1);
     ui->TasksBar->removeTab(0);
    for(decltype(tasks.size()) id_task=0;id_task<tasks.size();id_task++)
    {

          QTextBrowser * page = new QTextBrowser();
          page->setText(QString::fromStdString(tasks[id_task].get_text()));
          page->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
          QFont f = page->font();
          f.setPointSize(16);
          page->setFont(f);
        //RESIZE WINDOW PROPITERES
          page->setMaximumWidth(720);
          page->setMaximumHeight(720);
          tasks[id_task].task_page = std::move(std::unique_ptr<QTextBrowser>(page));
          ui->TasksBar->addTab(tasks[id_task].task_page.get(), QString::number(id_task));
          set_status_tab(id_task,Tab_mode::Common);
    }

    ui->Tasks->setIcon(QIcon());
    ui->Tasks->setEnabled(true);
    ui->Tasks->setStyleSheet("qlineargradient(spread:repeat, x1:1, y1:0, x2:1, y2:1, stop:0 rgba(139, 80, 255, 255),stop:1"
                             "rgba(105, 105, 255, 255));");


}
void Heart::set_online_group(const std::vector<QWidget*> & list)
{

    std::for_each(list_online.begin(),list_online.end(),[this](QWidget * elem){
        delete elem;
    });
    list_online.clear();
    std::for_each(list.cbegin(),list.cend(),[this](QWidget * elem){
        list_online.push_back(new QWidget());
        list_online[list_online.size()-1]=elem;
    });

    x_live_room=0;
    y_live_room=0;

    std::for_each(list_online.cbegin(),list_online.cend(),[this](QWidget * elem){
        ui->Online_list->addWidget(elem,y_live_room,x_live_room);
        x_live_room++;
        if(x_live_room>=4)
        {
            x_live_room=0;
            y_live_room++;
        }
    });
}

int Heart::get_curret_status()
{
    return ui->TasksBar->currentIndex();
}

void Heart::set_status_tab(int index_tab,const Tab_mode & mode)
{
    if(tasks.size()<=index_tab) return;

    //tasks[index_tab].mode=mode;

    auto color = QPixmap(50,50);
    switch(mode)
    {
        case Tab_mode::Accept :
         {
            color.fill(QColor("green"));
         }
         break;
        case Tab_mode::Bad :
         {
            color.fill(QColor("red"));
         }
         break;
        case Tab_mode::Common :
         {
            color.fill(QColor("yellow"));
         }
         break;
        case Tab_mode::InWork :
         {
            color.fill(QColor("white"));
         }
        break;
        case Tab_mode::InOtherWork :
         {
            color.fill(QColor("gray"));
         }
         break;
        case Tab_mode::WaitAnswer :
         {
            color.fill(QColor::fromRgb(144,0,144));
         }
         break;
        case Tab_mode::AcceptOther :
         {
            color.fill(QColor::fromRgb(40,40,40));
         }
         break;
    }
    ui->TasksBar->setTabIcon(index_tab,color);
}

void Heart::on_Tasks_clicked()
{
    ui->MenuBar->setCurrentIndex(0);
}

void Heart::on_WaitRoom_clicked()
{
    ui->MenuBar->setCurrentIndex(1);
}

void Heart::on_Talk_clicked()
{
    ui->MenuBar->setCurrentIndex(2);
}

void Heart::on_History_clicked()
{
    ui->MenuBar->setCurrentIndex(3);
}

void Heart::on_Help_clicked()
{
    ui->MenuBar->setCurrentIndex(4);
}

void Heart::on_pushButton_2_clicked()
{
    zoom_out();
}

void Heart::on_pushButton_3_clicked()
{
    zoom_in();
}


void Heart::on_BTake_clicked()
{

    emit s_Work_In();

    tasks[ui->TasksBar->currentIndex()].mode=Tab_mode::InWork;

    tabSelected();
    re_colors();
    is_ask_minutes_send=false;

    is_me_in_work=true;
    setTaskTimer(tasks[ui->TasksBar->currentIndex()].h,tasks[ui->TasksBar->currentIndex()].m,0);
    ui->Talk->setStyleSheet("QPushButton{background: qlineargradient(spread:repeat, x1:1, y1:0, x2:1, y2:1, stop:0 rgba(254, 171, 66, 255),stop:1 rgba(255, 153, 0, 255));}"
                            "QPushButton::hover{background-color: QLinearGradient(x1:0, y1:0, x2:0, y2:1, stop:1 rgba(254, 178, 78, 255) stop:0.4 rgba(254, 182, 80, 255), stop:0.2 rgba(254, 185, 82, 255), stop:0.1 rgba(254, 198, 99, 255));}"
                            );


}

void Heart::on_BSend_clicked()
{

        if(ui->WorkSpace->toPlainText()=="")
        {

           QMessageBox::warning(this, "Warining","Empty work");
           return;
        }

        if(ui->WorkSpace->toPlainText().size()>2000)
        {

           QMessageBox::warning(this, "Warning","So big message");
           return;
        }

        // отправляется запрос
        emit s_Check();

        tasks[ui->TasksBar->currentIndex()].mode=Tab_mode::WaitAnswer;
        tabSelected();
        re_colors();

        is_me_in_work = false;
        is_ask_minutes_send=false;

        timer_task.stop();


}
std::string Heart::get_text_from_area()
{
    return ui->WorkSpace->toPlainText().toStdString();
}

void Heart::disconnected()
{
    QMessageBox::warning(this, "From Server","Disconected");
}

void Heart::no_extra_time()
{
    QMessageBox::warning(this, "From Server","Sorry, admin give'nt your 15 extra minutes, read help");
}

void Heart::on_BRefuse_clicked()
{
        // защита
        if(!timer_task.time_over)
        {
            QMessageBox* pmbx =
                                new QMessageBox("Warining",
                                "<b>Do you want Refuse at this task?</b>",
                                QMessageBox::Information,
                                QMessageBox::Yes,QMessageBox::No,QMessageBox::Cancel | QMessageBox::Escape);
            int n = pmbx->exec();

            delete pmbx;

            if (n != QMessageBox::Yes) return;
        }
        // отправляется запрос
        emit s_Dismiss();

        tasks[ui->TasksBar->currentIndex()].mode=Tab_mode::Bad;
        tabSelected();
        re_colors();
        timer_task.stop();

        is_me_in_work = false;
        is_ask_minutes_send=false;

}

void Heart::on_AddMinutes_clicked()
{
    if(is_ask_minutes_send)
    {
        QMessageBox::warning(this, "Warining","Request already sent");
        return;
    }
    emit s_More_time();
}

std::string Heart::Time::timer()
{
    std::string st="";
    if(s<=0)
    {
        s=59;
        if(m<=0)
        {
            m=59;
            if(h<=0)
            {
               time_over=true;
            }
            else
            {
                h--;
            }
        }
        else
        {
            m--;
        }
    }
    else
    {
      s--;
    }

    if (h<10)
    {
        st+=(std::string("0"+std::to_string(h)));
    }
    else
    {
        st+=(std::to_string(h));
    }
    st+=(":");
    if (m<10)
    {
        st+=(std::string("0"+std::to_string(m)));
    }
    else
    {
        st+=(std::to_string(m));
    }
    st+=(":");
    if (s<10)
    {
        st+=(std::string("0"+std::to_string(s)));
    }
    else
    {
        st+=(std::to_string(s));
    }
    return st;
}
