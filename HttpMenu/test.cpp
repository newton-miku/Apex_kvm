#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <httplib.h>
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
    std::uniform_real_distribution<double> dis(-200.0, 200.0);
    std::uniform_int_distribution<int> healthDis(50, 100);
    std::uniform_int_distribution<int> armorDis(0, 4);

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
    httplib::Server server;

    auto handleRequest = [](const httplib::Request & /*req*/, httplib::Response &res)
    {
        std::vector<Enemy> enemies = generateEnemies(5);
        std::string response = createResponse(enemies);

        res.set_content(response, "application/json");
    };

    server.Get("/position", handleRequest);

    server.set_base_dir("./");
    server.listen("0.0.0.0", 8080);

    return 0;
}
