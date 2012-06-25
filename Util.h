#ifndef UTIL_H
#define UTIL_H
#include <sstream>
#include <cstdint>
#include <vector>
#include <ctime>
static const char* hextable[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f"};
#define rotl(x,y) ((x<<y)|(x>>(32-y)))
#define rotr(x,y) ((x>>y)|(x<<(32-y)))
#define Ch(x, y, z) (z ^ (x & (y ^ z)))
#define Ma(x, y, z) ((y & z) | (x & (y | z)))
//#define Ma(x ,y, z)   (((y & z) | x) & (y | z))
#define SHR(x, n)	(x >> n)
#define EndianSwap(n) (((n&0xFF)<<24) | ((n&0xFF00)<<8) | ((n&0xFF0000)>>8) | ((n&0xFF000000)>>24))
#define HexToChar(data) (data <= '9' ? data-'0' : data <= 'Z' ? data-'7' : data-'W')
#define _HexToChar(x,y) (HexToChar(x)*16+HexToChar(y))
#define CharToHex(c) (std::string(hextable[c/16]) + std::string(hextable[c%16]))

#define LineClear() std::cout << "\r\t\t\t\t\r";

template<typename T>
T FromString(std::string key)
{	
	std::stringstream sstr(key);
	T ret;
	sstr >> ret;
	return ret;
}

template<> bool FromString<bool>(std::string key);
template<> int32_t FromString<int>(std::string key);

template<typename T>
std::string ToString(T key)
{	
	std::stringstream sstr;
	sstr << key;
	return sstr.str();
}

std::string ToString(bool key, std::string truestring="yes", std::string falsestring="no");

template<typename T>
void SetValue(uint8_t* pos, T value)
{
	*(T*)pos = value;
}

template<typename T>
T GetValue(uint8_t* binary, uint32_t pos)
{
	return *(T*)&(binary[pos]);
}


#ifdef WIN32
#include "windows.h"
#define ticker() (clock()/(CLOCKS_PER_SEC/1000))
#define Wait_ms(n) Sleep(n)


#else
#include <unistd.h>
#include <sys/time.h>
clock_t ticker()
{
	timeval t;
	gettimeofday(&t, NULL);
	unsigned long long l = ((unsigned long long)(t.tv_sec))*1000 + t.tv_usec/1000;
	return l;
}

void Wait_ms(uint32_t n)
{
	timespec ts;
	ts.tv_sec = n/1000;
	ts.tv_nsec = n*1000000;
	nanosleep(&ts, NULL);
}
#endif


std::string humantime();

#include "ServerSettings.h"
extern std::vector<ServerSettings> servers;

struct Work
{
	uint32_t server_id;
	std::vector<uint8_t> data;
	std::vector<uint8_t> target_share;
	std::vector<uint8_t> midstate;
	std::vector<uint32_t> precalc;
	bool old;
	clock_t time;
	uint64_t ntime_at_getwork;
};

struct Share
{
	uint32_t server_id;
	std::vector<uint8_t> data;
	std::vector<uint8_t> target;
public:
	Share() {}
	//Share(vector<uint8_t> data_, uint32_t server_id_, uint32_t user_id_) : server_id(server_id_),data(data_),user_id(user_id_) {}
	Share(std::vector<uint8_t> data_,std::vector<uint8_t> target_, uint32_t server_id_) : server_id(server_id_),data(data_),target(target_) {}
};

std::vector<std::string> Explode(std::string, int8_t);
std::vector<uint8_t> HexStringToVector(std::string str);
std::string VectorToHexString(std::vector<uint8_t> vec);
#endif
