#include "Application.h"

// Standart Stuff
#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <list>


constexpr int ScreenWidth = 1280;
constexpr int ScreenHeight = 960;


class TSP : public LGE::Scene_t
{
public:

    std::vector<glm::vec2> cities;
    std::vector<glm::vec2> bestAnswer;
    unsigned int bestDist;
    unsigned int totalCities = 5;

    TSP()
    {
        bestAnswer.resize(totalCities);
        for (int i = 0; i < totalCities; ++i)
        {
            cities.push_back({ LGE::rand(0, ScreenWidth), LGE::rand(0, ScreenHeight) });
        }
        bestAnswer = cities;
        bestDist = calcDist();
    }

    ~TSP() {}

    void foundExatlyTSP()
    {

    }

    void Swap(unsigned int a, unsigned int b)
    {
        glm::vec2 c = cities[a];
        cities[a] = cities[b];
        cities[b] = c;
    }

    unsigned int calcDist()
    {
        unsigned int sum = 0;
        for (int i = 0; i < cities.size() - 1; ++i)
        {
            auto diff = cities[i] - cities[i + 1];
            sum += sqrt(diff.x * diff.x + diff.y * diff.y);
        }
        return sum;
    }

    void OnUpdate(float fElapsedTime) override
    {
        Draw();
        Swap(LGE::rand(0, cities.size() - 1), LGE::rand(0, cities.size() - 1));
        unsigned int dist = calcDist();
        if (dist < bestDist)
        {
            bestAnswer = cities;
            bestDist = dist;
            std::cout << dist << "\n";
        }
    }

    void Draw()
    {
        for (int i = 0; i < cities.size(); ++i)
        {
            DrawPoint(cities[i].x, cities[i].y);
            if (i + 1 < cities.size()) DrawLine(cities[i].x, cities[i].y, cities[i + 1].x, cities[i + 1].y, { 0.0f, 1.0f, 1.0f, 1.0f });
        }

        for (int i = 0; i < bestAnswer.size(); ++i)
        {
            if (i + 1 < bestAnswer.size()) DrawLine(bestAnswer[i].x, bestAnswer[i].y, bestAnswer[i + 1].x, bestAnswer[i + 1].y, { 1.0f, 0.0f, 1.0f, 1.0f });
        }
    }

    void OnImGuiRender() override
    {
        ImGui::Text("Smallest Dist: %d", bestDist);
    }
};

class Lexico : public LGE::Scene_t
{
public:

    std::vector<int> vals = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    Lexico()
    {
        
    }

    template <typename T>
    void Swap(std::vector<T>& vals, unsigned int a, unsigned int b)
    {
        T c = vals[a];
        vals[a] = vals[b];
        vals[b] = c;
    }

    void OnUpdate(float fElapsedTime) override
    {
        // https://www.quora.com/How-would-you-explain-an-algorithm-that-generates-permutations-using-lexicographic-ordering
        
        // STEP 1
        int largestI = -1;
        for (int i = 0; i < vals.size() - 1; ++i)
        {
            if (vals[i] < vals[i + 1]) largestI = i;
        }
        if (largestI == -1)  std::cout << "FOUNDED!!!\n";


        // STEP 2
        int largestJ = -1;
        for (int j = 0; j < vals.size(); ++j)
        {
            if (vals[largestI] < vals[j]) largestJ = j;
        }


        // STEP 3
        Swap<int>(vals, largestI, largestJ);


        // STEP 4: reverse from largestI + 1, to the end
        std::reverse(vals.begin() + largestI + 1, vals.end());

    }

    void OnImGuiRender() override
    {
        
        std::string p{};
        for (int i = 0; i < vals.size(); i++)
            p += std::to_string(vals[i]) + ", ";
        ImGui::Text("Vals: %s", p.c_str());
    }
};

int main(int argc, char** argv)
{
    LGE::Application Demo;
    // Demo.RegisterScene<SceneStaticQuadTree>("Static Quad Tree");
    // Demo.RegisterScene<PixelDrawing>("Pixel Drawing");
    // Demo.RegisterScene<MinutesPhysics>("10 Minutes Physics");
    Demo.RegisterScene<TSP>("TSP");
    Demo.RegisterScene<Lexico>("Lexico");
    Demo.Run ();

    return 0;
}