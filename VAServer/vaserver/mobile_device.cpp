#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <regex.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/io/coded_stream.h>
#include <opencv2/highgui/highgui.hpp>
#include "mobile_device.h"
#include "classifier.h"
#include "vanet.h"
#include "mobile_device.h"

#include <iomanip>

using namespace std;
using namespace vanet_pb;

namespace vanet {

    MobileDevice::MobileDevice(const string &ip, bool connect) {
		this->ip = ip;
		receiver_shutdown = false;
		if (connect) {
            if(open_connection() < 0) {
				status = false;
			}
			status = true;
		}
		else
			status = false;
	}

    MobileDevice::MobileDevice(const string &ip, int socket) {
		this->ip = ip;
		this->socket_fd = socket;
		receiver_shutdown = false;
		status = false;
	}

    MobileDevice::~MobileDevice() {
        for (Video *vid : videos)
            delete vid;
        close(socket_fd);
    }

	bool MobileDevice::establish_connection() {
		ClientMessage c_msg;
		ServerMessage s_msg;
		if (!receive_message(c_msg) || c_msg.type() != ClientMessage::CONNECT)
			return false;
		s_msg.set_type(ServerMessage::CONNECT);
		if (!send_message(s_msg))
			return false;

		status = true;
		return true;
	}

	int MobileDevice::open_connection() {
        int client = -1;
		struct sockaddr_in caddr;
		struct timeval tv_rcv, tv_snd;
		
		caddr.sin_family = AF_INET;
        caddr.sin_port = htons(DEFAULT_PORT);
		if (inet_aton(ip.c_str(), &caddr.sin_addr) == 0)
			return -1;
		client = socket(PF_INET, SOCK_STREAM, 0);
		if (client < 0) {
			cout << "Error on socket creation\n";
			return -1;
		}

		tv_rcv.tv_sec = RCV_TIMEOUT;
		tv_rcv.tv_usec = 0;
		tv_snd.tv_sec = SND_TIMEOUT;
		tv_snd.tv_usec = 0;

		setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, &tv_rcv, sizeof(tv_rcv));
		setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &tv_snd, sizeof(tv_snd));

        int ret;
        while ((ret = connect(client, (const struct sockaddr *)&caddr, sizeof(struct sockaddr))) < 0 && errno == EINTR);
        if (ret < 0)
            return -1;

		cout << "Connected to " << ip << endl;
        socket_fd = client;
		return client;
	}

    string MobileDevice::get_ip() {
        return ip;
    }

    void MobileDevice::add_video(Video *vid) {
		unique_lock<mutex> lck(vids_lck);
        videos.push_back(vid);
        vid->index = videos.size();
        remainingVideos.insert(vid);
		lck.unlock();
	}

    Video* MobileDevice::get_video(const string &path) {
		unique_lock<mutex> lck(vids_lck);
        for (Video *vid : videos) {
            if (vid->path == path) {
                lck.unlock();
                return vid;
            }
        }
        lck.unlock();
        return NULL;
	}

    void MobileDevice::setVideoStatus(Video *vid, ProcessStatus status) {
        unique_lock<mutex> lck(vids_lck);
        vid->status = status;
        lck.unlock();
    }

    const vector<Video*>& MobileDevice::get_videos() {
        return videos;
    }

    void MobileDevice::lock_videos() {
        vids_lck.lock();
    }

    void MobileDevice::unlock_videos() {
        vids_lck.unlock();
    }



	bool MobileDevice::send_message(const ServerMessage& message) {
		google::protobuf::uint32 message_length = message.ByteSize();
		int tmp = message_length, varint_size = 1;
		while (tmp > 127) {
			varint_size++;
			tmp >>= 7;
		}

		int buffer_length = message_length + varint_size;
		google::protobuf::uint8 buffer[buffer_length];
		
		google::protobuf::io::ArrayOutputStream array_output(buffer, buffer_length);
		google::protobuf::io::CodedOutputStream coded_output(&array_output);

		coded_output.WriteVarint32(message_length);
		
		int size;
		int *val;

		coded_output.GetDirectBufferPointer((void **) &val, &size);
		

		if(!message.SerializeToCodedStream(&coded_output))
			return false;
		
		if (write(socket_fd, buffer, buffer_length) != buffer_length)
			return false;

		return true;
	}

	bool MobileDevice::receive_message(ClientMessage &message) {
		char *buf;
		int rb, readBytes = 0, size;

		size = read_varint();

		if (size == 0)
			return false;

		buf = new char[size];

		/*while (readBytes < size) {
			if ((rb = read(socket, buf + readBytes, size)) < 0)
				return false;
			if (rb == 0 && readBytes < size)
				return false;
			readBytes += rb;
		}*/

		if (recv(socket_fd, buf, size, MSG_WAITALL) != size)
			return false;

		google::protobuf::io::ArrayInputStream ais(buf, size);
		google::protobuf::io::CodedInputStream coded_input(&ais);

		if (!message.ParseFromCodedStream(&coded_input))
			return false;
		
		delete [] buf;

		return true;
	}

	uint32_t MobileDevice::read_varint() {
		int readBytes, rb, length = 0;
		char bite = 0;
		if ((rb = read(socket_fd, &bite, 1)) < 0)
			return 0;
		readBytes = rb;
		length = bite & 0x7f;
		while (bite & 0x80) {
			memset(&bite, 0, 1);
            if ((rb = read(socket_fd, &bite, 1)) < 0)
                return 0;
			readBytes += rb;
			length |= (bite & 0x7f) << (7*(readBytes - 1));
		}

		return length;
	}

	void MobileDevice::start_receiver() {
		receiver_shutdown = false;
		receiver = thread(&MobileDevice::receiver_func, this);
		receiver.join();
		receiver_shutdown = false;

		/*for(map<string, Video*>::iterator it = videos.begin();
				it != videos.end(); it++) {
			cout << it->first << endl;
			vector<pair<int, vector<int>>> batches = it->second->get_batches();
			int count = 0;
			for (int i = 0; i < batches.size(); i++) {
				int size = batches[i].first;
				cout << size << endl;
				count += size;
			}
			cout << count << " frames total\n\n";
		}*/
	}

	void MobileDevice::stop_receiver() {
		receiver_shutdown = true;
		receiver.join();
		receiver_shutdown = false;
	}

	void recv_frame(const ClientMessage& msg, int socket_fd) {
		FILE *file = fopen("test.jpg", "w");
		char *buf;
		int size = msg.partial_info().frame_size();

		buf = new char[size];

		if (recv(socket_fd, buf, size, MSG_WAITALL) != size)
			return;
		
		fwrite(buf, 1, size, file);
		fclose(file);
	}

	void MobileDevice::receiver_func() {
        /*while (status && !receiver_shutdown) {
			ClientMessage c_msg;
			if(!receive_message(c_msg))
				continue;
			cout << "MESSAGE RECEIVED\n";
			if (c_msg.type() == ClientMessage::UPLINK_TEST) {
				cout << "Uplink test\n";
				long size = c_msg.size();
				char *buf = new char[size];
				recv(socket_fd, buf, size, MSG_WAITALL);
				delete[] buf;
			}
			if (c_msg.type() == ClientMessage::VIDEO_INFO) {
				//vids[string(c_msg.video_info(0).path())] = vid;
				//vids.insert(path, vid);
				add_video(new Video(c_msg.video_info(0)));
            }
			else if (c_msg.type() == ClientMessage::VIDEO) {
				string path = VIDEO_PATH;
				if (receive_video(path, c_msg) <= 0)
					continue;
				cout << "Received video\n";
			}
			else if (c_msg.type() == ClientMessage::INIT) {
				cout << "Init request\n";
				send_resources(c_msg.resources());
            }
        }*/
	}

	void MobileDevice::flush_socket() {
		uint8_t *buffer = new uint8_t[DOWNLOAD_BUFFER_SIZE];

		while(read(socket_fd, buffer, DOWNLOAD_BUFFER_SIZE)> 0);
		
		delete [] buffer;
		// Add failed server message
	}

	char* MobileDevice::receive_frame(const ClientMessage &msg) {
		char *buf;
		int size = msg.partial_info().frame_size();

		buf = new char[size];

		if (recv(socket_fd, buf, size, MSG_WAITALL) != size)
			return NULL;

		return buf;
	}


	cv::Mat MobileDevice::receive_frame_mat(const ClientMessage &msg) {
		char *buf;
		int size = msg.partial_info().frame_size();

		buf = new char[size];

		if (recv(socket_fd, buf, size, MSG_WAITALL) != size)
			return cv::Mat();

		cv::Mat raw = cv::Mat(1, size, CV_8UC1, buf);
		cv::Mat decoded = cv::imdecode(raw, cv::IMREAD_COLOR);

		return decoded;
	}


	// ADD FILE SUFFIX FOR REPEATS
	long MobileDevice::receive_video(string &path, const ClientMessage &msg) {
		long rb, readBytes = 0, i, bars, size_kb, size, read_amt;
		uint8_t buf[DOWNLOAD_BUFFER_SIZE];
		path += msg.video_info(0).name();
		FILE *file = fopen(path.c_str(), "w");

		size = msg.video_info(0).size();
		size_kb = size / 1024;
        //fflush(stdout);
        //system("setterm -cursor off");
		while (readBytes < size) {
			read_amt = size - readBytes;
			rb = read_amt < DOWNLOAD_BUFFER_SIZE ? read_amt : DOWNLOAD_BUFFER_SIZE;
            rb = read(socket_fd, buf, rb);
            if (rb < 0) {
                if (errno == EINTR)
                    continue;
				printf("\nReceive failed : [%s]\n", strerror(errno));
				flush_socket();
				return -1;
			}
			if (rb == 0 && readBytes < size) {
				printf("\nWrong file size: [%s]\n", strerror(errno));
				flush_socket();
				return -1;
			}
			fwrite(buf, 1, rb, file);
			readBytes += rb;
			
            /*if (rb) {
				printf("\r[");
				bars = readBytes * 50 / size;
				for(i = 0; i < 50; i++) {
					if (i < bars)
						printf("=");
					else
						printf(" ");
				}
				printf("] %ld/%ld KB", readBytes / 1024, size_kb);
				fflush(stdout);
            }*/

		}
		printf("\n");
        //system("setterm -cursor on");

        printf("Video received: %s\n", msg.video_info(0).name().c_str());

		fclose(file);
		return readBytes;
	}

	bool MobileDevice::send_resources(const google::protobuf::RepeatedField<int>& res) {
		struct stat f;
		ServerMessage msg;
		vector<pair<string, uint64_t>> files;

		cout << "Sending resources\n";

		msg.set_type(ServerMessage::RES);
		for (auto r : res) {
			string path;
			if (r == Resource::MODEL) {
				path = MODEL_PATH;
			}
			else if (r == Resource::WEIGHTS) {
				path = WEIGHTS_PATH;
			}
			else if (r == Resource::MEAN) {
				path = MEAN_PATH;
			}
			else if (r == Resource::SYNSET) {
				path = SYNSET_PATH;
			}


			if (stat(path.c_str(), &f))
				return false;
			
			msg.add_resources((Resource) r);
			long size = f.st_size;
			msg.add_size(size);
			files.push_back(pair<string, long>(path, size));
			cout << "Added " << path << " (" << size << "B)\n";
		}


		if (!send_message(msg))
			return false;
		
		for (auto f : files) {
			int fd = open(f.first.c_str(), O_RDONLY);
			cout << "Sending file\n";
			if (!send_file(fd, f.second))
				return false;
		}	
		return true;
	}

	bool MobileDevice::send_file(int fd, long size) {
		long sb, sentBytes = 0, i, bars, size_kb, send_amt;
		uint8_t buf[DOWNLOAD_BUFFER_SIZE];

		size_kb = size / 1024;
		fflush(stdout);
		system("setterm -cursor off");
		send_amt = size;
		sb = send_amt < DOWNLOAD_BUFFER_SIZE ? send_amt : DOWNLOAD_BUFFER_SIZE;
		while (send_amt) {
			if ((sb = read(fd, buf, sb))  < 0) {
				printf("\nSend failed: [%s]\n", strerror(errno));
				flush_socket();
				close(fd);
				return false;
			}
			if(write(socket_fd, buf, sb) != sb) {
				printf("\nSend failed: [%s]\n", strerror(errno));
				flush_socket();
				close(fd);
				return false;
			}
			sentBytes += sb;
			
			if (sb) {
				printf("\r[");
				bars = sentBytes * 50 / size;
				for(i = 0; i < 50; i++) {
					if (i < bars)
						printf("=");
					else
						printf(" ");
				}
				printf("] %ld/%ld KB", sentBytes , size);
				fflush(stdout);
			}
			send_amt = size - sentBytes;
			sb = send_amt < DOWNLOAD_BUFFER_SIZE ? send_amt : DOWNLOAD_BUFFER_SIZE;
		}
		printf("\n");
		system("setterm -cursor on");

		//printf("File received successfully\n");

		close(fd);
		return true;
	}

}
