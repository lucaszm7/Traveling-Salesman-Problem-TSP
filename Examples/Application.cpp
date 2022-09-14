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

static const char* fileNames[]{ "tsp1_253.txt","tsp2_1248.txt","tsp3_1194.txt","tsp4_7013.txt","tsp5_27603.txt" };
static int selectedFile = 0;

class TSP : public LGE::Scene_t
{
public:

    std::string fileName;

    std::vector<glm::vec2> cities;
    std::vector<int> order;
    std::vector<int> bestOrder;
    std::vector<int> bestDisplayOrder;
    float bestDist = std::numeric_limits<float>::infinity();
    float bestDisplayDist = std::numeric_limits<float>::infinity();

    int nodeCount = 0;
    int** adjacentMatrix;

    unsigned long long totalPermutations = 0;
    unsigned long long countPermutations = 0;

    bool founded = false;
    bool makeFile = false;
    int fileCount = 1;

    int subSteps = 100000;

    std::chrono::steady_clock::time_point start;
    std::chrono::duration<double> duration{0};

    TSP()
    {
        reset();
    }

    void reset()
    {
        std::srand(std::time(nullptr));
        start = std::chrono::high_resolution_clock::now();

        countPermutations = 0;
        founded = false;
        makeFile = false;

        genAdjacentMatrix();
        initGraph();

        totalPermutations = factorial(nodeCount);
    }

    void genAdjacentMatrix()
    {
        nodeCount = 0;

        fileName = fileNames[selectedFile];
        std::ifstream file;
        file.open(fileName);
        if (!file.is_open()) __debugbreak();
        std::string line;
        int matCount = 0;

        char sapChar;
        if (selectedFile == 3) sapChar = '\t';
        else sapChar = ' ';

        while (std::getline(file, line, sapChar))
        {
            if (line == std::string{ ' ' } || line == std::string{ "" } ||
                line == std::string{ '\t' }) continue;
            if (line == std::string{ '\n' }) break;
            if (line.find('\n') != std::string::npos)
            {
                if (line.find('\n') != 0)
                    matCount++;
                break;
            }
            matCount++;
        }

        // go to the initial of the file
        file.seekg(0);

        std::cout << "Nodes Count: " << matCount << "\n";

        nodeCount = matCount;
        adjacentMatrix = new int* [matCount];

        for (int i = 0; i < matCount; ++i)
        {
            adjacentMatrix[i] = new int[matCount];
        }

        int row = 0;
        int collum = 0;


        while (std::getline(file, line, sapChar) && row < matCount)
        {
            if (line == std::string{ ' ' } || line == std::string{ "" } ||
                line == std::string{ '\t' } || line == std::string{ '\n' }) continue;

            auto t = line.find('\n');
            if (t != std::string::npos)
            {
                std::string num = line.substr(0, t);
                adjacentMatrix[row][collum] = atoi(num.c_str());
                collum = 0; row++;
                std::string num2 = line.substr(t, line.size() - 1);
                adjacentMatrix[row][collum] = atoi(num2.c_str());
                collum++;
                continue;
            }
            adjacentMatrix[row][collum] = atoi(line.c_str());
            collum++;
        }

    }

    void initGraph()
    {
        cities.clear();
        order.clear();
        for (int i = 0; i < nodeCount; ++i)
        {
            cities.push_back({ LGE::rand(0, ScreenWidth), LGE::rand(0, ScreenHeight) });
            order.push_back(i);
        }
        bestDisplayOrder = order;
        bestOrder = order;
        bestDist = calcDist().x; 
        bestDisplayDist = calcDist().y;
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

    glm::vec2 calcDist()
    {
        glm::vec2 sum = {0, 0};

        for (int i = 0; i < order.size() - 1; ++i)
        {
            sum.x += (float)adjacentMatrix[order[i]][order[i + 1]];
        }
        sum.x += (float)adjacentMatrix[order[0]][order[order.size() - 1]];

        for (int i = 0; i < order.size() - 1; ++i)
        {
            auto diff = cities[order[i]] - cities[order[i + 1]];
            sum.y += sqrt(diff.x * diff.x + diff.y * diff.y);
        }

        auto diff = cities[order[0]] - cities[order[order.size() - 1]];
        sum.y += sqrt(diff.x * diff.x + diff.y * diff.y);
        
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

    void genOutputFile()
    {
        makeFile = true;
        auto fileOutputName = fileName.substr(0, fileName.find_first_of('.'));
        double percentage = (((double)countPermutations * 100.0f) / (double)totalPermutations);
        fileOutputName += "_exact_results.txt";
        std::cout << "\nOutput file: " << fileOutputName << "\n";

        std::ofstream results(fileOutputName);
        results << "Time Taken: " << std::to_string(duration.count()) << "s\n";
        results << "Percentage Complete: " << percentage << "%\n";
        std::string o{};
        for (int i = 0; i < order.size(); i++)
            o += std::to_string(bestOrder[i]) + ", ";
        results << "Best Order: " << o << "\n";
        results << "Minimal Distance: " << bestDist << "\n";
        results.close();
    }

    void OnUpdate(float fElapsedTime) override
    {
        Draw();
        for (int i = 0; i < subSteps; ++i)
        {
            if (founded)
            { 
                countPermutations += i;  
                if (!makeFile) { genOutputFile(); makeFile = true; }
                break;
            }

            Lexico();
            glm::vec2 dist = calcDist();

            if (dist.x < bestDist)
            {
                bestOrder = order;
                bestDist = dist.x;
            }
            if (dist.y < bestDisplayDist)
            {
                bestDisplayOrder = order;
                bestDisplayDist = dist.y;
            }
        }
        if (!founded) 
            countPermutations += subSteps;
    }

    void Draw()
    {
        for (const auto& city : cities)
        {
            DrawPoint(city.x, city.y);
        }

        for (int i = 0; i < order.size() - 1; ++i)
        {
            if (!founded)
                DrawLine(cities[order[i]].x, cities[order[i]].y, cities[order[i + 1]].x, cities[order[i + 1]].y, { 0.0f, 1.0f, 1.0f, 1.0f });
        }
        
        for (int i = 0; i < bestDisplayOrder.size() - 1; ++i)
        {
            DrawLine(cities[bestDisplayOrder[i]].x, cities[bestDisplayOrder[i]].y, cities[bestDisplayOrder[i + 1]].x, cities[bestDisplayOrder[i + 1]].y, { 0.0f, 1.0f, 0.0f, 1.0f });
        }
        DrawLine(cities[bestDisplayOrder[0]].x, cities[bestDisplayOrder[0]].y, cities[bestDisplayOrder[bestDisplayOrder.size() - 1]].x, cities[bestDisplayOrder[bestDisplayOrder.size() - 1]].y, { 0.0f, 1.0f, 0.0f, 1.0f });

    }

    void OnImGuiRender() override
    {
        std::string c{};
        std::string o{};

        ImGui::Combo(" ", &selectedFile, fileNames, IM_ARRAYSIZE(fileNames));
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
            reset();

        if(!founded) duration = std::chrono::high_resolution_clock::now() - start;
        ImGui::Text("Time taked: %.2f s", duration.count());

        double percentage = (((double)countPermutations * 100.0f) / (double)totalPermutations);
        ImGui::Text("Completeness: %.6f%%", percentage);
        if (founded)
        {
            ImGui::SameLine();
            ImGui::Text("FINISHED!");
        }

        ImGui::Text("Smallest Dist: %.2f", bestDist);
        for (int i = 0; i < order.size(); i++)
            o += std::to_string(bestOrder[i]) + ", ";
        ImGui::Text("Best Order: \n%s", o.c_str());

        for (const auto& ord : order)
            c += std::to_string(ord) + ", ";
        ImGui::Text("Order: \n%s", c.c_str());
    }
};


int main(int argc, char** argv)
{
    LGE::Application Demo;
    Demo.RegisterScene<TSP>("TSP");
    Demo.Run ();

    return 0;
}