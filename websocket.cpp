#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <httplib.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

struct Enemy
{
    double x;
    double y;
    int health;
    std::string armor;
};

std::vector<Enemy> generateEnemies(int count)
{
    std::vector<Enemy> enemies;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(-400.0, 400.0);
    std::uniform_int_distribution<int> healthDis(0, 100);
    std::uniform_int_distribution<int> armorDis(0, 7);

    for (int i = 0; i < count; i++)
    {
        Enemy enemy;
        enemy.x = dis(gen);
        enemy.y = dis(gen);
        enemy.health = healthDis(gen);

        int armorType = armorDis(gen);
        switch (armorType)
        {
        case 0:
            enemy.armor = "white";
            break;
        case 1:
            enemy.armor = "blue";
            break;
        case 2:
            enemy.armor = "purple";
            break;
        case 3:
            enemy.armor = "red";
            break;
        case 4:
            enemy.armor = "gold";
            break;
        case 5:
            enemy.armor = "down";
            break;
        case 6:
            enemy.armor = "team";
            break;
        default:
            enemy.armor = "white";
            break;
        }

        enemies.push_back(enemy);
    }

    return enemies;
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

int main()
{
    using websocketpp::lib::bind;
    using websocketpp::lib::placeholders::_1;
    using websocketpp::lib::placeholders::_2;

    typedef websocketpp::server<websocketpp::config::asio> server;
    server websocketServer;

    httplib::Server libhttpServer;

    // 设置端口重用
    websocketServer.set_reuse_addr(true);
    websocketServer.clear_access_channels(websocketpp::log::alevel::frame_header);
    websocketServer.clear_access_channels(websocketpp::log::alevel::control);

    std::vector<Enemy> initialEnemies = generateEnemies(5);
    std::string initialResponse = createResponse(initialEnemies);

    libhttpServer.Get("/position", [&](const httplib::Request &req, httplib::Response &res)
                      {
                          std::string protocol = req.get_header_value("Upgrade");
                          if (protocol.find("websocket") != std::string::npos)
                          {
                              // WebSocket协议请求，交给WebSocket服务器处理
                              res.set_content("", "application/json");
                              return;
                          }

                          // 其他协议请求，返回初始敌人位置数据
                          res.set_content(initialResponse, "application/json"); });

    websocketServer.set_message_handler([&](websocketpp::connection_hdl hdl, server::message_ptr msg)
                                        {
                                            std::vector<Enemy> updatedEnemies = generateEnemies(5); // 生成新的敌人位置数据
                                            std::string response = createResponse(updatedEnemies);  // 创建新的响应

                                            websocketServer.send(hdl, response, msg->get_opcode()); // 发送更新后的数据给客户端
                                        });

    std::thread httpThread([&]()
                           {
                              libhttpServer.set_base_dir("./");
                              libhttpServer.listen("0.0.0.0", 8080); });

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

    websocketServer.stop_listening();
    websocketServer.stop();

    libhttpServer.stop();
    httpThread.join();

    return 0;
}
