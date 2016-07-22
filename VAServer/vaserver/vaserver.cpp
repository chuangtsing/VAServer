#include <iostream>
#include "vanet.hpp"
#include "task_handlers.hpp"
#include <thread>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp> 
#include <dirent.h>
#include <chrono>
#include <ctime>
#include <fstream>
#include <future>

using namespace std;
using namespace vanet;

int main_port = DEFAULT_PORT;
string gateway = DEFAULT_GATEWAY;
extern int server;
extern vector<pair<string, int>> clist;
extern vector<string> client_list;
vector<tuple<string,int,vanet_pb::ClientMessage>> c_messages;

/*void task_dispatch(task_type task) {
	discover_ping();
	for (auto client : client_list) {
		cout << "Connected to " << client.first << " on socket " << client.second << endl;
	}
	if (server >= 0)
		close(server);
}*/

	typedef chrono::high_resolution_clock Time;
	typedef chrono::milliseconds ms;
	typedef chrono::duration<double> dur;
	typedef chrono::system_clock::time_point tp;

void task_dispatch(task_type task, vanet::Scheduling sched) {
	//discover_clients();
	if (client_list.size() == 0) {
		client_list.push_back("10.100.1.200");
		client_list.push_back("10.100.1.201");
		client_list.push_back("10.100.1.202");
		client_list.push_back("10.100.1.203");
	}
	int i, size, fd;
	pair<thread,bool> *threads;
	size = client_list.size();
	
	c_messages.clear();

	threads = new pair<thread,bool>[size];

	if (task == INIT) {
		for(i = 0; i < size; i++) {
			if ((fd = open_socket(client_list[i], DEFAULT_PORT, RCV_TIMEOUT, SND_TIMEOUT)) >= 0) {
				threads[i].first = thread(init_handler, fd, client_list[i]);
				threads[i].second = true;
			}
			else
				threads[i].second = false;
			
		}
	}

	if (task == QUERY_ALL) {
		for(i = 0; i < size; i++) {
			if ((fd = open_socket(client_list[i], DEFAULT_PORT, RCV_TIMEOUT, SND_TIMEOUT)) >= 0) {
				threads[i].first = thread(query_all_handler, fd, client_list[i]);
				threads[i].second = true;
			}
			else
				threads[i].second = false;
		}
	}

	for(int i = 0; i < size; i++) {
		if (threads[i].second)
			threads[i].first.join();
	}

	delete [] threads;

	if (task == QUERY_ALL && c_messages.size() > 0) {
		tp t1 = Time::now();
		query_task(sched);
		tp t2 = Time::now();
		dur d = t2 - t1;
		cout << "Time elapsed:\t" << (double) chrono::duration_cast<ms>(d).count() / 1000 << " sec" << endl;
	}
	
}

int main(int argc, char **argv) {

	/*caffe::Caffe::Brew mode;
	string file_name = string(argv[1]);
	if (argc >= 3 && string(argv[2]) == "cpu")
		mode = caffe::Caffe::CPU;
	else
		mode = caffe::Caffe::GPU;

    //caffe::CaffePred *caffe_pred = new caffe::CaffePred(MODEL_PATH, WEIGHT_PATH);
	caffe::Classifier *classifier = new caffe::Classifier(MODEL_PATH, WEIGHT_PATH, MEAN_PATH);
	classifier->Classify(cv::imread("videos/extracted/frame_1.jpg"), mode, 1);
	
	tp t1 = Time::now();
	clock_t c1 = clock();
	//string directory = extract_frames("VID_20150706_112355.mp4");
	vector<cv::Mat*> *imgs = new vector<cv::Mat*>();
	extract_frames(file_name, *imgs); 
	tp t2 = Time::now();
	clock_t c2 = clock();
	//process_group(directory, caffe_pred, mode, 1, 100);
	predict_frames(*imgs, classifier, mode);
	tp t3 = Time::now();
	clock_t c3 = clock();

	dur d1 = t2 - t1;
	dur d2 = t3 - t2;

	cout << "File: " << file_name << endl << endl;
	cout << "Extract time elapsed:\t" << chrono::duration_cast<ms>(d1).count() << endl;
	cout << "Extract time total:\t" << (double(c2 - c1) / CLOCKS_PER_SEC) * 1000 << endl << endl;
	cout << "Predict time elapsed:\t" << chrono::duration_cast<ms>(d2).count() << endl;
	cout << "Predict time total:\t" << (double(c3 - c2) / CLOCKS_PER_SEC) * 1000 << endl << endl << endl;

	delete imgs;
	

	delete classifier;*/


	int socket, a, b;
	string in, cmd, prev;
	vector<string> command;

	

	cout << INPUT_PROMPT;
	getline(cin, in);

	while (true) {
		boost::trim_if(in, boost::is_any_of("\t "));
		boost::algorithm::split(command, in, boost::is_any_of("\t "), boost::token_compress_on);
		cmd = command[0];

		if (cmd == "")
			cmd = prev;

		if (cmd == "help" || cmd == "h") {
			cout << "Available commands:\n"
			<< "->'init' or 'i'\n"
			<< "   Initialize devices on network\n"
			<< "->'query-all'\n"
			<< "   Pull all videos\n"
			<< "   (unprocessed videos may be processed on server)\n"
			<< "->'query-tags' [tags]\n"
			<< "   Pull videos associated with specified tags (space delimited)\n"
			<< "   (unprocessed videos may be processed on server)\n"
			<< "->'quit' or 'q'\n"
			<< "   Quit VAServer\n";
		}
		else if (cmd == "init" || cmd == "i")
			task_dispatch(INIT, SERVER);
		else if (cmd == "quit" || cmd == "q")
			return 0;
		else if (cmd == "query-all" || cmd == "a") {
			vanet::Scheduling sched = OPT;
			if (command.size() == 2) {
				string s = command[1];
				if (s == "mobile" || s == "m")
					sched = MOBILE;
				else if (s == "server" || s == "s")
					sched = SERVER;
			}
			task_dispatch(QUERY_ALL, sched);
		}
		else
			cout << "Command not recognized\n" << "Enter 'help' for available commands\n";
		
		prev = cmd;
		cout << INPUT_PROMPT;
		getline(cin, in);
	}

	//host_list.push_back(string("10.100.1.102"));

	return 0;
}

