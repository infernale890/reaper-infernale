#include "../headers/Global.h"
#include "../headers/App.h"

#include <stdexcept>

#include "../headers/AppOpenCL.h"
#include "../headers/Util.h"

int32_t main(int32_t argc, char* argv[])
{
	try
	{
		std::vector<std::string> args;
		for(int32_t i=0; i<argc; ++i)
		{
			args.push_back(argv[i]);
		}
		App app;
		app.Main(args);
	}
	catch(std::string s)
	{
		std::cout << humantime() << "Error: " << s << std::endl;
	}
	catch(std::runtime_error s)
	{
		std::cout << humantime() << "Runtime error: " << s.what() << std::endl;
	}
	catch(std::exception s)
	{
		std::cout << humantime() << "Exception: " << s.what() << std::endl;
	}
	catch(...)
	{
		std::cout << humantime() << "Unknown error." << std::endl;
	}
}
