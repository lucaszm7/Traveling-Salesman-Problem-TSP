#include "Application.h"

// Standart Stuff
#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <list>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>


constexpr int ScreenWidth = 1280;
constexpr int ScreenHeight = 960;

static const char* mainFileName;


class TSP : public LGE::Scene_t
{
public:

    std::string fileName;

    std::vector<glm::vec2> cities;
    std::vector<int> order;
    std::vector<glm::vec2> bestAnswer;
    unsigned int bestDist;
    unsigned int totalCities = 10;

    bool founded = false;

    TSP()
    {
        if (mainFileName != nullptr)
        {
            fileName = mainFileName;
            std::ifstream file;
            file.open(fileName);
            if (!file.is_open()) __debugbreak();
            std::string line;
            int matCount = 0;
            
            while (std::getline(file, line, ' '))
            {
                if (line == std::string{' '} || line == std::string{""}) continue;
                if (line.find('\n') != std::string::npos) { matCount++; break; }
                matCount++;
                std::cout << line << "-";
            }

            // go to the initial of the file
            file.seekg(0);

            std::cout << "\n";
            std::cout << "matCount: " << matCount << "\n";
            std::cout << "\n";

            int** adjacentMatrix = new int*[matCount];
            for (int i = 0; i < matCount; ++i)
            {
                adjacentMatrix[i] = new int[matCount];
            }

            int row = 0;
            int collum = 0;
            while (std::getline(file, line, ' ') && row < matCount)
            {
                if (line == std::string{ ' ' } || line == std::string{ "" }) continue;
                std::cout << line << "-";
                auto t = line.find('\n');
                if (t != std::string::npos) 
                { 
                    std::string num = line.substr(0, t);
                    adjacentMatrix[row][collum] = atoi(num.c_str());
                    collum = 0; row++; 
                    std::string num2 = line.substr(t, line.size()-1);
                    adjacentMatrix[row][collum] = atoi(num2.c_str());
                    collum++;
                    continue;
                }
                adjacentMatrix[row][collum] = atoi(line.c_str());
                collum++;
            }

            /*std::cout << "\n";
            std::cout << "\n";

            for (int i = 0; i < matCount; i++)
            {
                for (int j = 0; j < matCount; j++)
                {
                    std::cout << adjacentMatrix[i][j] << ", ";
                }
                std::cout << "\n";

            }*/

        }
        else
        {
            std::cout << "No Input File!\n";
        }
        std::srand(std::time(nullptr));
        bestOrder.resize(totalCities);
        for (int i = 0; i < totalCities; ++i)
        {
            cities.push_back({ LGE::rand(0, ScreenWidth), LGE::rand(0, ScreenHeight) });
            order.push_back(i);
        }
        bestAnswer = cities;
        bestDist = calcDist();
    }

    ~TSP() override = default;

    void foundExatlyTSP()
    {

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

    template <typename T>
    void Swap(std::vector<T>& vals, unsigned int a, unsigned int b) const
    {
        T c = vals[a];
        vals[a] = vals[b];
        vals[b] = c;
    }

    void Lexico()
    {
        // https://www.quora.com/How-would-you-explain-an-algorithm-that-generates-permutations-using-lexicographic-ordering

        // STEP 1
        int largestI = -1;
        for (int i = 0; i < order.size() - 1; ++i)
        {
            if (order[i] < order[i + 1]) largestI = i;
        }
        if (largestI == -1) { founded = true ; return; }


        // STEP 2
        int largestJ = -1;
        for (int j = 0; j < order.size(); ++j)
        {
            if (order[largestI] < order[j]) largestJ = j;
        }


        // STEP 3
        Swap<int>(order, largestI, largestJ);
        Swap<glm::vec2>(cities, largestI, largestJ);


        // STEP 4: reverse from largestI + 1, to the end
        std::reverse(order.begin() + largestI + 1, order.end());
        std::reverse(cities.begin() + largestI + 1, cities.end());
    }

    void OnUpdate(float fElapsedTime) override
    {
        Draw();
        for (int i = 0; i < 1000; ++i)
        {
            Lexico();
            unsigned int dist = calcDist();
            if (dist < bestDist)
            {
                bestAnswer = cities;
                bestDist = dist;
                std::cout << dist << "\n";
            }
        }
    }

    void Draw()
    {
        for (int i = 0; i < cities.size(); ++i)
        {
            DrawPoint(cities[i].x, cities[i].y);
            if (!founded)
                if (i + 1 < cities.size()) DrawLine(cities[i].x, cities[i].y, cities[i + 1].x, cities[i + 1].y, { 0.0f, 1.0f, 1.0f, 1.0f });
        }

        for (int i = 0; i < bestAnswer.size(); ++i)
        {
            if (i + 1 < bestAnswer.size()) DrawLine(bestAnswer[i].x, bestAnswer[i].y, bestAnswer[i + 1].x, bestAnswer[i + 1].y, { 1.0f, 0.0f, 1.0f, 1.0f });
        }
    }

    void OnImGuiRender() override
    {
        if(founded)
            ImGui::Text("FOUNDED!");
        ImGui::Text("Smallest Dist: %d", bestDist);
        std::string p{};
        for (int i = 0; i < order.size(); i++)
            p += std::to_string(order[i]) + ", ";
        ImGui::Text("Order: %s", p.c_str());

    }
};

int main(int argc, char** argv)
{
    if (argc > 1) mainFileName = argv[1];
    else mainFileName = nullptr;

    LGE::Application Demo;
    Demo.RegisterScene<TSP>("TSP");
    Demo.Run ();

    return 0;
}