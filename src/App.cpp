#include "../headers/Global.h"
#include "../headers/App.h"
#include "../headers/Util.h"

#include "../headers/json/json.h"


#include "../headers/AppOpenCL.h"

#ifndef CPU_MINING_ONLY
extern std::vector<_clState> GPUstates;
#endif
extern std::vector<Reap_CPU_param> CPUstates;


void DoubleSHA256(uint32_t* output2, uint32_t* workdata, uint32_t* midstate);


#ifdef WIN32
#undef SetPort
#else
void Wait_ms(uint32_t n);
#endif

#include "../headers/Config.h"

Config config;
GlobalConfs globalconfs;

uint64_t shares_valid = 0;
uint64_t shares_invalid = 0;
uint64_t shares_hwvalid = 0;
uint64_t shares_hwinvalid = 0;
uint64_t cpu_shares_hwvalid = 0;
uint64_t cpu_shares_hwinvalid = 0;
clock_t current_work_time = 0;
ServerSettings server;
extern Work current_work;

SHARETEST_VALUE ShareTest_BTC(uint32_t* workdata, uint32_t* target);
SHARETEST_VALUE scanhash_scrypt(uint8_t *pdata, uint8_t* scratchbuf, const uint8_t *ptarget);

bool getwork_now = false;
bool dont_check_shares = false;
void SubmitShare(Share& w, uint8_t* scratchbuf)
{
	Curl *curl = NULL;
	if (w.data.size()%128 != 0)
	{
		std::cout << "SubmitShare: Size of share is " << w.data.size() << ", should be a multiple of 128" << std::endl;
		return;
	}
	try
	{
		if (!dont_check_shares)
		{
			SHARETEST_VALUE sharevalid = scanhash_scrypt(&w.data[0],scratchbuf,&w.target[0]);
			if (sharevalid == ST_HNOTZERO)
				shares_hwinvalid++;
			else if (sharevalid == ST_MORETHANTARGET)
			{
				shares_hwvalid++;
				return;
			}
			else
				shares_hwvalid++;
		}
		std::string str = VectorToHexString(w.data);
		std::string ret = curl->TestWork(server,str);
		std::cout << ">>" << ret; 
		Json::Value root;
		Json::Reader reader;
		bool parse_success = reader.parse(ret, root);
		if (parse_success)
		{
			Json::Value result = root.get("result", "null");
			if (result.isObject())
			{
				Json::Value work = result.get("work", "null");
				if (work.isArray())
				{
					for(uint32_t i=0; i<work.size(); ++i)
					{
						Json::Value innerobj = work.get(Json::Value::UInt(i), "");
						if (innerobj.isObject())
						{
							Json::Value share_valid = innerobj.get("share_valid", "null");
							if (share_valid.isBool())
							{
								if (share_valid.asBool())
								{
									++shares_valid;
								}
								else
								{
									getwork_now = true;
									++shares_invalid;
								}
							}
							//Json::Value block_valid = innerobj.get("block_valid");
						}
					}
				}
			}
			else if (result.isBool())
			{
				if (result.asBool())
					++shares_valid;
				else
				{
					getwork_now = true;
					++shares_invalid;
				}
			}
			else
			{
				getwork_now = true;
				std::cout << "Stupid response from server." << std::endl;
			}
		}
		else
		{
			getwork_now = true;
			std::cout << "Weird response from server." << std::endl;
		}
	}
	catch(std::exception s)
	{
		std::cout << "(3) Error: " << s.what() << std::endl;
	}
}

bool sharethread_active;
void* ShareThread(void* param)
{
	std::cout << "Share thread started" << std::endl;
	uint32_t currentserverid = 0;
	std::vector<uint8_t> btcpadding(48,0);
	btcpadding[3] = 0x80;
	btcpadding[44] = 0x80;
	btcpadding[45] = 0x02;

	uint8_t scratchbuf[131072];
	Share s;
	while(!shutdown_now)
	{
		sharethread_active = true;
		Wait_ms(100);
#ifndef CPU_MINING_ONLY
		foreachgpu()
		{
			sharethread_active = true;
			if (!it->shares_available)
				continue;
			pthread_mutex_lock(&it->share_mutex);
			if (it->shares.empty())
			{
				it->shares_available = false;
				pthread_mutex_unlock(&it->share_mutex);
				continue;
			}
			s = it->shares.front();
			it->shares.pop_front();
			if (it->shares.empty())
				it->shares_available = false;
			pthread_mutex_unlock(&it->share_mutex);
		    currentserverid = s.server_id;
			{
				if (s.data.size() == 80)
					s.data.insert(s.data.end(),btcpadding.begin(),btcpadding.end());
				SubmitShare(s,scratchbuf);
			}
		}
#endif
		foreachcpu()
		{
			sharethread_active = true;
			if (!it->shares_available)
				continue;
			pthread_mutex_lock(&it->share_mutex);
			if (it->shares.empty())
			{
				pthread_mutex_unlock(&it->share_mutex);
				continue;
			}
			s = it->shares.front();
			it->shares.pop_front();
			if (it->shares.empty())
				it->shares_available = false;
			pthread_mutex_unlock(&it->share_mutex);
			currentserverid = s.server_id;
			{
				if (s.data.size() == 80)
					s.data.insert(s.data.end(),btcpadding.begin(),btcpadding.end());
				SubmitShare(s,scratchbuf);
			}
		}
	}
	pthread_exit(NULL);
	return NULL;
}

extern std::string longpoll_url;
extern bool longpoll_active;

struct LongPollThreadParams
{
	Curl* curl;
	App* app;
};

void* LongPollThread(void* param)
{
	LongPollThreadParams* p = (LongPollThreadParams*)param; 

	Curl* curl = p->curl;
	
	std::string LP_url = longpoll_url;
	std::string LP_path;

	std::cout << "Long polling URL: [" << LP_url << "]. trying to parse." << std::endl;
	clock_t lastcall = 0;

	{//parsing LP address
		std::vector<std::string> exploded = Explode(LP_url, '/');
		if (exploded.size() >= 2 && exploded[0] == "http:")
		{
			std::vector<std::string> exploded2 = Explode(exploded[1], ':');
			if (exploded2.size() != 2)
				goto couldnt_parse;
			std::cout << "LP Host: " << exploded2[0] << std::endl;
			//curl.SetHost(exploded2[0]);
			std::cout << "LP Port: " << exploded2[1] << std::endl;
			//curl.SetPort(exploded2[1]);
			if (exploded.size() <= 2)
				LP_path = '/';
			else
				LP_path = "/" + exploded[2];
			std::cout << "LP Path: " << LP_path << std::endl;
		}
		else if (LP_url.length() > 0 && LP_url[0] == '/')
		{
			LP_path = LP_url;
			std::cout << "LP Path: " << LP_path << std::endl;
		}
		else
		{
			goto couldnt_parse;
		}
	}

	while(!shutdown_now)
	{
		clock_t ticks = ticker();
		if (ticks-lastcall < 5000)
		{
			Wait_ms(ticks-lastcall);
		}
		lastcall = ticks;
		std::string r = curl->GetWork(server, LP_path, 60);
#ifdef _DEBUG_MSG_
		std::cout << "Got LP" << std::endl;
#endif
		p->app->Parse(r);
	}
	pthread_exit(NULL);
	return NULL;

couldnt_parse:
	std::cout << "Couldn't parse long polling URL [" << LP_url << "]. turning LP off." << std::endl;
	pthread_exit(NULL);
	return NULL;
}

bool shutdown_now=false;
void* ShutdownThread(void* param)
{
	std::cout << "Press [Q] and [Enter] to quit" << std::endl;
	while(shutdown_now == false)
	{
		std::string s;
		std::cin >> s;
		if (s == "q" || s == "Q")
			shutdown_now = true;
	}
	std::cout << "Quitting." << std::endl;
	pthread_exit(NULL);
	return NULL;
}



void App::SetupCurrency()
{
	Coin c;
	{
		c.local_worksize = config.GetValue<uint32_t>("worksize");
		{
			if (config.GetValue<std::string>("aggression") == "max")
			{
				c.global_worksize = 1<<11;
				c.max_aggression = true;
			}
			else
			{
				c.global_worksize = (1<<config.GetValue<uint32_t>("aggression"));
				c.max_aggression = false;
			}
			c.global_worksize /= c.local_worksize;
			c.global_worksize *= c.local_worksize;
		}
		c.threads_per_gpu = config.GetValue<uint32_t>("threads_per_gpu");
		c.cputhreads = config.GetValue<uint32_t>("cpu_mining_threads");
		c.share_threads = config.GetValue<uint32_t>("sharethreads");

		c.host = config.GetValue<std::string>("host");
		c.port = config.GetValue<std::string>("port");
		c.user = config.GetValue<std::string>("user");
		c.pass = config.GetValue<std::string>("pass");
		c.proxy = config.GetValue<std::string>("proxy");

		if (c.local_worksize > c.global_worksize)
			c.global_worksize = c.local_worksize;
	}

	std::cout << "I'm now mining litecoins!" << std::endl;

	globalconfs.coin = c;
	if (globalconfs.coin.host == "" ||
		globalconfs.coin.port == "" ||
		globalconfs.coin.user == "" ||
		globalconfs.coin.pass == "")
		throw std::string("Config is missing one of host/port/user/pass.");

	globalconfs.coin.sharekhs = pow(2.0,16.0)/1000.0;
	if (globalconfs.coin.cputhreads == 0 && globalconfs.coin.threads_per_gpu == 0)
	{
		throw std::string("No CPU or GPU mining threads.. please set either cpu_mining_threads or threads_per_gpu to something other than 0.");
	}

	server.host = globalconfs.coin.host;
	server.port = FromString<uint16_t>(globalconfs.coin.port);
	server.user = globalconfs.coin.user;
	server.pass = globalconfs.coin.pass;
	server.proxy = globalconfs.coin.proxy;
}



void App::Main(std::vector<std::string> args)
{
	std::cout << "\\||||||||||||||||||||||||/" << std::endl;
	std::cout << "-  Reaper " << REAPER_VERSION << " " << REAPER_PLATFORM << "     -" << std::endl;
	std::cout << "-       BETA 5           -" << std::endl;
	std::cout << "-   coded by mtrlt       -" << std::endl;
	std::cout << "- Edited by infernale890 -" << std::endl;
	std::cout << "/||||||||||||||||||||||||\\" << std::endl;
	std::cout << std::endl;

	std::string config_name = "litecoin.conf";
	if (args.size() > 2)
	{
		std::cout << "Please use config files to set host/port/user/pass" << std::endl;
		return;
	}
	if (args.size() == 2)
		config_name = args[1];
	getworks = 0;
	config.Load(config_name);
	SetupCurrency();
	dont_check_shares = config.GetValue<bool>("dont_check_shares");
	Wait_ms(100);
	nickbase = globalconfs.coin.user;

	globalconfs.save_binaries = config.GetValue<bool>("save_binaries");
	uint32_t numdevices = config.GetValueCount("device");
	for(uint32_t i=0; i<numdevices; ++i)
		globalconfs.devices.push_back(config.GetValue<uint32_t>("device", i));

#ifdef CPU_MINING_ONLY
	if (globalconfs.coin.cputhreads == 0)
	{
		throw std::string("cpu_mining_threads is zero. Nothing to do, quitting.");
	}
#endif
	globalconfs.platform = config.GetValue<uint32_t>("platform");

	BlockHash_Init();
	current_work.old = true;
	current_work.time = 0;

	Curl::GlobalInit();

	pthread_t sharethread;
	pthread_create(&sharethread, NULL, ShareThread, &curl);
	if (globalconfs.coin.share_threads)
	{
		for(uint32_t i=1; i< globalconfs.coin.share_threads; ++i)
			pthread_create(&sharethread, NULL, ShareThread, &curl);
	}

	opencl.Init();
	cpuminer.Init();
	Parse(curl.GetWork(server));

	int32_t work_update_period_ms = config.GetValue<uint32_t>("getwork_rate");

	if (work_update_period_ms == 0)
		work_update_period_ms = 7500;

	pthread_t longpollthread;
	LongPollThreadParams lp_params;
	if (longpoll_active)
	{
		std::cout << "Activating long polling." << std::endl;
		lp_params.app = this;
		lp_params.curl = &curl;
		pthread_create(&longpollthread, NULL, LongPollThread, &lp_params);
	}

	if (config.GetValue<bool>("enable_graceful_shutdown"))
	{
		pthread_t shutdownthread;
		pthread_create(&shutdownthread, NULL, ShutdownThread, NULL);
	}

	clock_t ticks = ticker();
	clock_t starttime = ticker();
	workupdate = ticker();

	clock_t sharethread_update_time = ticker();

	while(!shutdown_now)
	{
		Wait_ms(100);
		clock_t timeclock = ticker();
		if (timeclock - current_work_time >= WORK_EXPIRE_TIME_SEC*1000)
		{
			if (!current_work.old)
			{
				std::cout << humantime() << "Work too old... waiting for getwork.    " << std::endl;
			}
			current_work.old = true;
		}
		if (sharethread_active)
		{
			sharethread_active = false;
			sharethread_update_time = timeclock;
		}
		if (timeclock-sharethread_update_time >= SHARE_THREAD_RESTART_THRESHOLD_SEC*1000)
		{
			std::cout << humantime() << "Share thread messed up. Starting another one.   " << std::endl;
			pthread_create(&sharethread, NULL, ShareThread, &curl);
		}
		if (getwork_now || timeclock - workupdate >= work_update_period_ms)
		{
			uint32_t timmii = ticker();
			Parse(curl.GetWork(server));
			timmii = ticker()-timmii;
			if (timmii > 5000)
				std::cout << "Getwork took " << timmii/1000.0 << " s!  " << std::endl;
			getwork_now = false;
		}
		if (timeclock - ticks >= 1000)
		{
			uint64_t totalhashesGPU=0;
#ifndef CPU_MINING_ONLY
			foreachgpu()
			{
				totalhashesGPU += it->hashes;
			}
#endif
			uint64_t totalhashesCPU=0;
			foreachcpu()
			{
				totalhashesCPU += it->hashes;
			}

			ticks += (timeclock-ticks)/1000*1000;
			float stalepercent = 0.0f;
			if (shares_valid+shares_invalid != 0)
				stalepercent = 100.0f*float(shares_invalid)/float(shares_invalid+shares_valid);
			if (ticks-starttime == 0)
				std::cout << std::dec << "   ??? kH/s, shares: " << shares_valid << "|" << shares_invalid << ", stale " << stalepercent << "%, " << (ticks-starttime)/1000 << "s    \r";
			else
			{
				std::stringstream stream;
				stream.precision(4);
				if (totalhashesGPU != 0)
				{
					stream << "GPU ";
					double spd = double(totalhashesGPU)/(ticks-starttime);
					if (spd >= 1000000.0)
						stream << spd/1000000.0 << "GH/s, ";
					else if (spd >= 1000.0)
						stream << spd/1000.0 << "MH/s, ";
					else
						stream << spd << "kH/s, ";
				}
				if (totalhashesCPU != 0)
				{
					stream << "CPU " << double(totalhashesCPU)/(ticks-starttime) << "kH/s, ";
				}
				stream << "shares: " << shares_valid << "|" << shares_invalid << ", stale " << stalepercent << "%, ";
				float hwpercent = 100.0f*float(shares_hwinvalid)/float(shares_hwinvalid+shares_hwvalid);
				if (shares_hwinvalid+shares_hwvalid > 0)
					stream << "GPU errors: " << hwpercent << "%, ";
				float cpuhwpercent = 100.0f*float(cpu_shares_hwinvalid)/float(cpu_shares_hwinvalid+cpu_shares_hwvalid);
				if (cpu_shares_hwinvalid+cpu_shares_hwvalid > 0)
					stream << "CPU errors: " << cpuhwpercent << "%, ";
				std::cout << std::dec << stream.str() << "~" << 1000.0*(shares_hwvalid+cpu_shares_hwvalid)/double(ticks-starttime)*globalconfs.coin.sharekhs << " kH/s, " << (ticks-starttime)/1000 << "s  \r";
				std::cout.flush();
			}
		}
	}
	cpuminer.Quit();
	opencl.Quit();
	Curl::GlobalQuit();
	BlockHash_DeInit();
}

bool targetprinted=false;

pthread_mutex_t current_work_mutex = PTHREAD_MUTEX_INITIALIZER;
Work current_work;

void App::Parse(std::string data)
{
	workupdate = ticker();
	if (data == "")
	{
		std::cout << humantime() << "Couldn't connect to server. ";
		std::cout << "Trying again in a few seconds... " << std::endl;

		return;
	}

	Parse_BTC(data);
}

void Precalc_BTC(Work& work, uint32_t vectors);
std::vector<uint8_t> CalculateMidstate(std::vector<uint8_t> in);
void App::Parse_BTC(std::string data)
{
	Json::Value root, result, error;
	Json::Reader reader;
	bool parsing_successful = reader.parse( data, root );
	if (!parsing_successful)
	{
		goto got_error;
	}

	result = root.get("result", "null");
	error = root.get("error", "null");
	
	if (result.isObject())
	{
		Json::Value::Members members = result.getMemberNames();
		uint32_t neededmembers=0;
		bool has_midstate = false;
		for(Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
		{
			if (*it == "data")
				++neededmembers;
			if (*it == "midstate")
				has_midstate = true;
		}
		if (neededmembers != 1 || !result["data"].isString())
		{
			goto got_error;
		}

		++getworks;
		Work newwork;
		if (has_midstate)
			newwork.midstate = HexStringToVector(result["midstate"].asString());
		else
			newwork.midstate = CalculateMidstate(HexStringToVector(result["data"].asString().substr(0,128)));
		newwork.data = HexStringToVector(result["data"].asString().substr(0, 160));
		newwork.old = false;
		newwork.time = ticker();
		current_work_time = ticker();

		if (!targetprinted)
		{
			targetprinted = true;
			std::cout << result["target"].asString() << std::endl;
		}

		{
			std::vector<uint8_t> targetuint8_t = HexStringToVector(result["target"].asString());
			newwork.target_share.assign(32,0);
			for(uint32_t i=0; i<8; ++i)
			{
				uint32_t number = (((uint32_t*)(&targetuint8_t[0]))[i]);
				newwork.target_share[4*i] = number;
				newwork.target_share[4*i+1] = number>>8;
				newwork.target_share[4*i+2] = number>>16;
				newwork.target_share[4*i+3] = number>>24;
			}
			newwork.server_id = 0;
		}

		Precalc_BTC(newwork,opencl.GetVectorSize());
		current_work.time = ticker();
		pthread_mutex_lock(&current_work_mutex);
		current_work = newwork;
		pthread_mutex_unlock(&current_work_mutex);
		return;
	}
	else if (!error.isNull())
	{
		std::cout << error.asString() << std::endl;
		std::cout << "Code " << error["code"].asInt() << ", \"" << error["message"].asString() << "\"" << std::endl;
	}
got_error:
	std::cout << "Error with server: " << data << std::endl;
	return;
}
