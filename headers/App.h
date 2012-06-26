#ifndef APP_H
#define APP_H

#include "Curl.h"
#include "AppOpenCL.h"
#include "CPUMiner.h"

class App
{
private:
	struct ServerData
	{
		uint32_t current_id;
		uint32_t last_tried;
	};

	uint32_t current_server_id;
	std::string nickbase;

	Curl curl;
	OpenCL opencl;
	CPUMiner cpuminer;

	clock_t workupdate;

	uint32_t getworks;

	void SetupCurrency();

	void Parse_SLC(std::string data);
	void Parse_BTC(std::string data);
public:
	void Main(std::vector<std::string> args);
	void Parse(std::string data);
	void LoadServers();
};

#endif
