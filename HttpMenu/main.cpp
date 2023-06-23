#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <httplib.h>
#include <jansson.h>

// 下面的是显示ip地址用的
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// 控制变量
float max_value = 999.0f;
float max_dist = 200.0f * 40.0f;
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

// 处理GET和POST请求的回调函数
void handle_request(const httplib::Request &req, httplib::Response &res)
{
    const char *response = NULL;
    int status_code = 200;

    if (req.path == "/api/control")
    {
        if (req.method == "GET")
        {
            // 处理GET请求的逻辑...
            // 构建控制变量的JSON对象
            json_t *json_root = json_object();
            json_object_set_new(json_root, "max", json_real(max_value));
            json_object_set_new(json_root, "max_dist", json_real(max_dist));
            json_object_set_new(json_root, "team_player", json_integer(team_player));
            json_object_set_new(json_root, "max_fov", json_real(max_fov));
            json_object_set_new(json_root, "aim", json_integer(aim));
            json_object_set_new(json_root, "esp", json_integer(esp));
            json_object_set_new(json_root, "item_glow", json_integer(item_glow));
            json_object_set_new(json_root, "player_glow", json_integer(player_glow));

            // 将JSON对象转换为字符串
            char *json_str = json_dumps(json_root, JSON_COMPACT);
            response = json_str;

            json_decref(json_root);
        }
        else if (req.method == "POST")
        {
            // 处理POST请求的逻辑...
            // 解析POST请求的JSON数据
            json_error_t error;
            json_t *json_root = json_loadb(req.body.c_str(), req.body.size(), JSON_DISABLE_EOF_CHECK, &error);
            if (json_root != nullptr)
            {
                // 更新控制变量的值
                json_t *json_value;

                json_value = json_object_get(json_root, "max");
                if (json_value && json_is_number(json_value))
                    max_value = json_real_value(json_value);

                json_value = json_object_get(json_root, "max_dist");
                if (json_value && json_is_number(json_value))
                    max_dist = json_real_value(json_value);

                json_value = json_object_get(json_root, "team_player");
                if (json_value && json_is_integer(json_value))
                    team_player = json_integer_value(json_value);

                json_value = json_object_get(json_root, "max_fov");
                if (json_value && json_is_number(json_value))
                    max_fov = json_real_value(json_value);

                json_value = json_object_get(json_root, "aim");
                if (json_value && json_is_integer(json_value))
                    aim = json_integer_value(json_value);

                json_value = json_object_get(json_root, "esp");
                if (json_value && json_is_integer(json_value))
                    esp = json_integer_value(json_value);

                json_value = json_object_get(json_root, "item_glow");
                if (json_value && json_is_integer(json_value))
                    item_glow = json_integer_value(json_value);

                json_value = json_object_get(json_root, "player_glow");
                if (json_value && json_is_integer(json_value))
                    player_glow = json_integer_value(json_value);

                json_decref(json_root);
                response = "ok";
            }
            else
            {
                response = "json ERR";
            }
        }
    }
    else
    {
        // 未知的API路径
        response = "Unknown API";
        status_code = 404;
    }

    res.set_content(response, "application/json");
    // 设置 CORS 头部
    res.set_header("Access-Control-Allow-Origin", "*"); // 允许所有域
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    res.status = status_code;
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
