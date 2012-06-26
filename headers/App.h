#ifndef APP_H
#define APP_H

#include "Curl.h"
#include "../headers/AppOpenCL.h"
#include "../headers/CPUMiner.h"
#include "../headers/RSHash.h"
class App
{
private:
	struct ServerData
	{
		uint32_t current_id;
		uint32_t last_tried;
	};

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
};

#endif
