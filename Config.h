#ifndef CONFIG_H
#define CONFIG_H

#include "Util.h"
#include <map>
class Config
{
private:
	std::map<std::string, std::vector<std::string> > config;
	std::string configfilename;

	struct CombiKey
	{
		std::string base;
		uint32_t id;
		std::string prop;

		CombiKey(){id=-1;}
		~CombiKey(){}
	};

	//returns default (base "", id -1, prop "") if key is not a CombiKey
	CombiKey GetCombiKey(std::string keyname)
	{
		CombiKey cs;
		size_t dotplace = keyname.find('.');
		if (dotplace == std::string::npos || dotplace == keyname.length()-1)
			return cs;
		std::string base_plus_id = keyname.substr(0,dotplace);
		std::string prop = keyname.substr(dotplace+1);
		if (base_plus_id == "" || prop == "")
			return cs;
		size_t idplace = base_plus_id.find_last_not_of("0123456789");
		if (idplace == base_plus_id.length()-1)
			return cs;
		std::string base = base_plus_id.substr(0,idplace+1);
		if (base == "")
			return cs;
		uint32_t id = FromString<uint32_t>(base_plus_id.substr(idplace+1));
		cs.base = base;
		cs.id = id;
		cs.prop = prop;
		return cs;
	}

public:
	//function disabled because of "include" functionality
	//void Save(string filename);
	void Load(std::string filename, std::vector<std::string> included_already = std::vector<std::string>());
	void Clear();

	template<typename T>
	void SetValue(std::string key, uint32_t index, T val)
	{
		if (config[key].size() < index)
			return;
		if (config[key].size() == index)
			config[key].push_back(ToString(val));
		else
			config[key][index] = ToString(val);
	}

	template<typename T>
	void SetCombiValue(std::string base, uint32_t id, std::string prop, uint32_t index, T val)
	{
		string key = base + ToString(id) + "." + prop;
		SetValue<T>(key, index, val);
	}

	template<typename T>
	T GetValue(std::string key, uint32_t index = 0)
	{
		if (config[key].size() <= index)
			return T();
		return FromString<T>(config[key][index]);
	}

	template<typename T>
	T GetCombiValue(std::string base, uint32_t id, std::string prop, uint32_t index = 0)
	{
		std::string key = base + ToString(id) + "." + prop;
		return GetValue<T>(key, index);
	}

	uint32_t GetValueCount(std::string key)
	{
		return (uint32_t)config[key].size();
	}
};
extern Config config;
#endif
