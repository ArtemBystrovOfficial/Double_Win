#pragma once

#include "heart.h"

#include "FClient.h"

#include <QInputDialog>

namespace fs = std::filesystem;

enum Bridge_from_gui_to_server
{
    Checks,
    More_times,
    Dismisss,
    WorkIns
};

struct Pocket
{
    enum class commands_server
    {
        Refresh,
        Refresh_online,
        Start,
        Yes,
        No,
        Id,
        Disconnect
    };

    enum class commands_client
    {
        Check,
        More_time,
        Dismiss,
        WorkIn,
        Nickname
    };

    void set_text(const std::string & s)
    {
        for(auto i=0;i<s.size();i++)
        {
            work_buffer[i]=s[i];
        }
        work_buffer[s.size()]='\0';
    }

    void set_status(std::vector<int>& vec)
    {
        for (int i = 0; i < vec.size(); i++)
        {
            status[i] = vec[i];
        }
        status[vec.size()] = -1000;
    }

    void set_online(std::vector<std::string>& vec)
        {
            int k = 0;
            for (int j = 0; j < vec.size(); j++)
            {
                auto s = vec[j];
                for (int i = 0; i < s.size(); i++)
                {
                    online[k] = s[i];
                    k++;
                }
                online[k] = '^';
                k++;
            }
            online[k] = '$';
        }

    Pocket()
    {

    }

    Pocket(commands_server&& mode) : mode_server(mode)
    {

    }

    char online[1024];
    int status[256];
    int task_id;
    int person_id = -1;
    char work_buffer[2048];
    int time_completetion;

    commands_server mode_server;
    commands_client mode_client;

};


class Double_Exam_Client : public QObject
{
    Q_OBJECT
signals:
  void s_add_pages(std::vector<Info_page>);
  void s_start_work();
  void s_set_online(std::vector<std::string>);
  void s_refresh_tabs(std::vector<int>);
  void s_add_minutes();
  void s_set_logo_and_nickname(std::string, std::string);
  void disconnected();
  void no_extra_time();

public slots:

  void sl_Check();
  void sl_Dismiss();
  void sl_More_time();
  void sl_Work_In();
public:

  void set_nick()
  {
     // std::cout << fs::current_path();
     emit s_set_logo_and_nickname(fs::current_path().string()+"\\images\\images.png",nickname);
  }

private:  
    std::string nickname;
    Heart * window;
    Client<Pocket> client;
    std::thread read;
    //std::thread send;
    int count_pages=0;
    int my_id=-1;


    void Reader()
    {
        Pocket pocket;
        int id;
        while (!client.is_client_disconected())
        {
            client >> pocket;
            switch (pocket.mode_server)
            {
                case Pocket::commands_server::Id :
                {
                    window->this_id = pocket.person_id;

                    if (pocket.person_id == -1)
                    {
                        throw "Error with person id";
                    }
                }
                break;
                case Pocket::commands_server::Start :
                {

                     window->time_completition = pocket.time_completetion;

                    std::vector <Info_page> pages;
                    for (const auto& entry : fs::directory_iterator("./pages"))
                    {
                       // if (app_name(entry.path().filename().u8string()) == "html")
                       // {
                            Info_page page;

                            count_pages++;

                            std::fstream in(entry.path());


                            if(!in.is_open())
                            {
                                 throw false;
                            }

                            std::string s;
                            s.reserve(2000);
                            char b;
                            int k=0;
                            int task_time, score;
                            in >> b;

                            if(b!='$') throw "Error with time and score";
                            in >> task_time;

                            in >> b;

                            if(b!='$') throw "Error with time and score";
                            in >> score;

                            while(!in.eof())
                            {

                                 in.get(b);
                                 s.push_back(b);

                            }
                            page.data = s;

                            page.minutes = task_time;

                            page.score = score;

                            pages.push_back(page);

                            //window->add_pages(pages);
                       // }
                    }

                    emit s_add_pages(pages);




                    emit s_start_work();

                    //window->start_work();
                }
                break;
                case Pocket::commands_server::Refresh_online :
                {

                    std::vector<std::string> list_online;
                    list_online.reserve(20);

                    char b;
                    int it=0;
                    std::string nick="";
                    do
                    {
                        b = pocket.online[it];

                        if(b=='^')
                        {
                            list_online.push_back(nick);
                            nick="";
                        }
                        else
                        {
                            if(b!='$')
                                nick.push_back(b);
                        }
                        it++;
                    } while(b!='$');

                       //window->set_online(list_online);
                      emit s_set_online(list_online);
                }
                break;
                case Pocket::commands_server::Refresh :
                {

                    std::vector<int> list_online;
                    list_online.reserve(20);
                  for(int i=0 ;i<256&&pocket.status[i]!=-1000;i++)
                  {
                      list_online.push_back(pocket.status[i]);
                  }         
                    //window->refresh_tabs(list_online);
                    emit s_refresh_tabs(list_online);

                }
                break;
                case Pocket::commands_server::No :
                {                 
                    emit no_extra_time();
                }
                break;
                case Pocket::commands_server::Yes :
                {
                    emit s_add_minutes();
                }
                break;
                case Pocket::commands_server::Disconnect :
                {                
                      emit disconnected();
                }
                break;
                default:
                {

                }
                break;
            }

        }
    }

    void set_colors();

public:

    Double_Exam_Client() = delete;

    explicit Double_Exam_Client(const char* IP, int port, Heart * window) : client (IP, port), window(window)
    {
            try
            {

                if (!fs::exists("./pages"))
                {
                    fs::create_directory("pages");
                }

                client.set_path_download("./pages/");
            }
            catch (...)
            {
                QMessageBox::warning(window, "Error ","Error with pages director, write to help");

                std::cout << "Error with pages directory\n";

                exit(0);
            }
         for (const auto& entry : fs::directory_iterator("./pages"))
         {
             fs::remove(entry.path());
         }

        try
           {


        client.connect_to_server();

        bool bOk;
        QString str = QInputDialog::getText( window,
                                             "Regestration",
                                             "Write your nickname:",
                                             QLineEdit::Normal,
                                             "Maria",
                                             &bOk
                                            );
        if (!bOk) {
            str = "Anonym";
        }
        this->nickname = str.toStdString();
        Pocket nick;

        nick.mode_client = Pocket::commands_client::Nickname;

        nick.set_text(str.toStdString());

        client << nick;

        }
        catch(const char *)
        {
            QMessageBox::warning(window, "Error ","Failed connected to server");
            exit(0);
        }



        read = std::move(std::thread(&Double_Exam_Client::Reader,this));

    };

    ~Double_Exam_Client()
    {
        client.disconnect();
       // send.join();
        read.join();
    }
};


