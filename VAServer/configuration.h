#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "video.h"
#include <map>

class Configuration {
	public:
		Configuration(std::string configFile);
		~Configuration();
		Video *getVideo(const std::string &name);
        int getTransferSpeed() { return transferSpeed; };
        int getMobileSpeed() { return mobileSpeed; };
        int getServerSpeed() { return serverSpeed; };
        const std::vector<std::string> &getDevices() { return devices; };

	private:
		std::map<std::string, Video*> videos;
        std::vector<std::string> devices;
		double transferSpeed;
        double mobileSpeed;
        double serverSpeed;

};

#endif
