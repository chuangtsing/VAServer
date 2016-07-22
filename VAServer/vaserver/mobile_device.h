#ifndef MOBILE_DEVICE_H
#define MOBILE_DEVICE_H

#include <string>
#include <vector>
#include <unistd.h>
#include <thread>
#include <queue>
#include <vector>
#include "protobuf/vanet_pb.pb.h"
#include <chrono>
#include "mobile_device.h"
#include <opencv2/core/core.hpp>
#include <mutex>
#include <set>
#include "video.h"

//extern std::queue<std::pair<vanet::Video*, vanet::MobileDevice*>> proc_queue;
//extern std::vector<vanet::Video*> finished_vec;

namespace vanet {


typedef std::chrono::high_resolution_clock Time;
typedef std::chrono::milliseconds MS;
typedef std::chrono::duration<double> Duration;
typedef std::chrono::system_clock::time_point TimePoint;

class MobileDevice {

	public:
        MobileDevice(const std::string &ip, bool connect);
        MobileDevice(const std::string &ip, int socket);
        ~MobileDevice();

        void set_ip(const std::string &ip) { this->ip = ip; };
		void set_socket(int socket_fd) { this->socket_fd = socket_fd; };
        std::string get_ip();
        void add_video(Video *vid);
        Video *get_video(const std::string &path);
        const std::vector<Video*> &get_videos();
        void removeRemaining(Video *vid) { remainingVideos.erase(vid); };
        bool remainingEmpty() { return remainingVideos.empty(); };
        int getRemaining() { return remainingVideos.size(); };
        void setVideoStatus(Video *vid, ProcessStatus status);
        void lock_videos();
        void unlock_videos();
		int get_socket() { return socket_fd; };
		bool establish_connection();
		int open_connection();
		bool send_message(const vanet_pb::ServerMessage&);
		bool receive_message(vanet_pb::ClientMessage&);
		void start_receiver();
		void stop_receiver();
		bool send_resources(const google::protobuf::RepeatedField<int>& res);
		bool send_file(int fd, long size);
		long receive_video(std::string &path, const vanet_pb::ClientMessage &msg);
		char* receive_frame(const vanet_pb::ClientMessage &msg);
		cv::Mat receive_frame_mat(const vanet_pb::ClientMessage &msg);
		uint32_t read_varint();
		void flush_socket();
        int index;

	private:
		long mac_addr;
		std::string ip;
		int socket_fd;
        bool status;
		std::mutex vids_lck;
		std::thread receiver;
		void receiver_func();
		bool receiver_shutdown;
        std::vector<Video*> videos;
        std::set<Video*> remainingVideos;

	};
}

#endif
