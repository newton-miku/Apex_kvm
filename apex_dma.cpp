#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <chrono>
#include <iostream>
#include <cfloat>
#include "Game.h"
#include <thread>
#include <fstream>

#include <stdlib.h>
#include <httplib.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

// 下面的是显示ip地址用的
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

Memory apex_mem;
Memory client_mem;

bool firing_range = false;
bool active = true;
uintptr_t aimentity = 0;
uintptr_t tmp_aimentity = 0;
uintptr_t lastaimentity = 0;
bool show_shield = true;
bool ViewWarn = true;
float item_glow_dist = 100.0f;
float max = 999.0f;
float max_dist = 200.0f * 40.0f;
float max_radar_dist = 450.0f;
float max_dist_float = 1200.0f;
int team_player = 0;
int local_team = 0;
float max_fov = 40;
const int toRead = 100;
char mapname[64] = {};
float mapSizeX;
float mapSizeY;
float mapSize;
int aim = 2;
bool esp = false;
bool item_glow = false;
bool player_glow = false;
extern bool aim_no_recoil;
bool aiming = false;
int aim_key = MOUSE_RIGHT;
extern float smooth;
extern int bone;
bool thirdperson = false;
bool chargerifle = false;
bool shooting = false;
uint64_t LocalPlayerPtr = 0;

bool actions_t = false;
bool players_t = false;
bool allplayers_t = false;
bool esp_t = false;
bool aim_t = false;
bool vars_t = false;
bool item_t = false;
uint64_t g_Base;
uint64_t c_Base;
bool next = false;
bool valid = false;
bool lock = false;

extern int mode1, mode2, mode3, mode4;
float handCol[3] = {0.0f, 0.0f, 0.0f};
float oldDelay = 0;

extern float WHITE[3];

typedef struct player
{
	float dist = 0;
	int entity_team = 0;
	float boxMiddle = 0;
	float h_y = 0;
	float width = 0;
	float height = 0;
	float b_x = 0;
	float b_y = 0;
	bool knocked = false;
	bool visible = false;
	int health = 0;
	int shield = 0;
	char name[33] = {0};
} player;

typedef struct playerData
{
	float dist = 0;
	int team = 0;
	float x = 0;
	float y = 0;
	bool knocked = false;
	float dis = 0;
	float rx = 0;
	float ry = 0;
	int health = 0;
	int shield = 0;
	int shieldType = 0;
	char name[64] = {0};
	bool isfriend = false;
} playerData;

struct Enemy
{
	double x;
	double y;
	int health;
	std::string armor;
	std::string name;
};

struct Matrix
{
	float matrix[16];
};

float lastvis_esp[toRead];
float lastvis_aim[toRead];

int tmp_spec = 0, spectators = 0;
int tmp_all_spec = 0, allied_spectators = 0;

bool k_f2 = 0;
bool k_f3 = 0;

//////////////////////////////////////////////////////////////////////////////////////////////////

int showIP()
{
	struct ifaddrs *ifaddr, *ifa;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1)
	{
		perror("getifaddrs");
		return 1;
	}

	// 遍历网络接口
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL)
		{
			continue;
		}

		int family = ifa->ifa_addr->sa_family;
		if (family == AF_INET || family == AF_INET6)
		{
			// 排除本地地址
			if (family == AF_INET && strcmp(ifa->ifa_name, "lo") == 0)
			{
				continue;
			}
			if (family == AF_INET6)
			{
				struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ifa->ifa_addr;
				if (IN6_IS_ADDR_LOOPBACK(&ipv6->sin6_addr))
				{
					continue;
				}
				if (IN6_IS_ADDR_LINKLOCAL(&ipv6->sin6_addr))
				{
					continue;
				}
			}

			// 获取 IP 地址
			void *addr;
			if (family == AF_INET)
			{
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)ifa->ifa_addr;
				addr = &(ipv4->sin_addr);
			}
			else
			{
				struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ifa->ifa_addr;
				addr = &(ipv6->sin6_addr);
			}

			// 转换为字符串形式
			if (inet_ntop(family, addr, host, NI_MAXHOST) == NULL)
			{
				perror("inet_ntop");
				return 1;
			}

			// 打印 IP 地址
			printf("接口: %s\tIP地址: %s\n", ifa->ifa_name, host);
		}
	}

	freeifaddrs(ifaddr);
	return 0;
}

void handle_options(const httplib::Request &req, httplib::Response &res)
{
	res.set_header("Access-Control-Allow-Origin", "*");
	res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
	res.set_header("Access-Control-Allow-Headers", "Content-Type");
	res.status = 200; // 设置状态码为 200，表示请求成功
}
void save_variables_to_file()
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	rapidjson::Document json_root;
	json_root.SetObject();

	json_root.AddMember("item_glow_dist", item_glow_dist, json_root.GetAllocator());
	json_root.AddMember("max_dist", max_dist, json_root.GetAllocator());
	json_root.AddMember("team_player", team_player, json_root.GetAllocator());
	json_root.AddMember("max_fov", max_fov, json_root.GetAllocator());
	json_root.AddMember("smooth", smooth, json_root.GetAllocator());
	json_root.AddMember("aim", aim, json_root.GetAllocator());
	json_root.AddMember("aimkey", aim_key, json_root.GetAllocator());
	json_root.AddMember("esp", esp, json_root.GetAllocator());
	json_root.AddMember("item_glow", item_glow, json_root.GetAllocator());
	json_root.AddMember("player_glow", player_glow, json_root.GetAllocator());
	json_root.AddMember("show_shield", show_shield, json_root.GetAllocator());
	json_root.AddMember("ViewWarn", ViewWarn, json_root.GetAllocator());
	json_root.AddMember("mode1", mode1, json_root.GetAllocator());
	json_root.AddMember("mode2", mode2, json_root.GetAllocator());
	json_root.AddMember("mode3", mode3, json_root.GetAllocator());
	json_root.AddMember("mode4", mode4, json_root.GetAllocator());

	json_root.Accept(writer);
	const char *json_str = buffer.GetString();

	std::ofstream file("settings.json");
	if (file.is_open())
	{
		file << json_str;
		file.close();
	}
	else
	{
		std::cout << "文件打开失败" << std::endl; // Handle file open error if necessary
	}
}
bool load_variables_from_file()
{
	std::ifstream file("settings.json");
	if (!file.is_open())
	{
		std::cout << "文件打开失败" << std::endl; // Handle file open error if necessary
		return false;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	std::string json_str = buffer.str();

	rapidjson::Document json_root;
	if (json_root.Parse(json_str.c_str()).HasParseError())
	{
		std::cout << "Json解析失败" << std::endl; // Handle JSON parsing error if necessary
		return false;
	}

	if (json_root.HasMember("item_glow_dist") && json_root["item_glow_dist"].IsNumber())
		item_glow_dist = json_root["item_glow_dist"].GetFloat();

	if (json_root.HasMember("max_dist") && json_root["max_dist"].IsNumber())
		max_dist = json_root["max_dist"].GetFloat();

	if (json_root.HasMember("team_player") && json_root["team_player"].IsInt())
		team_player = json_root["team_player"].GetInt();

	if (json_root.HasMember("max_fov") && json_root["max_fov"].IsNumber())
		max_fov = json_root["max_fov"].GetFloat();

	if (json_root.HasMember("smooth") && json_root["smooth"].IsNumber())
		smooth = json_root["smooth"].GetFloat();

	if (json_root.HasMember("aim") && json_root["aim"].IsInt())
		aim = json_root["aim"].GetInt();

	if (json_root.HasMember("aimkey") && json_root["aimkey"].IsInt())
		aim_key = json_root["aimkey"].GetInt();

	if (json_root.HasMember("esp") && json_root["esp"].IsInt())
		esp = json_root["esp"].GetInt();

	if (json_root.HasMember("item_glow") && json_root["item_glow"].IsInt())
		item_glow = json_root["item_glow"].GetInt();

	if (json_root.HasMember("player_glow") && json_root["player_glow"].IsInt())
		player_glow = json_root["player_glow"].GetInt();

	if (json_root.HasMember("show_shield") && json_root["show_shield"].IsInt())
		show_shield = json_root["show_shield"].GetInt();

	if (json_root.HasMember("ViewWarn") && json_root["ViewWarn"].IsInt())
		ViewWarn = json_root["ViewWarn"].GetInt();

	if (json_root.HasMember("mode1") && json_root["mode1"].IsInt())
		mode1 = json_root["mode1"].GetInt();

	if (json_root.HasMember("mode2") && json_root["mode2"].IsInt())
		mode2 = json_root["mode2"].GetInt();

	if (json_root.HasMember("mode3") && json_root["mode3"].IsInt())
		mode3 = json_root["mode3"].GetInt();

	if (json_root.HasMember("mode4") && json_root["mode4"].IsInt())
		mode4 = json_root["mode4"].GetInt();

	return true;
}
void handle_request(const httplib::Request &req, httplib::Response &res)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	if (req.path == "/api/control")
	{
		if (req.method == "GET")
		{
			// 处理GET请求的逻辑...
			// 构建控制变量的JSON对象
			rapidjson::Document json_root;
			json_root.SetObject();

			json_root.AddMember("item_glow_dist", item_glow_dist, json_root.GetAllocator());
			json_root.AddMember("max_dist", max_dist, json_root.GetAllocator());
			json_root.AddMember("team_player", team_player, json_root.GetAllocator());
			json_root.AddMember("max_fov", max_fov, json_root.GetAllocator());
			json_root.AddMember("smooth", smooth, json_root.GetAllocator());
			json_root.AddMember("aim", aim, json_root.GetAllocator());
			json_root.AddMember("aimkey", aim_key, json_root.GetAllocator());
			json_root.AddMember("esp", esp, json_root.GetAllocator());
			json_root.AddMember("item_glow", item_glow, json_root.GetAllocator());
			json_root.AddMember("player_glow", player_glow, json_root.GetAllocator());
			json_root.AddMember("show_shield", show_shield, json_root.GetAllocator());
			json_root.AddMember("ViewWarn", ViewWarn, json_root.GetAllocator());
			json_root.AddMember("mode1", mode1, json_root.GetAllocator());
			json_root.AddMember("mode2", mode2, json_root.GetAllocator());
			json_root.AddMember("mode3", mode3, json_root.GetAllocator());
			json_root.AddMember("mode4", mode4, json_root.GetAllocator());

			// 将JSON对象转换为字符串
			json_root.Accept(writer);
			const char *json_str = buffer.GetString();

			res.set_content(json_str, "application/json");
		}
		else if (req.method == "POST")
		{
			// 处理POST请求的逻辑...
			// 解析POST请求的JSON数据
			rapidjson::Document json_root;
			if (!json_root.Parse(req.body.c_str()).HasParseError())
			{
				// 更新控制变量的值
				if (json_root.HasMember("item_glow_dist") && json_root["item_glow_dist"].IsNumber())
					item_glow_dist = json_root["item_glow_dist"].GetFloat();

				if (json_root.HasMember("max_dist") && json_root["max_dist"].IsNumber())
					max_dist = json_root["max_dist"].GetFloat();

				if (json_root.HasMember("team_player") && json_root["team_player"].IsInt())
					team_player = json_root["team_player"].GetInt();

				if (json_root.HasMember("max_fov") && json_root["max_fov"].IsNumber())
					max_fov = json_root["max_fov"].GetFloat();

				if (json_root.HasMember("smooth") && json_root["smooth"].IsNumber())
					smooth = json_root["smooth"].GetFloat();

				if (json_root.HasMember("aim") && json_root["aim"].IsInt())
					aim = json_root["aim"].GetInt();

				if (json_root.HasMember("aimkey") && json_root["aimkey"].IsInt())
					aim_key = json_root["aimkey"].GetInt();

				if (json_root.HasMember("esp") && json_root["esp"].IsInt())
					esp = json_root["esp"].GetInt();

				if (json_root.HasMember("item_glow") && json_root["item_glow"].IsInt())
					item_glow = json_root["item_glow"].GetInt();

				if (json_root.HasMember("player_glow") && json_root["player_glow"].IsInt())
					player_glow = json_root["player_glow"].GetInt();

				if (json_root.HasMember("show_shield") && json_root["show_shield"].IsInt())
					show_shield = json_root["show_shield"].GetInt();

				if (json_root.HasMember("ViewWarn") && json_root["ViewWarn"].IsInt())
					ViewWarn = json_root["ViewWarn"].GetInt();

				if (json_root.HasMember("mode1") && json_root["mode1"].IsInt())
					mode1 = json_root["mode1"].GetInt();
				if (json_root.HasMember("mode2") && json_root["mode2"].IsInt())
					mode2 = json_root["mode2"].GetInt();
				if (json_root.HasMember("mode3") && json_root["mode3"].IsInt())
					mode3 = json_root["mode3"].GetInt();
				if (json_root.HasMember("mode4") && json_root["mode4"].IsInt())
					mode4 = json_root["mode4"].GetInt();

				res.set_content("ok", "text/plain");
			}
			else
			{
				res.set_content("json ERR", "text/plain");
			}
		}
	}
	else if (req.path == "/api/save")
	{
		save_variables_to_file();
	}
	else if (req.path == "/api/load")
	{
		load_variables_from_file();
	}
	else
	{
		// 未知的API路径
		res.set_content("Unknown API", "text/plain");
		res.status = 404;
	}

	// 设置 CORS 头部
	res.set_header("Access-Control-Allow-Origin", "*"); // 允许所有域
	res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
	res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

std::string createResponse(const std::vector<Enemy> &enemies)
{
	rapidjson::Document document;
	document.SetArray();

	for (const auto &enemy : enemies)
	{
		rapidjson::Value obj(rapidjson::kObjectType);
		obj.AddMember("x", enemy.x, document.GetAllocator());
		obj.AddMember("y", enemy.y, document.GetAllocator());
		obj.AddMember("health", enemy.health, document.GetAllocator());
		obj.AddMember("armor", rapidjson::Value(enemy.armor.c_str(), document.GetAllocator()).Move(), document.GetAllocator());
		document.PushBack(obj, document.GetAllocator());
	}

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	document.Accept(writer);

	return buffer.GetString();
}
std::string createResponse(const char *map, const std::vector<Enemy> &enemies)
{

	rapidjson::Document document;
	document.SetObject();

	rapidjson::Value mapValue(map, document.GetAllocator());
	document.AddMember("map", mapValue, document.GetAllocator());
	document.AddMember("mapSizex", mapSizeX, document.GetAllocator());
	document.AddMember("mapSizey", mapSizeY, document.GetAllocator());
	// std::cout << enemies.size() << std::endl;
	rapidjson::Value enemiesArray(rapidjson::kArrayType);
	for (const auto &enemy : enemies)
	{
		// std::cout << "Enemy: x=" << enemy.x << ", y=" << enemy.y << ", armor=" << enemy.armor << std::endl;
		rapidjson::Value enemyObj(rapidjson::kObjectType);
		enemyObj.AddMember("x", enemy.x, document.GetAllocator());
		enemyObj.AddMember("y", enemy.y, document.GetAllocator());
		enemyObj.AddMember("armor", rapidjson::Value(enemy.armor.c_str(), document.GetAllocator()).Move(), document.GetAllocator());
		enemyObj.AddMember("name", rapidjson::Value(enemy.name.c_str(), document.GetAllocator()).Move(), document.GetAllocator());
		enemiesArray.PushBack(enemyObj, document.GetAllocator());
	}
	document.AddMember("enemies", enemiesArray, document.GetAllocator());

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	document.Accept(writer);

	return buffer.GetString();
}
std::vector<playerData> playersData;
std::vector<playerData> AllplayersData;
std::vector<Enemy> generateEnemies(int type = 1)
{
	std::vector<Enemy> enemies;
	std::vector<playerData> playersCopy;
	if (type != 1)
	{
		playersCopy = AllplayersData;
		// std::cout << playersCopy.size() << " " << AllplayersData.size() << " " << playersData.size() << std::endl;
	}
	else
	{
		playersCopy = playersData;
	}

	for (int i = 0; i < playersCopy.size(); i++)
	{
		auto player = playersCopy[i];
		Enemy enemy;
		enemy.x = player.rx;
		enemy.y = player.ry;
		enemy.health = player.health;
		std::string name = "【" + std::to_string(player.team) + "】";
		enemy.name = name + player.name;
		if (player.knocked)
		{
			player.shieldType = 6;
		}
		if (player.isfriend)
		{
			player.shieldType = 7;
		}

		switch (player.shieldType)
		{
		case 0:
			enemy.armor = "white";
			break;
		case 1:
			enemy.armor = "white";
			break;
		case 2:
			enemy.armor = "blue";
			break;
		case 3:
			enemy.armor = "purple";
			break;
		case 4:
			enemy.armor = "gold";
			break;
		case 5:
			enemy.armor = "red";
			break;
		case 6:
			enemy.armor = "down";
			break;
		case 7:
			enemy.armor = "team";
			break;
		default:
			enemy.armor = "white";
			break;
		}
		if (player.shield == 0)
		{
			enemy.armor = "white";
		}

		enemies.push_back(enemy);
	}

	return enemies;
}
int wsServer_Func()
{
	using websocketpp::lib::bind;
	using websocketpp::lib::placeholders::_1;
	using websocketpp::lib::placeholders::_2;

	typedef websocketpp::server<websocketpp::config::asio> server;
	server websocketServer;

	// 设置端口重用
	websocketServer.set_reuse_addr(true);
	websocketServer.clear_access_channels(websocketpp::log::alevel::frame_header);
	websocketServer.clear_access_channels(websocketpp::log::alevel::control);

	websocketServer.set_message_handler([&](websocketpp::connection_hdl hdl, server::message_ptr msg)
										{
        std::string request = msg->get_payload();
		if (!strcmp(request.c_str(), "get_map"))
        {
			//std::cout<<"map"<<std::endl;
			std::vector<Enemy> enemies = generateEnemies(2);						 // 生成新的敌人位置数据
			std::string response = createResponse(mapname, enemies);			 // 创建新的响应
			websocketServer.send(hdl, response, msg->get_opcode()); // 发送更新后的数据给客户端
		}
        else// if (request == "get_positions")
        {
            std::vector<Enemy> enemies = generateEnemies(); // 生成新的敌人位置数据
            std::string response = createResponse(enemies); // 创建新的响应

            // 发送更新后的数据给客户端
            websocketServer.send(hdl, response, websocketpp::frame::opcode::TEXT);
        } });

	try
	{
		websocketServer.init_asio();
		websocketServer.listen(8080);
		websocketServer.start_accept();
		websocketServer.run();
	}
	catch (const std::exception &e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
	catch (websocketpp::lib::error_code e)
	{
		std::cout << "Error code: " << e << ", " << e.message() << std::endl;
	}
	catch (...)
	{
		std::cout << "Unknown exception" << std::endl;
	}

	return 0;
}

int Server_Func()
{
	showIP();
	httplib::Server server;
	server.set_mount_point("/", "./rec");

	server.Get("/api/control", [](const httplib::Request &req, httplib::Response &res)
			   { handle_request(req, res); });
	server.Get("/api/save", [](const httplib::Request &req, httplib::Response &res)
			   { handle_request(req, res); });
	server.Get("/api/load", [](const httplib::Request &req, httplib::Response &res)
			   { handle_request(req, res); });
	server.Post("/api/save", [](const httplib::Request &req, httplib::Response &res)
				{ handle_request(req, res); });
	server.Post("/api/load", [](const httplib::Request &req, httplib::Response &res)
				{ handle_request(req, res); });
	auto handleRequest = [](const httplib::Request & /*req*/, httplib::Response &res)
	{
		std::vector<Enemy> enemies = generateEnemies(1);
		std::string response = createResponse(enemies);

		res.set_content(response, "application/json");
	};

	server.Get("/position", handleRequest);

	server.Post("/api/control", [](const httplib::Request &req, httplib::Response &res)
				{ handle_request(req, res); });

	server.Options("/api/control", [](const httplib::Request &req, httplib::Response &res)
				   { handle_options(req, res); });

	server.set_error_handler([](const httplib::Request &req, httplib::Response &res)
							 {
        res.set_content("Unknown API", "text/plain");
        res.status = 404; });

	printf("HTTP server running on port 8000\n");
	server.listen("::", 8000);
	// server.listen("0.0.0.0", 8000);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void ProcessPlayer(Entity &LPlayer, Entity &target, uint64_t entitylist, int index)
{
	int entity_team = target.getTeamId();

	if (!target.isAlive())
	{
		float localyaw = LPlayer.GetYaw();
		float targetyaw = target.GetYaw();

		if (localyaw == targetyaw)
		{
			if (LPlayer.getTeamId() == entity_team)
				tmp_all_spec++;
			else
				tmp_spec++;
		}
		return;
	}

	Vector EntityPosition = target.getPosition();
	Vector LocalPlayerPosition = LPlayer.getPosition();
	float dist = LocalPlayerPosition.DistTo(EntityPosition);
	if (dist > max_dist)
		return;
	int mode = CheckGameMode();
	if (mode == CONTROL_MODE)
	{
		entity_team %= 2;
		team_player %= 2;
	}
	if (!firing_range)
		if (entity_team < 0 || entity_team > 50 || entity_team == team_player)
			return;

	if (aim == 2)
	{
		if ((target.lastVisTime() > lastvis_aim[index]))
		{
			float fov = CalculateFov(LPlayer, target);
			if (fov < max)
			{
				max = fov;
				tmp_aimentity = target.ptr;
			}
		}
		else
		{
			if (aimentity == target.ptr)
			{
				aimentity = tmp_aimentity = lastaimentity = 0;
			}
		}
	}
	else
	{
		float fov = CalculateFov(LPlayer, target);
		if (fov < max)
		{
			max = fov;
			tmp_aimentity = target.ptr;
		}
	}
	lastvis_aim[index] = target.lastVisTime();
}

// 生成一个颜色数组循环彩虹的颜色
auto Rainbow(float delay)
{
	static uint32_t cnt = 0;
	float freq = delay;

	if (++cnt >= (uint32_t)-1)
	{
		cnt = 0;
	}
	if (delay != oldDelay)
	{
		cnt = 0;
		oldDelay = delay;
	}

	return std::make_tuple(std::sin(freq * cnt + 0) * 2.f, std::sin(freq * cnt + 2) * 2.3f, std::sin(freq * cnt + 4) * 2.6f);
}
void DoActions()
{
	actions_t = true;
	while (actions_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		bool tmp_thirdperson = false;
		bool tmp_chargerifle = false;
		uint32_t counter = 0;

		while (g_Base != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(30));

			uint64_t LocalPlayer = 0;
			apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
			if (LocalPlayer == 0)
				continue;

			Entity LPlayer = getEntity(LocalPlayer);

			team_player = LPlayer.getTeamId();
			if (team_player < 0 || team_player > 50)
			{
				continue;
			}

			if (thirdperson && !tmp_thirdperson)
			{
				if (!aiming)
				{
					apex_mem.Write<int>(g_Base + OFFSET_THIRDPERSON, 1);
					apex_mem.Write<int>(LPlayer.ptr + OFFSET_THIRDPERSON_SV, 1);
					tmp_thirdperson = true;
				}
			}
			else if ((!thirdperson && tmp_thirdperson) || aiming)
			{
				if (tmp_thirdperson)
				{
					apex_mem.Write<int>(g_Base + OFFSET_THIRDPERSON, -1);
					apex_mem.Write<int>(LPlayer.ptr + OFFSET_THIRDPERSON_SV, 0);
					tmp_thirdperson = false;
				}
			}

			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;

			uint64_t baseent = 0;
			apex_mem.Read<uint64_t>(entitylist, baseent);
			if (baseent == 0)
			{
				continue;
			}

			max = 999.0f;
			tmp_aimentity = 0;
			tmp_spec = 0;
			tmp_all_spec = 0;
			if (firing_range)
			{
				int c = 0;
				for (int i = 0; i < 10000; i++)
				{
					uint64_t centity = 0;
					apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
					if (centity == 0)
						continue;
					if (LocalPlayer == centity)
						continue;

					Entity Target = getEntity(centity);
					if (!Target.isDummy())
					{
						continue;
					}

					if (player_glow && !Target.isGlowing())
					{
						Target.enableGlow();
					}
					else if (!player_glow && Target.isGlowing())
					{
						Target.disableGlow();
					}

					ProcessPlayer(LPlayer, Target, entitylist, c);
					c++;
				}
			}
			else
			{
				float *color = WHITE;
				int mode = CheckGameMode();
				for (int i = 0; i < toRead; i++)
				{
					uint64_t centity = 0;
					apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
					if (centity == 0)
						continue;
					if (LocalPlayer == centity)
						continue;

					Entity Target = getEntity(centity);
					if (!Target.isPlayer())
					{
						continue;
					}

					ProcessPlayer(LPlayer, Target, entitylist, i);

					int entity_team = Target.getTeamId();

					if (mode == CONTROL_MODE)
					{
						entity_team %= 2;
						team_player %= 2;
					}
					if (entity_team == team_player)
					{
						continue;
					}

					if (player_glow)
					{
						if (Target.IsVisible())
						{
							if (Target.isKnocked())
								Target.enableGlow(nullptr, true, true);
							else
								Target.enableGlow(nullptr, true);
						}
						else
						{
							if (Target.isKnocked())
								Target.enableGlow(nullptr, false, true);
							else
							{
								Set_Color(Target.getShieldType(), Target.getShield(), Target.getHealth(), entity_team, color);
								Target.enableGlow(color);
							}
						}
					}
					else if (!player_glow && Target.isGlowing())
					{
						Target.disableGlow();
					}
				}
			}
			handCol[0] = std::get<0>(Rainbow(0.05f));
			handCol[1] = std::get<1>(Rainbow(0.05f));
			handCol[2] = std::get<2>(Rainbow(0.05f));

			uint64_t ViewModelHandle = 0;
			apex_mem.Read<uint64_t>(LocalPlayer + OFFSET_VIEWMODEL + 0xc, ViewModelHandle);

			ViewModelHandle &= 0xffff;

			uint64_t ViewModelPtr = 0;
			apex_mem.Read<uint64_t>(g_Base + OFFSET_ENTITYLIST + ViewModelHandle * 0x20, ViewModelPtr);
			if (tmp_spec > 0)
			{
				GlowHand(ViewModelPtr, handCol);
			}
			else
			{
				GlowHandDisable(ViewModelPtr);
			}

			if (!spectators && !allied_spectators)
			{
				spectators = tmp_spec;
				allied_spectators = tmp_all_spec;
			}
			else
			{
				// refresh spectators count every ~2 seconds
				counter++;
				if (counter == 70)
				{
					spectators = tmp_spec;
					allied_spectators = tmp_all_spec;
					counter = 0;
				}
			}

			if (!lock)
				aimentity = tmp_aimentity;
			else
				aimentity = lastaimentity;

			if (chargerifle)
			{
				charge_rifle_hack(LocalPlayer);
				tmp_chargerifle = true;
			}
			else
			{
				if (tmp_chargerifle)
				{
					apex_mem.Write<float>(g_Base + OFFSET_TIMESCALE + 0x68, 1.f);
					tmp_chargerifle = false;
				}
			}
		}
	}
	actions_t = false;
}

void HotKeys()
{
	while (1)
	{

		if (get_button_state(KEY_F2) && k_f2 == 0)
		{
			k_f2 = 1;
			if (aim == 0)
				aim = 2;
			else
				aim = 0;
		}
		else if (!get_button_state(KEY_F2) && k_f2 == 1)
		{
			k_f2 = 0;
		}
		if (get_button_state(KEY_F3) && k_f3 == 0)
		{
			k_f3 = 1;
			player_glow = !player_glow;
		}
		else if (!get_button_state(KEY_F3) && k_f3 == 1)
		{
			k_f3 = 0;
		}
	}
}
Entity VisTarget;
void GetPlayers()
{
	players_t = true;
	while (players_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		uint32_t counter = 0;

		while (g_Base != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

			uint64_t LocalPlayer = 0;
			apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
			if (LocalPlayer == 0)
				continue;
			LocalPlayerPtr = LocalPlayer;
			Entity LPlayer = getEntity(LocalPlayer);
			Entity Local = LPlayer;

			team_player = LPlayer.getTeamId();
			if (team_player < 0 || team_player > 50)
			{
				continue;
			}

			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;

			uint64_t baseent = 0;
			apex_mem.Read<uint64_t>(entitylist, baseent);
			if (baseent == 0)
			{
				continue;
			}
			max = 999.0f;
			tmp_spec = 0;
			tmp_all_spec = 0;

			std::vector<playerData> TmpPlayerList = {};
			playerData tmpPlayer;

			if (!LPlayer.isAlive())
			{
				bool visok = false;
				float localyaw = LPlayer.GetYaw();
				float targetyaw = VisTarget.GetYaw();
				if (localyaw == targetyaw)
				{
					Local = VisTarget;
					visok = true;
				}
				if (!visok)
				{
					uint64_t centity = 0;
					for (int i = 0; i < toRead; i++)
					{
						apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
						if (centity == 0 || LocalPlayer == centity)
							continue;
						Entity Target = getEntity(centity);

						if (!Target.isPlayer())
						{
							continue;
						}
						float localyaw = LPlayer.GetYaw();
						float targetyaw = Target.GetYaw();
						if (localyaw == targetyaw)
						{
							Local = Target;
							VisTarget = Target;
							break;
						}
					}
				}
			}
			local_team = Local.getTeamId();
			for (int i = 0; i < 64; i++)
			{
				bool farplayer = false;
				uint64_t centity = 0;
				apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
				if (centity == 0)
					continue;
				if (LocalPlayer == centity)
					farplayer = true;
				Entity Target = getEntity(centity);
				if (!farplayer)
				{
					if (!Target.isPlayer())
					{
						continue;
					}
					if (!Target.isAlive())
					{
						continue;
					}
					if (!Local.isAlive())
						farplayer = true;
				}
				tmpPlayer.team = Target.getTeamId();
				tmpPlayer.health = Target.getHealth();
				tmpPlayer.shield = Target.getShield();
				tmpPlayer.shieldType = Target.getShieldType();

				Vector EntityPosition = Target.getPosition();
				Vector LocalPlayerPosition = Local.getPosition();
				float dist = LocalPlayerPosition.DistTo(EntityPosition);
				if (dist > max_radar_dist * 40.0f)
					farplayer = true;

				int mode = CheckGameMode();
				if (mode == CONTROL_MODE)
				{
					tmpPlayer.team %= 2;
					team_player %= 2;
				}
				if (tmpPlayer.team == team_player)
				{
					tmpPlayer.isfriend = true;
				}
				else
				{
					tmpPlayer.isfriend = false;
				}
				if (!tmpPlayer.isfriend)
					tmpPlayer.knocked = Target.isKnocked();
				if (!farplayer)
				{ // 距离换算
					tmpPlayer.y = (LocalPlayerPosition.y - EntityPosition.y) / 39.62;
					tmpPlayer.x = (LocalPlayerPosition.x - EntityPosition.x) / 39.62;

					Vector myView = Local.GetViewAnglesV();
					double Yaw = -(double)(myView.y - 90) * 3.1415926 / 180.f;
					// 得出朝向敌人雷达绘制在屏幕的坐标
					tmpPlayer.rx = -(tmpPlayer.x * (float)cos(Yaw) - tmpPlayer.y * (float)sin(Yaw));
					tmpPlayer.ry = tmpPlayer.x * (float)sin(Yaw) + tmpPlayer.y * (float)cos(Yaw);
					tmpPlayer.ry *= -1;

					Target.get_name(g_Base, i - 1, &tmpPlayer.name[0]);
					TmpPlayerList.push_back(tmpPlayer);
				}
			}
			if (TmpPlayerList.size() != 0)
			{
				playersData = TmpPlayerList;
			}
			TmpPlayerList.clear();
		}
	}
	players_t = false;
}
void GetAllPlayers()
{
	allplayers_t = true;
	while (allplayers_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		uint32_t counter = 0;

		while (g_Base != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

			uint64_t LocalPlayer = 0;
			apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
			if (LocalPlayer == 0)
				continue;

			Entity LPlayer = getEntity(LocalPlayer);
			Entity Local = LPlayer;

			team_player = LPlayer.getTeamId();
			if (team_player < 0 || team_player > 50)
			{
				continue;
			}

			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;

			uint64_t baseent = 0;
			apex_mem.Read<uint64_t>(entitylist, baseent);
			if (baseent == 0)
			{
				continue;
			}
			IsLobby();
			if (!strcmp(mapname, "mp_rr_aqueduct"))
			{
				mapSizeX = -4788;
				mapSizeY = 689;
			}
			else if (!strcmp(mapname, "mp_rr_arena_composite"))
			{
				mapSizeX = -7196;
				mapSizeY = 10181;
			}
			else if (!strcmp(mapname, "mp_rr_arena_habitat"))
			{
				mapSizeX = -8191;
				mapSizeY = 8191;
			}
			else if (!strcmp(mapname, "mp_rr_arena_phase_runner"))
			{
				mapSizeX = 17901;
				mapSizeY = 26514;
			}
			else if (!strcmp(mapname, "mp_rr_canyonlands_hu"))
			{
				mapSizeX = -37541;
				mapSizeY = 43886;
			}
			else if (!strcmp(mapname, "mp_rr_canyonlands_mu3"))
			{
				mapSizeX = -37541;
				mapSizeY = 43886;
			}
			else if (!strcmp(mapname, "mp_rr_desertlands_hu"))
			{
				mapSizeX = -45056;
				mapSizeY = 45055;
			}
			else if (!strcmp(mapname, "mp_rr_desertlands_mu4"))
			{
				mapSizeX = -45056;
				mapSizeY = 45055;
			}
			else if (!strcmp(mapname, "mp_rr_divided_moon"))
			{
				mapSizeX = -45000;
				mapSizeY = 43500;
			}
			else if (!strcmp(mapname, "mp_rr_freedm_skulltown"))
			{
				mapSizeX = -12126;
				mapSizeY = 9946;
			}
			else if (!strcmp(mapname, "mp_rr_olympus_mu2"))
			{
				mapSizeX = -52024;
				mapSizeY = 48025;
			}
			else if (!strcmp(mapname, "mp_rr_party_crasher"))
			{
				mapSizeX = -4989;
				mapSizeY = 5646;
			}
			else if (!strcmp(mapname, "mp_rr_tropic_island_mu1"))
			{
				mapSizeX = -50606;
				mapSizeY = 52139;
			}

			mapSize = mapSizeY - mapSizeX;

			tmp_spec = 0;
			tmp_all_spec = 0;

			std::vector<playerData> TmpPlayerList = {};
			std::vector<playerData> TmpAllPlayerList = {};
			playerData tmpPlayer;
			playerData tmpallPlayer;
			if (!LPlayer.isAlive())
			{
				bool visok = false;
				float localyaw = LPlayer.GetYaw();
				float targetyaw = VisTarget.GetYaw();
				if (localyaw == targetyaw)
				{
					Local = VisTarget;
					visok = true;
				}
				if (!visok)
				{
					uint64_t centity = 0;
					for (int i = 0; i < toRead; i++)
					{
						apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
						if (centity == 0 || LocalPlayer == centity)
							continue;
						Entity Target = getEntity(centity);

						if (!Target.isPlayer())
						{
							continue;
						}
						float localyaw = LPlayer.GetYaw();
						float targetyaw = Target.GetYaw();
						if (localyaw == targetyaw)
						{
							Local = Target;
							VisTarget = Target;
							break;
						}
					}
				}
			}
			local_team = Local.getTeamId();
			for (int i = 0; i < 64; i++)
			{
				bool isme = false;
				uint64_t centity = 0;
				apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
				if (centity == 0)
					continue;
				if (Local.ptr == centity)
					isme = true;
				Entity Target = getEntity(centity);
				if (!Target.isPlayer())
				{
					continue;
				}
				if (!Target.isAlive())
				{
					continue;
				}
				tmpallPlayer.team = Target.getTeamId();
				tmpallPlayer.health = Target.getHealth();
				tmpallPlayer.shield = Target.getShield();
				tmpallPlayer.shieldType = Target.getShieldType();

				Vector EntityPosition = Target.getPosition();
				Vector LocalPlayerPosition = Local.getPosition();
				float dist = LocalPlayerPosition.DistTo(EntityPosition);

				int mode = CheckGameMode();
				if (mode == CONTROL_MODE)
				{
					tmpallPlayer.team %= 2;
					team_player %= 2;
				}
				if (tmpallPlayer.team == team_player)
				{
					tmpallPlayer.isfriend = true;
				}
				else
				{
					tmpallPlayer.isfriend = false;
				}
				if (!tmpallPlayer.isfriend)
					tmpallPlayer.knocked = Target.isKnocked();

				// tmpallPlayer.rx = (EntityPosition.x + mapSizeX / 2) / mapSizeX;
				// tmpallPlayer.ry = abs(EntityPosition.y - mapSizeY / 2) / mapSizeY;
				if (!isme)
					tmpallPlayer.dis = dist / 40.0f;
				else
					tmpallPlayer.dis = 99999.0f;

				tmpallPlayer.rx = (EntityPosition.x - mapSizeX) / mapSize;
				tmpallPlayer.ry = abs(EntityPosition.y - mapSizeY) / mapSize;
				Target.get_name(g_Base, i - 1, &tmpallPlayer.name[0]);
				TmpAllPlayerList.push_back(tmpallPlayer);
			}
			if (TmpAllPlayerList.size() != 0)
			{
				AllplayersData = TmpAllPlayerList;
			}
			TmpAllPlayerList.clear();
		}
	}
	allplayers_t = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

player players[toRead];

static void EspLoop()
{
	esp_t = true;
	while (esp_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while (g_Base != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			if (esp)
			{
				valid = false;

				uint64_t LocalPlayer = 0;
				apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
				if (LocalPlayer == 0)
				{
					next = true;
					while (next && g_Base != 0 && esp)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
					continue;
				}
				Entity LPlayer = getEntity(LocalPlayer);
				int team_player = LPlayer.getTeamId();
				if (team_player < 0 || team_player > 50)
				{
					next = true;
					while (next && g_Base != 0 && esp)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
					continue;
				}
				Vector LocalPlayerPosition = LPlayer.getPosition();

				uint64_t viewRenderer = 0;
				apex_mem.Read<uint64_t>(g_Base + OFFSET_RENDER, viewRenderer);
				uint64_t viewMatrix = 0;
				apex_mem.Read<uint64_t>(viewRenderer + OFFSET_MATRIX, viewMatrix);
				Matrix m = {};
				apex_mem.Read<Matrix>(viewMatrix, m);

				uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;

				memset(players, 0, sizeof(players));
				if (firing_range)
				{
					int c = 0;
					for (int i = 0; i < 10000; i++)
					{
						uint64_t centity = 0;
						apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
						if (centity == 0)
						{
							continue;
						}

						if (LocalPlayer == centity)
						{
							continue;
						}

						Entity Target = getEntity(centity);

						if (!Target.isDummy())
						{
							continue;
						}

						if (!Target.isAlive())
						{
							continue;
						}
						int entity_team = Target.getTeamId();

						Vector EntityPosition = Target.getPosition();
						float dist = LocalPlayerPosition.DistTo(EntityPosition);
						if (dist > max_dist || dist < 50.0f)
						{
							continue;
						}

						Vector bs = Vector();
						WorldToScreen(EntityPosition, m.matrix, 1920, 1080, bs);
						if (bs.x > 0 && bs.y > 0)
						{
							Vector hs = Vector();
							Vector HeadPosition = Target.getBonePositionByHitbox(0);
							WorldToScreen(HeadPosition, m.matrix, 1920, 1080, hs);
							float height = abs(abs(hs.y) - abs(bs.y));
							float width = height / 2.0f;
							float boxMiddle = bs.x - (width / 2.0f);
							int health = Target.getHealth();
							int shield = Target.getShield();
							players[c] =
								{
									dist,
									entity_team,
									boxMiddle,
									hs.y,
									width,
									height,
									bs.x,
									bs.y,
									0,
									(Target.lastVisTime() > lastvis_esp[c]),
									health,
									shield};
							Target.get_name(g_Base, i - 1, &players[c].name[0]);
							lastvis_esp[c] = Target.lastVisTime();
							valid = true;
							c++;
						}
					}
				}
				else
				{
					for (int i = 0; i < toRead; i++)
					{
						uint64_t centity = 0;
						apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
						if (centity == 0)
						{
							continue;
						}

						if (LocalPlayer == centity)
						{
							continue;
						}

						Entity Target = getEntity(centity);

						if (!Target.isPlayer())
						{
							continue;
						}

						if (!Target.isAlive())
						{
							continue;
						}

						int entity_team = Target.getTeamId();
						if (entity_team < 0 || entity_team > 50 || entity_team == team_player)
						{
							continue;
						}

						Vector EntityPosition = Target.getPosition();
						float dist = LocalPlayerPosition.DistTo(EntityPosition);
						if (dist > max_dist || dist < 50.0f)
						{
							continue;
						}

						Vector bs = Vector();
						WorldToScreen(EntityPosition, m.matrix, 1920, 1080, bs);
						if (bs.x > 0 && bs.y > 0)
						{
							Vector hs = Vector();
							Vector HeadPosition = Target.getBonePositionByHitbox(0);
							WorldToScreen(HeadPosition, m.matrix, 1920, 1080, hs);
							float height = abs(abs(hs.y) - abs(bs.y));
							float width = height / 2.0f;
							float boxMiddle = bs.x - (width / 2.0f);
							int health = Target.getHealth();
							int shield = Target.getShield();

							players[i] =
								{
									dist,
									entity_team,
									boxMiddle,
									hs.y,
									width,
									height,
									bs.x,
									bs.y,
									Target.isKnocked(),
									(Target.lastVisTime() > lastvis_esp[i]),
									health,
									shield};
							Target.get_name(g_Base, i - 1, &players[i].name[0]);
							lastvis_esp[i] = Target.lastVisTime();
							valid = true;
						}
					}
				}

				next = true;
				while (next && g_Base != 0 && esp)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
		}
	}
	esp_t = false;
}

static void AimbotLoop()
{
	aim_t = true;
	while (aim_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		while (g_Base != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			if (aim > 0)
			{
				aiming = get_button_state(aim_key);
				if (!aiming)
				{
					if (aim_key == MOUSE_RIGHT)
					{
						aiming = get_button_state(MOUSE_LEFT);
					}
					else if (aim_key == KEY_XBUTTON_LTRIGGER)
					{
						aiming = get_button_state(KEY_XBUTTON_RTRIGGER);
					}
				}

				if (aimentity == 0 || !aiming)
				{
					lock = false;
					lastaimentity = 0;
					continue;
				}
				lock = true;
				lastaimentity = aimentity;
				uint64_t LocalPlayer = 0;
				apex_mem.Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
				if (LocalPlayer == 0)
					continue;
				Entity LPlayer = getEntity(LocalPlayer);
				if (LPlayer.isKnocked() || !LPlayer.isAlive())
				{
					continue;
				}

				QAngle Angles = CalculateBestBoneAim(LPlayer, aimentity, max_fov);
				if (Angles.x == 0 && Angles.y == 0)
				{
					lock = false;
					lastaimentity = 0;
					continue;
				}
				LPlayer.SetViewAngles(Angles);
			}
		}
	}
	aim_t = false;
}
void SG_Func()
{
	bool superGlideStart = false;
	int frameSleepTimer, superGlideTimer;
	float lastFrameNumber = 0;
	float curTime;

	while (true)
	{
		if (!LocalPlayerPtr)
		{
			continue;
		}

		float w, m_traversalProgress, m_traversalProgressTmp;
		float curFrameNumber;
		apex_mem.Read<float>(g_Base + OFFSET_GLOBALVAR + 0x08, curFrameNumber);
		if (curFrameNumber > lastFrameNumber)
		{
			frameSleepTimer = 10; // <- middle of the frame
		}
		lastFrameNumber = curFrameNumber;

		if (frameSleepTimer == 0)
		{
			apex_mem.Read<float>(LocalPlayerPtr + OFFSET_HANG_TIME, m_traversalProgress);
			if (m_traversalProgress > 0.35 && m_traversalProgress < 1.0)
			{
				superGlideStart = true;
			}

			if (superGlideStart)
			{
				superGlideTimer++;

				if (superGlideTimer == 5)
				{
					apex_mem.Write<int>(g_Base + OFFSET_IN_JUMP + 0x08, 5);
				}
				else if (superGlideTimer == 6)
				{
					apex_mem.Write<int>(g_Base + OFFSET_IN_DUCK + 0x08, 6);
				}
				else if (superGlideTimer == 14)
				{
					apex_mem.Write<int>(g_Base + OFFSET_IN_JUMP + 0x08, 4);
					m_traversalProgressTmp = m_traversalProgress;
				}
				else if (superGlideTimer > 10 && m_traversalProgress != m_traversalProgressTmp)
				{
					superGlideStart = false;
					superGlideTimer = 0;
				}
			}
		}
		frameSleepTimer -= 1;
	}
}

static void set_vars(uint64_t add_addr)
{
	printf("Reading client vars...\n");
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	// Get addresses of client vars
	uint64_t check_addr = 0;
	client_mem.Read<uint64_t>(add_addr, check_addr);
	uint64_t aim_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t), aim_addr);
	uint64_t esp_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 2, esp_addr);
	uint64_t aiming_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 3, aiming_addr);
	uint64_t g_Base_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 4, g_Base_addr);
	uint64_t next_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 5, next_addr);
	uint64_t player_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 6, player_addr);
	uint64_t valid_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 7, valid_addr);
	uint64_t max_dist_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 8, max_dist_addr);
	uint64_t item_glow_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 9, item_glow_addr);
	uint64_t player_glow_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 10, player_glow_addr);
	uint64_t aim_no_recoil_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 11, aim_no_recoil_addr);
	uint64_t smooth_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 12, smooth_addr);
	uint64_t max_fov_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 13, max_fov_addr);
	uint64_t bone_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 14, bone_addr);
	uint64_t thirdperson_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 15, thirdperson_addr);
	uint64_t spectators_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 16, spectators_addr);
	uint64_t allied_spectators_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 17, allied_spectators_addr);
	uint64_t chargerifle_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 18, chargerifle_addr);
	uint64_t shooting_addr = 0;
	client_mem.Read<uint64_t>(add_addr + sizeof(uint64_t) * 19, shooting_addr);

	uint32_t check = 0;
	client_mem.Read<uint32_t>(check_addr, check);

	if (check != 0xABCD)
	{
		printf("Incorrect values read. Check if the add_off is correct. Quitting.\n");
		active = false;
		return;
	}
	vars_t = true;
	while (vars_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (c_Base != 0 && g_Base != 0)
		{
			client_mem.Write<uint32_t>(check_addr, 0);
			printf("\nReady\n");
		}

		while (c_Base != 0 && g_Base != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			client_mem.Write<uint64_t>(g_Base_addr, g_Base);
			client_mem.Write<int>(spectators_addr, spectators);
			client_mem.Write<int>(allied_spectators_addr, allied_spectators);

			client_mem.Read<int>(aim_addr, aim);
			client_mem.Read<bool>(esp_addr, esp);
			client_mem.Read<bool>(aiming_addr, aiming);
			client_mem.Read<float>(max_dist_addr, max_dist);
			client_mem.Read<bool>(item_glow_addr, item_glow);
			client_mem.Read<bool>(player_glow_addr, player_glow);
			client_mem.Read<bool>(aim_no_recoil_addr, aim_no_recoil);
			client_mem.Read<float>(smooth_addr, smooth);
			client_mem.Read<float>(max_fov_addr, max_fov);
			client_mem.Read<int>(bone_addr, bone);
			client_mem.Read<bool>(thirdperson_addr, thirdperson);
			client_mem.Read<bool>(shooting_addr, shooting);
			client_mem.Read<bool>(chargerifle_addr, chargerifle);

			if (esp && next)
			{
				if (valid)
					client_mem.WriteArray<player>(player_addr, players, toRead);
				client_mem.Write<bool>(valid_addr, valid);
				client_mem.Write<bool>(next_addr, true); // next

				bool next_val = false;
				do
				{
					client_mem.Read<bool>(next_addr, next_val);
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				} while (next_val && g_Base != 0);

				next = false;
			}
		}
	}
	vars_t = false;
}

static void item_glow_t()
{
	item_t = true;
	while (item_t)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		int k = 0;
		while (g_Base != 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
			if (item_glow)
			{
				for (int i = 0; i < 10000; i++)
				{
					uint64_t centity = 0;
					apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
					if (centity == 0)
						continue;
					Item item = getItem(centity);

					if (item.isItem() && !item.isGlowing())
					{
						item.enableGlow();
					}
				}
				k = 1;
				std::this_thread::sleep_for(std::chrono::milliseconds(600));
			}
			else
			{
				if (k == 1)
				{
					for (int i = 0; i < 10000; i++)
					{
						uint64_t centity = 0;
						apex_mem.Read<uint64_t>(entitylist + ((uint64_t)i << 5), centity);
						if (centity == 0)
							continue;

						Item item = getItem(centity);

						if (item.isItem() && item.isGlowing())
						{
							item.disableGlow();
						}
					}
					k = 0;
				}
			}
		}
	}
	item_t = false;
}

int main(int argc, char *argv[])
{
	if (geteuid() != 0)
	{
		printf("Error: %s is not running as root\n", argv[0]);
		return 0;
	}

	const char *cl_proc = "client_ap.exe";
	const char *ap_proc = "R5Apex.exe";
	// const char* ap_proc = "EasyAntiCheat_launcher.exe";

	// Client "add" offset
	uint64_t add_off = 0x3f880;

	std::thread aimbot_thr;
	std::thread esp_thr;
	std::thread hotkeys_thr;
	std::thread actions_thr;
	// std::thread SG_thr;
	std::thread players_thr;
	std::thread allplayers_thr;
	std::thread server_thr;
	std::thread ws_server_thr;
	std::thread itemglow_thr;
	std::thread vars_thr;
	while (active)
	{
		if (apex_mem.get_proc_status() != process_status::FOUND_READY)
		{
			if (aim_t)
			{
				aim_t = false;
				esp_t = false;
				actions_t = false;
				players_t = false;
				allplayers_t = false;
				item_t = false;
				g_Base = 0;

				aimbot_thr.~thread();
				esp_thr.~thread();
				actions_thr.~thread();
				players_thr.~thread();
				allplayers_thr.~thread();
				itemglow_thr.~thread();
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("Searching for apex process...\n");

			apex_mem.open_proc(ap_proc);

			if (apex_mem.get_proc_status() == process_status::FOUND_READY)
			{
				g_Base = apex_mem.get_proc_baseaddr();
				c_Base = 1;
				printf("\nApex process found\n");
				printf("Base: %lx\n", g_Base);

				aimbot_thr = std::thread(AimbotLoop);
				esp_thr = std::thread(EspLoop);
				server_thr = std::thread(Server_Func);
				ws_server_thr = std::thread(wsServer_Func);
				actions_thr = std::thread(DoActions);
				// SG_thr = std::thread(SG_Func);
				hotkeys_thr = std::thread(HotKeys);
				players_thr = std::thread(GetPlayers);
				allplayers_thr = std::thread(GetAllPlayers);
				itemglow_thr = std::thread(item_glow_t);
				aimbot_thr.detach();
				esp_thr.detach();
				server_thr.detach();
				ws_server_thr.detach();
				actions_thr.detach();
				// SG_thr.detach();
				hotkeys_thr.detach();
				players_thr.detach();
				allplayers_thr.detach();
				itemglow_thr.detach();
			}
		}
		else
		{
			apex_mem.check_proc();
		}

		/*if (client_mem.get_proc_status() != process_status::FOUND_READY)
		{
			if (vars_t)
			{
				vars_t = false;
				c_Base = 0;

				vars_thr.~thread();
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
			printf("Searching for client process...\n");

			client_mem.open_proc(cl_proc);

			if (client_mem.get_proc_status() == process_status::FOUND_READY)
			{
				c_Base = client_mem.get_proc_baseaddr();
				printf("\nClient process found\n");
				printf("Base: %lx\n", c_Base);

				vars_thr = std::thread(set_vars, c_Base + add_off);
				vars_thr.detach();
			}
		}
		else
		{
			client_mem.check_proc();
		}*/

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}