```c++
struct Vector2 {
    float x, y;
};
void dwm_draw()
{
    for (int i = 0; i <= 60; i++)
    {
        //读自己坐标
        if (i != 0)
        {
            uint64_t LocalPlayer = 0;
            Read<uint64_t>(g_Base + OFFSET_LOCAL_ENT, LocalPlayer);
            Vector localplayer = { 0,0,0 };
            Read<Vector>(LocalPlayer + OFFSET_ORIGIN, localplayer);
            //读敌人坐标

            uint64_t entity = 0;
            Read<uint64_t>(g_Base + OFFSET_ENTITYLIST + ((uint64_t)i << 5), entity);
            if (entity != LocalPlayer)
            {
                Vector player = { 0,0,0 };
                Read<Vector>(entity + OFFSET_ORIGIN, player);


                //距离换算
                float people_subs_y = (localplayer.y - player.y) / 39.62;
                float people_subs_x = (localplayer.x - player.x) / 39.62;




                //读自己朝向角度
                Vector2 test = { 0,0 };

                Read<Vector2>(LocalPlayer + OFFSET_VIEWANGLES, test);
                //换算
                double Yaw = -(double)(test.y - 90) * 3.1415926 / 180.f;
                //得出朝向敌人雷达绘制在屏幕的坐标
                float rarda_x = -(people_subs_x * (float)cos(Yaw) - people_subs_y * (float)sin(Yaw));
                float rarda_y = people_subs_x * (float)sin(Yaw) + people_subs_y * (float)cos(Yaw);
                drawtext(960 + rarda_x, 540 + rarda_y, 15, 0xff0000ff, (char*)"0");
            }
            if (dma_next && valid)
            {
                int name_list_lenght = 0;
                if (players_o[i].health > 0)
                {
                    /* name_list_lenght++;*/
                     //drawtext(5, 0 + name_list_lenght * 18, 10, 0xffFFFFFF, players_o[i].name);
                    std::string distance = std::to_string(players_o[i].dist / 39.62);
                    distance = distance.substr(0, distance.find('.')) + "m(" + std::to_string(players_o[i].entity_team) + ")";
                    if (v.box)
                    {
                        if (players

```

