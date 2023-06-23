#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <httplib.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

// 下面的是显示ip地址用的
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// 控制变量
float item_glow_dist = 200.0f;
float max_dist = 1200.0f;
int team_player = 1;
float max_fov = 15.0f;
int aim = 0;
int esp = 0;
int item_glow = 0;
int player_glow = 0;

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

void handle_request(const httplib::Request &req, httplib::Response &res)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    if (req.path == "/api/control")
    {
        if (req.method == "GET")
        {
            // 构建控制变量的 JSON 对象
            rapidjson::Document json_root;
            json_root.SetObject();

            json_root.AddMember("item_glow_dist", item_glow_dist, json_root.GetAllocator());
            json_root.AddMember("max_dist", max_dist, json_root.GetAllocator());
            json_root.AddMember("team_player", team_player, json_root.GetAllocator());
            json_root.AddMember("max_fov", max_fov, json_root.GetAllocator());
            json_root.AddMember("aim", aim, json_root.GetAllocator());
            json_root.AddMember("esp", esp, json_root.GetAllocator());
            json_root.AddMember("item_glow", item_glow, json_root.GetAllocator());
            json_root.AddMember("player_glow", player_glow, json_root.GetAllocator());

            // 将 JSON 对象转换为字符串
            json_root.Accept(writer);
            const char *json_str = buffer.GetString();

            res.set_content(json_str, "application/json");
        }
        else if (req.method == "POST")
        {
            // 解析 POST 请求的 JSON 数据
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

                if (json_root.HasMember("aim") && json_root["aim"].IsInt())
                    aim = json_root["aim"].GetInt();

                if (json_root.HasMember("esp") && json_root["esp"].IsInt())
                    esp = json_root["esp"].GetInt();

                if (json_root.HasMember("item_glow") && json_root["item_glow"].IsInt())
                    item_glow = json_root["item_glow"].GetInt();

                if (json_root.HasMember("player_glow") && json_root["player_glow"].IsInt())
                    player_glow = json_root["player_glow"].GetInt();

                res.set_content("ok", "text/plain");
            }
            else
            {
                res.set_content("json ERR", "text/plain");
            }
        }
    }
    else
    {
        // 未知的 API 路径
        res.set_content("Unknown API", "text/plain");
        res.status = 404;
    }

    // 设置 CORS 头部
    res.set_header("Access-Control-Allow-Origin", "*"); // 允许所有域
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

int main()
{
    showIP();
    httplib::Server server;
    server.set_mount_point("/", "./rec");

    server.Get("/api/control", [](const httplib::Request &req, httplib::Response &res)
               { handle_request(req, res); });

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
