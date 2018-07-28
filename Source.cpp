#include "XLibrary11.hpp"
using namespace XLibrary;

const int width = 100;
const int height = 100;
const float w1 = 1.1f;
const float w2 = -0.28f;
const float r1 = 2.9f;
const float r2 = 6.0f;
bool map[width][height];

void GetAroundCellIndexes(int cx, int cy, int level, int width, int height, std::vector<XMINT2>& indexes)
{
    int d[4][2] = { {-1, -1}, {-1, 1}, {1, 1}, {1, -1} };
    if (level == 0)
    {
        indexes.push_back(XMINT2(cx, cy));
        return;
    }
    indexes.resize(4 * 2 * level);
    int index = 0;
    for (int i = 0; i < 4; i++)
    {
        int x = cx + d[i][0] * level;
        int y = cy + d[i][1] * level;
        int dx, dy;
        if (d[i][0] * d[i][1] > 0)
        {
            dx = 0;
            dy = -1 * d[i][1];
        }
        else
        {
            dx = d[i][1];
            dy = 0;
        }
        for (int j = 0; j < 2 * level; j++, x += dx, y += dy)
        {
            indexes[index] = XMINT2(
                (x + width) % width,
                (y + height) % height
            );
            index++;
        }
    }
}

bool CalculateWeight(int x, int y, float dx, float dy, float workboard[width][height])
{
    if (dx * dx + dy * dy > r2 * r2)
        return false;

    if (dx * dx + dy * dy > r1 * r1)
        workboard[x][y] += w2;
    else
        workboard[x][y] += w1;

    return true;
}

void Generate()
{
    /*! http://www.usamimi.info/~ide/programe/turingmodel/index.html */
    Float2 boardRepresentation[width][height];

    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            map[x][y] = Random::GetValue() < 0.2f;
            boardRepresentation[x][y] = Float2(x + 0.5f, y + 0.5f);
        }
    }

    float workboard[width][height] = {};

    int count = 0;
    while (count < 3)
    {
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                if (!map[x][y])
                    continue;

                Float2 center = boardRepresentation[x][y];
                bool findTarget = true;
                for (int level = 0; findTarget; level++)
                {
                    std::vector<XMINT2> indexes;
                    GetAroundCellIndexes(x, y, level, width, height, indexes);
                    findTarget = false;
                    for (int i = 0; i < indexes.size(); i++)
                    {
                        int tx = indexes[i].x;
                        int ty = indexes[i].y;
                        Float2 rp = boardRepresentation[tx][ty];
                        float dx = fabsf(rp.x - center.x);
                        float dy = fabsf(rp.y - center.y);
                        dx = fminf(dx, width - dx);
                        dy = fminf(dy, height - dy);
                        if (CalculateWeight(tx, ty, dx, dy, workboard))
                            findTarget = true;
                    }
                }
            }
        }
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                if (workboard[x][y] > 0.0f)
                    map[x][y] = true;
                else if (workboard[x][y] < 0.0f)
                    map[x][y] = false;
            }
        }
        count++;
    }
}

enum Mode
{
    Init,
    Game,
};

int MAIN()
{
    Camera camera;
    camera.color = Float4(0.0f, 0.0f, 0.0f, 0.0f);

    Sprite ground(L"ground.png");
    ground.scale = 2.0f;

    Sprite player(L"player.png");
    player.scale = 2.0f;
    player.SetPivot(Float2(0.0f, -1.0f));

    Float3 position;

    Mode mode = Init;

    while (Refresh())
    {
        Window::SetTitle(std::to_wstring(Timer::GetFrameRate()).c_str());

        camera.Update();

        switch (mode)
        {
        case Init:
            Generate();

            XMINT2 temp;
            do
            {
                temp = XMINT2(Random::Range(0, width - 1), Random::Range(0, height - 1));
            } while (!map[temp.x][temp.y]);

            position = Float3(temp.x * 32.0f, temp.y * 32.0f, 0.0f);

            mode = Game;
            break;
        case Game:
            Float3 velocity;

            if (Input::GetKeyDown(VK_RETURN))
                mode = Init;

            if (Input::GetKey(VK_LEFT))
                velocity.x = -2.0f;

            if (Input::GetKey(VK_RIGHT))
                velocity.x = 2.0f;

            if (Input::GetKey(VK_UP))
                velocity.y = 2.0f;

            if (Input::GetKey(VK_DOWN))
                velocity.y = -2.0f;

            XMINT2 center((int)roundf(position.x / 32.0f), (int)roundf(position.y / 32.0f));

            for (int x = center.x - 1; x <= center.x + 1; x++)
            {
                for (int y = center.y - 1; y <= center.y + 1; y++)
                {
                    Float3 now = position;
                    Float3 next = position + velocity;
                    Float2 hit = Float2(x * 32.0f, y * 32.0f);

                    if ((x < 0 || y < 0 || x >= width || y >= height) ||
                        !map[x][y])
                    {
                        if (next.x > hit.x - 32 &&
                            next.x < hit.x + 32 &&
                            now.y > hit.y - 32 &&
                            now.y < hit.y + 32)
                        {
                            velocity.x = roundf(next.x / 32.0f) * 32.0f - now.x;
                        }

                        if (now.x > hit.x - 32 &&
                            now.x < hit.x + 32 &&
                            next.y > hit.y - 32 &&
                            next.y < hit.y + 32)
                        {
                            velocity.y = roundf(next.y / 32.0f) * 32.0f - now.y;
                        }
                    }
                }
            }

            XMINT2 size(Window::GetSize().x / 32 / 2 + 1, Window::GetSize().y / 32 / 2 + 1);
            for (int x = center.x - size.x; x <= center.x + size.x; x++)
            {
                for (int y = center.y - size.y; y <= center.y + size.y; y++)
                {
                    if (x < 0 || y < 0 || x >= width || y >= height)
                        continue;

                    if (!map[x][y])
                        continue;

                    ground.position = Float3(x * 32.0f, y * 32.0f, 0);
                    ground.Draw();
                }
            }

            position += velocity;
            camera.position = position;
            player.position = position;
            player.Draw();
            break;
        }
    }

    return 0;
}
