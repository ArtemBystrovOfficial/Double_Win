#include "FServer.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

class HTTP_Results;

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

	void set_text(const std::string& s)
	{
		for (auto i = 0; i < s.size(); i++)
		{
			work_buffer[i] = s[i];
		}
		work_buffer[s.size()] = '\0';
	}

	void set_status(std::vector<int> & vec)
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

	Pocket(commands_server&& mode): mode_server (mode)
	{

	}

	char online[1024];
	int status[256];
	int task_id;
	int person_id=-1;
	char work_buffer[2048];
	int time_completetion;

	commands_server mode_server;
	commands_client mode_client;

};

class Double_Exam_Server
{
public:
	enum class Server_command
	{
		Start,
		Send_pages
	};
	
	void set_time(int time)
	{
		time_completition = time;
	}
	std::vector<int> tasks_list;
	std::map<int, std::string> nicknames;

private:

	std::mutex lock_nicks;
	int time_completition;
	Server<Pocket> server;
	std::thread read;
	std::thread listen;
	std::thread add_nickname;
	int count_pages;

	std::atomic<bool> is_start{ false };
	void Reader()
	{
		Pocket pocket;
		int id;
		std::pair< Pocket , int> input;

		while (!server.is_server_stoped())
		{
			server >> input;

			auto [pocket, id] = input;
			if (id != -1)
			{
				if (pocket.task_id >= tasks_list.size())
				{
					if (pocket.mode_client != Pocket::commands_client::More_time)
						std::cout << "ERROR: with pages restart\n" << id+1 << std::endl;
				}
				switch (pocket.mode_client)
				{
				case Pocket::commands_client::Check:
				{
					std::cout << "-+-+-+-+-+-+-+-+-+| Task " << pocket.task_id << " from id: " << id+1 <<
						" |+-+-+-+-+-+-+-+-+-\n";
					std::cout << pocket.work_buffer << std::endl;
					std::cout << "+-+-+-+-+-+-+-+-+-| $Ok or $Bad |-+-+-+-+-+-+-+-+-+-+\n";
					std::string s;

					while (true)
					{
						std::cin >> s;
						if (s == "$Ok")
						{
							tasks_list[pocket.task_id] = 1000 + id + 1;
							break;
						}
						if (s == "$Bad")
						{
							tasks_list[pocket.task_id] = - id - 1;
							break;
						}
					}
				}
				break;
				case Pocket::commands_client::More_time:
				{
					std::cout << "-+-+-+-+-+-+-+-+-+| +10 min for " << pocket.task_id << " from id: " << id+1 <<
						" |+-+-+-+-+-+-+-+-+-\n";
					std::cout << pocket.work_buffer << std::endl;
					std::cout << "+-+-+-+-+-+-+-+-+-| $Ok or $Bad |-+-+-+-+-+-+-+-+-+-+\n";

					std::string s;

					while (true)
					{
						std::cin >> s;
						if (s == "$Ok")
						{
							server << std::pair{ Pocket(Pocket::commands_server::Yes), id };
							break;
						}
						if (s == "$Bad")
						{
							server << std::pair{ Pocket(Pocket::commands_server::No), id };
							break;
						}
					}
				}
				break;
				case Pocket::commands_client::Dismiss:
				{

					tasks_list[pocket.task_id] = -id-1;

				}
				break;
				case Pocket::commands_client::WorkIn:
				{
					tasks_list[pocket.task_id] = id+1;
				}
				break;
				}

				pocket.mode_server = Pocket::commands_server::Refresh;

				pocket.set_status(tasks_list);

				server << pocket;
			}
		}
	}

public:

	Double_Exam_Server() = delete;

	void Sender(Server_command && mode)
	{
		switch (mode)
		{
			case Server_command::Send_pages:
			{
				for (const auto& entry : fs::directory_iterator("./pages"))
				{
					server << entry.path().filename().u8string();
				}
			}
			break;

			case Server_command::Start:
			{

				is_start.store(true);
				Pocket info(Pocket::commands_server::Start);
				info.time_completetion = this->time_completition;
				server << info;

				auto list = server.list_all_online();
				Pocket pocket;
				pocket.mode_server = Pocket::commands_server::Id;

				for (auto i : list)
				{
					pocket.person_id = i+1;
					server << std::pair{ pocket,i };
				}

				std::thread readf(&Double_Exam_Server::Reader, this);

				read = std::move(readf);

			}
			break;

		}
	}



	explicit Double_Exam_Server(const char* IP, int port, int count_pages) : server(IP, port),
		count_pages(count_pages), tasks_list(count_pages, 0)
	{
		server.start();

		server.set_path_download("./pages/");

		listen = std::move(std::thread{ [this]() {
			Pocket pocket;
			pocket.mode_server = Pocket::commands_server::Refresh_online;
			while (!is_start.load())
			{

			auto list = server.list_all_online();
			std::vector<std::string> nicks;

			lock_nicks.lock();
			for (auto & i : list)
			{
				auto data = nicknames.find(i);
				if (data == nicknames.end())
					nicks.push_back(std::to_string(i));
				else
					nicks.push_back(data->second);
			}

			pocket.set_online(nicks);

			lock_nicks.unlock();

			server << pocket;

			Sleep(1000);

			}

			} });

		add_nickname = std::move(std::thread{ [this]() {
			std::pair< Pocket , int> input;
			while (!is_start.load())
			{
				server >> input;
				lock_nicks.lock();
				auto [pocket, id] = input;
				if (id != -1)
				{
					if (pocket.mode_client == Pocket::commands_client::Nickname)
					{
						nicknames[id] = std::string(pocket.work_buffer);
					}				
				}
				lock_nicks.unlock();
			}

			} });
	};

	~Double_Exam_Server()
	{

		server << Pocket{ Pocket::commands_server::Disconnect };

		server.stop();
		read.join();
		listen.join();
		add_nickname.join();
	}
};

std::string app(std::string s)
{
	auto it = std::find(s.begin(), s.end(), '.');
	it++;
	std::string out="";
	for (auto begin = it; begin < s.end(); begin++)
	{
		out += *begin;
	}
	return out;
}

class HTTP_Results
{
private:
	std::thread read;
	Server <bool> server;
	std::vector<int> task_complete;
	std::vector<int> tab_score;
	std::map<int,int> top;
	std::map<int, std::string> nicknames;
public:
	HTTP_Results(const std::vector<int>& list_tasks,const  std::map<int, std::string> & nicknames  ,const char * IP) :
				task_complete(list_tasks),nicknames(nicknames), server(IP, 80) {

		parser_score();

		set_top();

		print_html_table();

		server.set_path_download(fs::current_path().string() + '\\');

		server.start();
		
		read = std::thread([this]() {

				while (!server.is_server_stoped())
				{
					auto id = server.get_http_connect();
						if (id != -1)
						{
							server << HTTP{ "out.html",id };
						}
					Sleep(100);
				}
				
			});

	};
	
	void parser_score()
	{
		for (const auto& entry : fs::directory_iterator("./pages"))
		{
			std::ifstream in(entry.path());
			if (!in.is_open())
				throw("file didn't open");

			std::string s;
			in >> s;

			if (s == "$")
			{
				in >> s;
			}
			else
			{
				throw "error with parse $";
			}
			int score;

			in >> s;
			if (s == "$")
			{
				in >> score;
			}
			else
			{
				throw "error with parse $";
			}
			tab_score.push_back(score);
		}
	}
	
	void set_top()
	{
		for_each(task_complete.begin(), task_complete.end(), [this](int n) {
			static int id = 0;
			if (n > 1000)
			{
				if (top.find(n - 1001) == top.end())
					top[n - 1001] = tab_score[id];
				else
					top[n - 1001] += tab_score[id];
			}
			id++;
			});
	}
	
	void print_html_table()
	{
		std::ofstream out("out.html");

		std::vector<std::pair<int, int>> top_list;
		for_each(top.begin(), top.end(),[&top_list](std::pair <int,int> i) {
			top_list.push_back(std::pair{ i.second,i.first });
			});
		
		sort(top_list.rbegin(), top_list.rend());

		out << "<style>body{background:#000033;}table{margin:auto;border-collapse:collapse;background:linear-gradient(180deg,#11053b,#1e0b63,#11053b);}td{color:white;font-weight:800;min-width:30px;padding:20px;border:1pxsolidrgba(255,255,255,0.8);}</style><head><title>Results</title></head>";

		out << "<table>";

		out << "<tr>";

		out << "<td>";
			
		out << "Id/Score";

		out << "</td>";

		for (int i = 0; i < task_complete.size(); i++)
		{
			out << "<td>";

			out << i+1;

			out << "</td>";
		}


		out << "<td>";

		out << "Total";

		out << "</td>";

		out << "</tr>";

		for (auto i = 0; i < top_list.size(); i++)
		{
			out << "<tr>";

			out << "<td>";

			auto nick = nicknames.find(top_list[i].second);
			if (nick == nicknames.end())
				out << std::to_string(top_list[i].first);
			else
				out << nick->second;

			out << "</td>";

			for (int j = 0; j < task_complete.size(); j++)
			{
				out << "<td>";


				if (task_complete[j] > 1000)
				{
					if(task_complete[j]-1001 == top_list[i].second)
							out << tab_score[j];
					else
							out << "";
				}
				else
				{
					out << "";
				}


				out << "</td>";
			}

			out << "<td>";

			out << top_list[i].first;

			out << "</td>";

			out << "</tr>";
		}
		out << "</table>";
	}

	HTTP_Results() = delete;
	
	~HTTP_Results()
	{
		server.stop();
		read.join();
	}

};




int main()
{
	setlocale(LC_ALL, "Russian");

	std::string IP;
	int port;

		std::cout << " /+-+-+-+-+-+|-------------|+-+-+-+-+-+\\\n";
		std::cout << "|-+-+-+-+-+-+| Double Win |+-+-+-+-+-+-|\n";
		std::cout << "|-+-+-+-+-+-+|    2022    |+-+-+-+-+-+-|\n";
		std::cout << " \\-+-+-+-+-+-+|-----------|+-+-+-+-+-+-/\n";

		Sleep(1000);
		system("cls");

	std::cout << "Correct IP: ";
	std::cin >> IP;
		system("cls");

	std::cout << "Correct PORT: ";
	std::cin >> port;
		system("cls");

	std::vector<int> list_result;
	std::map<int, std::string> nicknames;

	{
		int count_pages = 0;

		auto path = fs::current_path();

		path.append("pages");

		try
		{
			if (!fs::exists(path))
				throw "Did'nt find ./pages";

			for (const auto& entry : fs::directory_iterator("./pages"))
			{
				if (app(entry.path().filename().u8string()) != "html")
					throw "Not all files is HTML";

				count_pages++;
			}
		}
		catch (const char* msg)
		{
			std::cout << msg << std::endl;
			Sleep(1000);
			exit(0);
		}

		std::cout << "Status: Files_page_ok\n";
		int time;
		std::cout << "How mush time compitition will worked?\n";
		std::cin >> time;

		Double_Exam_Server sv(IP.c_str(), port, count_pages);

		sv.set_time(time);

		std::cout << "Server: Server start\n";

		std::string command;

		do
		{
			std::cout << "$Send for send pages to all\n";
			std::cin >> command;
		} while (command != "$Send");

		sv.Sender(Double_Exam_Server::Server_command::Send_pages);

		do
		{
			std::cout << "$Start for start\n";
			std::cin >> command;
		} while (command != "$Start");

		sv.Sender(Double_Exam_Server::Server_command::Start);

		Sleep(time * 60000);

		list_result = sv.tasks_list;
		nicknames = sv.nicknames;
	}

	std::cout << "Completition has end, write $End for close HTML server\n";

	HTTP_Results html(list_result,nicknames,IP.c_str());

	std::string s;

	while (s != "$End")
	{
		std::cin >> s;
	}

}