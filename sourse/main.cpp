#include "heart.h"
#include <QApplication>
#include <QFile>
#include "DEX_Cleint.h"
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Heart w;

    QPixmap pix((fs::current_path().string()+="\\images\\splash.png").c_str());
    QSplashScreen splashScreen( pix );
    splashScreen.show();

    w.setWindowTitle("Double Win");
    w.setWindowIcon(QIcon(":/images/icon.png"));


    Double_Exam_Client client_server("188.168.25.28",21112,&w);

    Sleep(1000);

    QObject::connect(&client_server, &Double_Exam_Client::s_add_pages, &w, &Heart::add_pages,Qt::QueuedConnection);
    QObject::connect(&client_server, &Double_Exam_Client::s_refresh_tabs, &w, &Heart::refresh_tabs,Qt::QueuedConnection);
    QObject::connect(&client_server, &Double_Exam_Client::s_start_work, &w, &Heart::start_work,Qt::QueuedConnection);
    QObject::connect(&client_server, &Double_Exam_Client::s_set_online, &w, &Heart::set_online,Qt::QueuedConnection);
    QObject::connect(&client_server, &Double_Exam_Client::s_add_minutes, &w, &Heart::add_extra_minutes,Qt::QueuedConnection);
    QObject::connect(&client_server, &Double_Exam_Client::s_set_logo_and_nickname, &w, &Heart::set_logo_and_nickname,Qt::QueuedConnection);
    QObject::connect(&client_server, &Double_Exam_Client::disconnected, &w, &Heart::disconnected,Qt::QueuedConnection);
    QObject::connect(&client_server, &Double_Exam_Client::no_extra_time, &w, &Heart::no_extra_time,Qt::QueuedConnection);

    QObject::connect(&w, &Heart::s_Check, &client_server , &Double_Exam_Client::sl_Check);
    QObject::connect(&w, &Heart::s_Dismiss, &client_server ,&Double_Exam_Client::sl_Dismiss);
    QObject::connect(&w, &Heart::s_More_time, &client_server , &Double_Exam_Client::sl_More_time);
    QObject::connect(&w, &Heart::s_Work_In, &client_server , &Double_Exam_Client::sl_Work_In);

    client_server.set_nick();

    w.show();
    splashScreen.finish( &w );

    return app.exec();
}

