#include "DEX_Cleint.h"

using cm = Pocket::commands_client;

void Double_Exam_Client::sl_Check()
{
    Pocket pocket;

    pocket.task_id = window->get_curret_status();
    pocket.mode_client = cm::Check;
    pocket.set_text(window->get_text_from_area().c_str());

    client << pocket;
}

void Double_Exam_Client::sl_Dismiss()
{
    Pocket pocket;

    pocket.mode_client = cm::Dismiss;
    pocket.task_id = window->get_curret_status();

    client << pocket;
}

void Double_Exam_Client::sl_More_time()
{
    Pocket pocket;

    pocket.task_id = window->get_curret_status();
    pocket.mode_client = cm::More_time;
    pocket.set_text(window->get_text_from_area().c_str());

    client << pocket;
}

void Double_Exam_Client::sl_Work_In()
{
    Pocket pocket;

    pocket.task_id = window->get_curret_status();
    pocket.mode_client = cm::WorkIn;

    client << pocket;
}

