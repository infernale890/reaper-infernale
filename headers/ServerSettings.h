#ifndef SERVERSETTINGS_H
#define SERVERSETTINGS_H

class ServerSettings
{
public:
	std::string host,user,pass,proxy;
	uint16_t port;

	std::string ToString()
	{
		return std::string("http://") + user + ":" + pass  + "@" + host + ":" + ::ToString(port) + "/";
	}
};

#endif
