#ifndef TASKS_H
#define TASKS_H

#include <string>
#include <memory>
#include <QLabel>
#include <QScrollArea>
#include <QTextBrowser>

enum class Tab_mode
{
    WaitAnswer,
    Common,
    InWork,
    InOtherWork,
    Accept,
    Bad,
    AcceptOther
};

struct Info_page
{
    int minutes;
    int score;
    std::string data;
};

class Tasks
{
private:
    std::string task;
public:
    int h=0,m=0;

    int score=0;

    Tab_mode mode = Tab_mode::Common;
    std::unique_ptr<QTextBrowser> task_page;
    Tasks() = delete;

    Tasks(const std::string & str,int h=0,int m=0,int score=0)
    {
        task = str;
    }

    std::string get_text()
    {
        return task;
    }
};

/*std::string app_name(std::string s)
{
    auto it = std::find(s.begin(), s.end(), '.');
    it++;
    std::string out = "";
    for (auto begin = it; begin < s.end(); begin++)
    {
        out += *begin;
    }
    return out;
}*/

#endif // TASKS_H
