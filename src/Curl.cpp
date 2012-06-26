#include "../headers/Global.h"
#include "../headers/Curl.h"

#include "curl/curl.h"
#include "../headers/Util.h"

void Curl::GlobalInit()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

void Curl::GlobalQuit()
{
	curl_global_cleanup();
}

void* Curl::Init()
{
	void* curl = curl_easy_init();
	if (curl == NULL)
	{
		throw std::string("libcurl initialization failure");
	}
	return curl;
}

void Curl::Quit(void* curl)
{
	if (curl != NULL)
	{
		curl_easy_cleanup(curl);
	}
}

size_t ResponseCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	std::string* getworksentdata = (std::string*)data;
	try 	
	{ 	
		for(uint32_t i=0; i<size*nmemb; ++i) 	
		{ 		
			if(ptr!=NULL && data!=NULL) 		
			{ 			
				int8_t c = ((char*)ptr)[i]; 			
				getworksentdata->push_back(c); 		
			} 	
		}
	} 	
	catch(std::exception s) 	
	{ 		
		std::cout << "(1) Error: " << s.what() << std::endl; 	
	}
	return size*nmemb; 
}

std::string longpoll_url;
bool longpoll_active=false;
size_t HeaderCallback( void *ptr, size_t size, size_t nmemb, void *userdata)
{
	std::string hdr;
	for(uint32_t i=0; i<size*nmemb; ++i)
	{
		int8_t c = ((char*)ptr)[i];
		hdr.push_back(c);
	}
	if (!longpoll_active && hdr.length() >= 0x10 && hdr.substr(0,0xF) == "X-Long-Polling:")
	{
		longpoll_url = hdr.substr(0x10);
		longpoll_url = longpoll_url.substr(0, longpoll_url.length()-2);
		std::cout << "Longpoll url -->" << longpoll_url << "<-- " << std::endl;
		longpoll_active = true;
	}
	return size*nmemb;
}

std::string Curl::GetWork_LP(ServerSettings& s, std::string path, uint32_t timeout)
{
	return Execute(s,GETWORK_LP, "", path, timeout);
}

std::string Curl::GetWork(ServerSettings& s, std::string path, uint32_t timeout)
{
	return Execute(s,GETWORK, "", path, timeout);
}

std::string Curl::TestWork(ServerSettings& s, std::string work)
{
	return Execute(s,TESTWORK, work, "", 30);
}

std::string Curl::Execute(ServerSettings& s, Curl::EXEC_TYPE type, std::string work, std::string path, uint32_t timeout)
{
	void* curl = Init();
	std::string responsedata;
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_URL, ("http://" + s.host + path).c_str());
	curl_easy_setopt(curl, CURLOPT_USERPWD, (s.user + ":" + s.pass).c_str());
	curl_easy_setopt(curl, CURLOPT_PORT, s.port);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ResponseCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responsedata);

	if (s.proxy != "")
		curl_easy_setopt(curl, CURLOPT_PROXY, s.proxy.c_str());

	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);

	Execute_BTC(curl,type,work,path,timeout);
	Quit(curl);
	return responsedata;
}

void Curl::Execute_BTC(void* curl, Curl::EXEC_TYPE type, std::string work, std::string path, uint32_t timeout)
{
	curl_slist* headerlist = NULL;
	headerlist = curl_slist_append(headerlist, "Accept: */*");
	headerlist = curl_slist_append(headerlist, "Content-Type: application/json");
	headerlist = curl_slist_append(headerlist, "User-Agent: reaper/" REAPER_VERSION);
	headerlist = curl_slist_append(headerlist, "User-Agent: jansson 1.3");
	headerlist = curl_slist_append(headerlist, "X-Mining-Extensions: midstate rollntime");

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
	
	if (type == GETWORK_LP)
	{
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, NULL);
		curl_easy_setopt(curl, CURLOPT_POST, 0);
	}
	else if (type == GETWORK)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, "{\"method\":\"getwork\",\"params\":[],\"id\":\"1\"}");
	}
	else if (type == TESTWORK)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		std::string str = "{\"method\":\"getwork\",\"id\":\"1\",\"params\":[\"" + work + "\"]}";
		curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, str.c_str());
	}
	else
	{
		std::cout << "Unknown case " << (int)type << " in Curl::Execute()" << std::endl;
	}
	CURLcode code = curl_easy_perform(curl);
	if(code != CURLE_OK && type != TESTWORK)
	{
		if (code == CURLE_COULDNT_CONNECT)
		{
			std::cout << "Could not connect. Server down?" << std::endl;
		}
		else
		{
			std::cout << "Error " << code << " getting work. See http://curl.haxx.se/libcurl/c/libcurl-errors.html for error code explanations." << std::endl;
		}
	}
	curl_slist_free_all(headerlist);
}
