#ifndef CURLMUNACPP
#define CURLMUNACPP

#include "../headers/ServerSettings.h"

class Curl
{
private:
	enum EXEC_TYPE
	{
		GETWORK,
		GETWORK_LP,
		TESTWORK,
	};

	std::string Execute(ServerSettings& s, Curl::EXEC_TYPE type, std::string work, std::string path, uint32_t timeout);
	void Execute_BTC(void* curl, Curl::EXEC_TYPE type, std::string work, std::string path, uint32_t timeout);

public:

	Curl() {}
	~Curl() {}

	static void GlobalInit();
	static void GlobalQuit();

	void* Init();
	void Quit(void* curl);

	std::string GetWork_LP(ServerSettings& s, std::string path="", uint32_t timeout = 60);
	std::string GetWork(ServerSettings& s, std::string path="", uint32_t timeout = 5);
	std::string TestWork(ServerSettings& s, std::string work);
};

#endif
