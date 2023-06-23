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

std::string createResponse(const std::string &map, const std::vector<Enemy> &enemies)
{
    rapidjson::Document document;
    document.SetObject();

    rapidjson::Value mapValue(map.c_str(), document.GetAllocator());
    document.AddMember("map", mapValue, document.GetAllocator());
    document.AddMember("mapSizex", 107087, document.GetAllocator());
    document.AddMember("mapSizey", 108413, document.GetAllocator());

    rapidjson::Value enemiesArray(rapidjson::kArrayType);
    for (const auto &enemy : enemies)
    {
        rapidjson::Value enemyObj(rapidjson::kObjectType);
        enemyObj.AddMember("x", enemy.x + 53543.5, document.GetAllocator());
        enemyObj.AddMember("y", enemy.y + 54206.5, document.GetAllocator());
        enemyObj.AddMember("armor", rapidjson::Value(enemy.armor.c_str(), document.GetAllocator()).Move(), document.GetAllocator());
        enemiesArray.PushBack(enemyObj, document.GetAllocator());
    }
    document.AddMember("enemies", enemiesArray, document.GetAllocator());

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}

std::vector<Enemy> generateEnemies(int count)
{
    std::vector<Enemy> enemies;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> disx(-54129.0, 52958.0);
    std::uniform_real_distribution<double> disy(-55265.0, 53148.0);
    std::uniform_int_distribution<int> healthDis(0, 100);
    std::uniform_int_distribution<int> armorDis(0, 7);

    for (int i = 0; i < count; i++)
    {
        Enemy enemy;
        enemy.x = disx(gen);
        enemy.y = disy(gen);
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

int main()
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
        if (msg->get_payload() == "get_map")
        {
            std::vector<Enemy> enemies = generateEnemies(30); // 生成新的敌人位置数据
            std::string response = createResponse("mp_rr_olympus_mu2", enemies); // 创建新的响应
            
            websocketServer.send(hdl, response, msg->get_opcode()); // 发送更新后的数据给客户端
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
