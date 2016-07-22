#include "video.h"
#include "mobile_device.h"

using namespace std;

Video::Video(const vanet_pb::VideoInfo &info, vanet::MobileDevice *dev) {
    name = info.name();
    path = info.path();
    timestamp = info.timestamp();
    size = info.size();
    duration = info.duration();
    bitrate = info.bitrate();
    mime = info.mime();
    if (loc = info.has_loc_lat() && info.has_loc_long()) {
        loc_lat = info.loc_lat();
        loc_long = info.loc_long();
	}
    width = info.width();
    height = info.height();
    rotation = info.rotation();
    frames_processed = info.frames_processed();
    frames_total = info.frames_total();
    if (info.tags_size() > 0) {
        for (auto tag : info.tags())
            tags.push_back(tag);
	}
    this->dev = dev;
    status = PENDING;
}

bool Video::is_match(const std::vector<int> classes) {
    for (int clazz : classes) {
        for (int tag : tags) {
            if (clazz == tag)
                return true;
        }
    }
    return false;
}
