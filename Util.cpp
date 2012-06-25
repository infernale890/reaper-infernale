#include "Util.h"


template<> 
bool FromString<bool>(std::string key)
{
	if (key == "true" || key == "on" || key == "yes")
		return true;
	if (key == "false" || key == "off" || key == "no")
		return false;
	if (key == "")
		return false;
	return false;
}

std::string ToString(bool key, std::string truestring, std::string falsestring)
{
	if (key)
		return truestring;
	else
		return falsestring;
}

template<> int32_t FromString<int>(std::string key)
{
	int32_t ret=0;
	std::stringstream sstr(key);
	if (key != "")
		sstr >> ret;
	return ret;
}





std::string humantime()
{
	time_t rawtime;
	int8_t formattedtime[100];
	tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime((char*)formattedtime, 100, "%Y-%m-%d %H:%M:%S ", timeinfo);
	std::string ret = (char*)formattedtime;
	return ret;
}


std::vector<std::string> Explode(std::string s, int8_t delim)
{
	std::vector<std::string> returner;
	std::stringstream ss(s);
	std::string temp;
	returner.clear();

	if (s.length() == 0)
		return returner;
	
	while(getline(ss, temp, (char)delim))
	{
		if (temp.size() > 0)
			returner.push_back(temp);
	}
	return returner;
}


std::vector<uint8_t> HexStringToVector(std::string str)
{
	std::vector<uint8_t> ret;
	ret.assign(str.length()/2, 0);
	for(uint32_t i=0; i<str.length(); i+=2)
	{
		ret[i/2] = _HexToChar(str[i+0], str[i+1]);
	}
	return ret;
}
std::string VectorToHexString(std::vector<uint8_t> vec)
{
	std::string ret;
	for(uint32_t i=0; i<vec.size(); i++)
	{
		ret += CharToHex(vec[i]);
	}
	return ret;
}