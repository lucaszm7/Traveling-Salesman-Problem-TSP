#include "Application.h"

// Standart Stuff
#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <list>
#include <fstream>
#include <chrono>

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
    std::vector<int> bestOrder;
    float bestDist = std::numeric_limits<float>::infinity();

    int** adjacentMatrix;

    unsigned long long totalPermutations;
    unsigned long long countPermutations = 0;

    unsigned int totalCities = 12;
    bool founded = false;

    int subSteps = 1000000;
    bool inputFile = false;

    std::chrono::steady_clock::time_point start;
    std::chrono::duration<double> duration;

    TSP()
        : duration(0)
    {
        start = std::chrono::high_resolution_clock::now();
        int nodeCount = 0;


        if (mainFileName != nullptr)
        {
            inputFile = true;
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

            nodeCount = matCount;
            adjacentMatrix = new int*[matCount];

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
        }

        else
        {
            nodeCount = totalCities;
            std::cout << "No Input File!\n";
        }

        std::srand(std::time(nullptr));
        bestOrder.resize(nodeCount);
        for (int i = 0; i < nodeCount; ++i)
        {
            cities.push_back({ LGE::rand(0, ScreenWidth), LGE::rand(0, ScreenHeight) });
            order.push_back(i);
        }
        bestOrder = order;
        bestDist = calcDist();

        totalPermutations = factorial(nodeCount);
    }

    unsigned long long factorial(int n) const
    {
        unsigned long long fac = 1;
        for (int i = 2; i <= n; ++i)
        {
            fac *= i;
        }
        return fac;
    }

    float calcDist()
    {
        float sum = 0;

        if (inputFile)
        {
            for (int i = 0; i < order.size() - 1; ++i)
            {
                sum += adjacentMatrix[order[i]][order[i + 1]];
            }
            sum += adjacentMatrix[order[0]][order[order.size() - 1]];
        }
        else
        {
            for (int i = 0; i < order.size() - 1; ++i)
            {
                auto diff = cities[order[i]] - cities[order[i + 1]];
                sum += sqrt(diff.x * diff.x + diff.y * diff.y);
            }
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


        // STEP 4: reverse from largestI + 1, to the end
        std::reverse(order.begin() + largestI + 1, order.end());
    }

    void OnUpdate(float fElapsedTime) override
    {
        Draw();
        for (int i = 0; i < subSteps; ++i)
        {
            if (founded) { countPermutations += i;  break; }

            Lexico();
            float dist = calcDist();
            if (dist < bestDist)
            {
                bestOrder = order;
                bestDist = dist;
            }
        }

        if(!founded) countPermutations += subSteps;
    }

    void Draw()
    {
        for (int i = 0; i < cities.size(); ++i)
        {
            DrawPoint(cities[i].x, cities[i].y);
        }

        for (int i = 0; i < order.size() - 1; ++i)
        {
            if (!founded)
                DrawLine(cities[order[i]].x, cities[order[i]].y, cities[order[i + 1]].x, cities[order[i + 1]].y, { 0.0f, 1.0f, 1.0f, 1.0f });
        }
        
        for (int i = 0; i < bestOrder.size() - 1; ++i)
        {
                DrawLine(cities[bestOrder[i]].x, cities[bestOrder[i]].y, cities[bestOrder[i + 1]].x, cities[bestOrder[i + 1]].y, { 0.0f, 1.0f, 0.0f, 1.0f });
        }

    }

    void OnImGuiRender() override
    {
        std::string c{};
        std::string o{};
        if(founded)
            ImGui::Text("FOUNDED!");
        duration = std::chrono::high_resolution_clock::now() - start;
        ImGui::Text("Time taked: %.2f s", duration.count());
        double percentage = (((double)countPermutations * 100.0f) / (double)totalPermutations);
        ImGui::Text("Completeness: %.2f%%", percentage);
        ImGui::Text("Tested %.2f of %.2f", (double)countPermutations, (double)totalPermutations);
        ImGui::Text("Smallest Dist: %.2f", bestDist);
        for (int i = 0; i < order.size(); i++)
            o += std::to_string(bestOrder[i]) + ", ";
        ImGui::Text("Best Order: \n%s", o.c_str());
        for (int i = 0; i < order.size(); i++)
            c += std::to_string(order[i]) + ", ";
        ImGui::Text("Order: \n%s", c.c_str());


    }
};


int main(int argc, char** argv)
{
    if (argc > 1)
    {
        mainFileName = argv[1];
        std::cout << mainFileName << "\n";
    }
    else mainFileName = nullptr;

    LGE::Application Demo;
    Demo.RegisterScene<TSP>("TSP");
    Demo.Run ();

    return 0;
}