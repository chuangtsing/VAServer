#ifndef VIDEO_H
#define VIDEO_H

#include <string>
#include <vector>
#include "protobuf/vanet_pb.pb.h"

typedef enum ProcessStatus {
    PENDING, PROCESSING_MOBILE, PROCESSING_SERVER, UPLOADING, DONE
} ProcessStatus;

namespace vanet {
    class MobileDevice;
}


class Video {
	private:
		bool loc;
        std::vector<int> tags;

	public:
		Video() { loc = false; };
        Video(const vanet_pb::VideoInfo &info, vanet::MobileDevice *dev);

		bool has_loc() { return loc; };

        bool has_tags() { return tags.empty() ? false : true; };
        const std::vector<int> &get_tags() { return tags; };
        void add_tag(int tag) { tags.push_back(tag); };
        void set_tags(std::vector<int> &new_tags) { tags = new_tags; };
        bool is_match(const std::vector<int> classes);

		std::string name;
		std::string path;
		std::string local_path;
		std::string timestamp;
		uint64_t size;
		uint64_t duration;
		uint32_t bitrate;
		std::string mime;
		double loc_lat;
		double loc_long;
		uint32_t width;
		uint32_t height;
		uint32_t rotation;
		uint32_t frames_processed;
		uint32_t frames_total;
        vanet_pb::ProcessMode mode;
        ProcessStatus status;
        vanet::MobileDevice *dev;
        int index;
};
#endif
