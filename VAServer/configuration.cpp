#include "configuration.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace std;

Configuration::Configuration(string configFile) {
	boost::property_tree::ptree pt;

	read_xml(configFile, pt);

    for (auto &setting : pt.get_child("config.settings")) {
		string name = setting.second.get<string>("<xmlattr>.name");

		if (name == "transfer_speed") {
			transferSpeed = setting.second.get<double>("<xmlattr>.value");
		}
        else if (name == "mobile_speed") {
            mobileSpeed = setting.second.get<double>("<xmlattr>.value");
        }
        else if (name == "server_speed") {
            serverSpeed = setting.second.get<double>("<xmlattr>.value");
        }
	}

    for (auto &dev : pt.get_child("config.devices")) {
        devices.push_back(dev.second.get<string>("<xmlattr>.ip"));
    }

    for (auto &video : pt.get_child("config.videos")) {
        Video *vid = new Video();
        vid->name = video.second.get<string>("<xmlattr>.name");
        for (auto &classes : video.second.get_child("")) {
            if (classes.first != "<xmlattr>") {
                vid->add_tag(classes.second.get<int>("<xmlattr>.index"));
            }
        }
        videos[vid->name] = vid;
    }
}

Configuration::~Configuration() {
	for (map<string, Video*>::iterator it = videos.begin(); it != videos.end(); it++)
		delete it->second;
}

Video *Configuration::getVideo(const std::string &name) {
	map<string,Video*>::iterator it = videos.find(name);
	if (it != videos.end()) {
		return it->second;
	}
	
	return NULL;
}
