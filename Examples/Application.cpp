#include "Application.h"

// Standart Stuff
#include <iostream>
#include <thread>
#include <memory>
#include <vector>
#include <array>
#include <list>
#include <fstream>
#include <chrono>
#include <utility>
#include <functional>
#include <omp.h>

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
    std::vector<int> bestExactOrder;
    std::vector<int> bestAproxOrder;
    std::vector<int> bestDisplayOrder;

    float bestExactDist = std::numeric_limits<float>::infinity();
    float bestAproxDist = std::numeric_limits<float>::infinity();
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
    std::chrono::duration<double> durationExact{0};
    double durationAprox{0};

    // Threads
    const unsigned int numThreads = omp_get_num_procs();
    unsigned long long threadsTotalPermutations;

    std::vector<std::vector<int>> threadsOrder;
    std::vector<std::vector<int>> threadsBestOrder;
    std::vector<float> threadsBestDist;
    std::vector<std::vector<int>> threadsBestDisplayOrder;
    std::vector<float> threadsBestDisplayDist;

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

        bestExactDist = std::numeric_limits<float>::infinity();
        bestAproxDist = std::numeric_limits<float>::infinity();
        bestDisplayDist = std::numeric_limits<float>::infinity();

        genAdjacentMatrix();
        initGraph();

        totalPermutations = factorial(nodeCount - 1);
        LGE::Timer t;
        twiceAroundTheTree();
        durationAprox = t.now();

        // Init threads
        omp_set_num_threads(numThreads);
        threadsOrder.resize(numThreads);
        threadsBestOrder.resize(numThreads);
        threadsBestDist.resize(numThreads);
        threadsBestDisplayOrder.resize(numThreads);
        threadsBestDisplayDist.resize(numThreads);

        threadsTotalPermutations = totalPermutations / numThreads;
        for (int i = 0; i < numThreads; ++i)
        {
            std::list<int> orderList;
            std::copy(order.begin(), order.end(), std::back_inserter(orderList));
            std::list<int> thisOrderList = GetThLexicoOrder(orderList, i * threadsTotalPermutations);
            std::vector<int> thisOrderVec{ std::begin(thisOrderList), std::end(thisOrderList) };
            std::cout << "Order result: \n";
            for (const auto& a : thisOrderVec)
                std::cout << a << ",";
            std::cout << "\n";

            // Seters
            threadsBestDist[i] = std::numeric_limits<float>::infinity();
            threadsBestDisplayDist[i] = std::numeric_limits<float>::infinity();
            threadsBestOrder[i] = threadsBestDisplayOrder[i] = threadsOrder[i] = thisOrderVec;

        }
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
        bestExactOrder = order;
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

    glm::vec2 calcExactDist(int threadId)
    {
        // Exact
        glm::vec2 sum = {0, 0};

        for (int i = 0; i < threadsOrder[threadId].size() - 1; ++i)
        {
            sum.x += (float)adjacentMatrix[threadsOrder[threadId][i]][threadsOrder[threadId][i + 1]];
        }
        sum.x += (float)adjacentMatrix[threadsOrder[threadId][0]][threadsOrder[threadId][threadsOrder[threadId].size() - 1]];

        for (int i = 0; i < threadsOrder[threadId].size() - 1; ++i)
        {
            auto diff = cities[threadsOrder[threadId][i]] - cities[threadsOrder[threadId][i + 1]];
            sum.y += sqrt(diff.x * diff.x + diff.y * diff.y);
        }

        auto diff = cities[threadsOrder[threadId][0]] - cities[threadsOrder[threadId][threadsOrder[threadId].size() - 1]];
        sum.y += sqrt(diff.x * diff.x + diff.y * diff.y);
        
        return sum;
    }

    int calcAproxDist()
    {
        // Aprox
        int aproxWeight = 0;
        for (int i = 0; i < bestAproxOrder.size() - 1; ++i)
        {
            aproxWeight += (float)adjacentMatrix[bestAproxOrder[i]][bestAproxOrder[i + 1]];
        }
        aproxWeight += (float)adjacentMatrix[bestAproxOrder[0]][bestAproxOrder[bestAproxOrder.size() - 1]];

        
        return aproxWeight;
    }

    void twiceAroundTheTree()
    {
        // STEP 1: Complete Graph - Adjacent Matrix

        struct DisjointSets
        {
            int* parent, * rnk;
            int n;

            DisjointSets(int n)
                : n(n)
            {
                parent = new int[n + 1];
                rnk = new int[n + 1];

                for (int i = 0; i <= n; i++)
                {
                    rnk[i] = 0;
                    parent[i] = i;
                }
            }

            int find(int u)
            {
                if (u != parent[u])
                    parent[u] = find(parent[u]);
                return parent[u];
            }

            void merge(int x, int y)
            {
                x = find(x); y = find(y);

                if (rnk[x] > rnk[y])
                    parent[y] = x;
                else // If rnk[x] <= rnk[y]
                    parent[x] = y;

                if (rnk[x] == rnk[y])
                    rnk[y]++;
            }
        };


        // STEP 2: Find Minimum Spanning Tree (kruskal)
        std::vector<std::pair<int, glm::vec2>> weights;
        std::vector<glm::vec2> mst;
        std::vector<bool> addedIndex;
        weights.reserve(nodeCount * nodeCount);
        addedIndex.reserve(nodeCount);


        for (int i = 0; i < nodeCount; ++i)
        {
            for (int j = 0; j < nodeCount; ++j)
            {
                if (adjacentMatrix[i][j] == 0) continue;
                std::pair<int, glm::vec2> map;
                map.first = adjacentMatrix[i][j];
                map.second = glm::vec2(i, j);
                weights.push_back( map );
            }
        }
        
        std::sort(weights.begin(), weights.end(), [](const std::pair<int, glm::vec2>& a, const std::pair<int, glm::vec2>& b) {
            return a.first < b.first; });

        /*std::cout << "Weight of Edge - Edge:\n";
        for (const auto& a : weights)
        {
            std::cout << a.first << " - " << a.second.x << ", " << a.second.y << "\n";
        }*/

        DisjointSets ds(nodeCount);

        std::vector<std::pair<int, glm::vec2> >::iterator it;
        for (it = weights.begin(); it != weights.end(); it++)
        {
            int u = it->second.x;
            int v = it->second.y;

            int set_u = ds.find(u);
            int set_v = ds.find(v);

            // Check if the selected edge is creating
            // a cycle or not (Cycle is created if u
            // and v belong to same set)
            if (set_u != set_v)
            {
                // Current edge will be in the MST
                mst.push_back({ u, v });

                // Merge two sets
                ds.merge(set_u, set_v);
            }
        }

        /*std::cout << "Minimal Spanning Tree:\n";
        for (const auto& a : mst)
        {
            std::cout << a.x << ", " << a.y << "\n";
        }*/

        // STEP 3: Deep First Search
        // STEP 4: Delete duplicated Vertices

        std::unordered_map<int, std::list<int>> edges;
        for (auto& edge : mst)
        {
            edges[edge.x].push_back(edge.y);
            edges[edge.y].push_back(edge.x);
        }

        std::vector<int> dfs;
        dfs.reserve(nodeCount);
        std::vector<bool> visited;
        visited.resize(nodeCount);

        std::function<void(int v)> make_dfs;
        make_dfs = [&](int v)
        {
            visited[v] = true;
            dfs.push_back(v);

            for (auto i = edges[v].begin(); i != edges[v].end(); ++i)
            {
                if (!visited[(*i)])
                    make_dfs(*i);
            }
            return;
        };

        make_dfs(ds.find(0));
        bestAproxOrder = dfs;
        bestAproxDist = calcAproxDist();

        for (int i = 0; i < nodeCount; ++i)
        {
            make_dfs(ds.find(i));
            auto dist = calcAproxDist();
            if (dist < bestAproxDist)
            {
                bestAproxDist = dist;
                bestAproxOrder = dfs;
            }
        }
        /*std::cout << "Deepth First Search: \n";
        for (auto a : dfs)
        {
            std::cout << a << " - ";
        }
        std::cout << "\n";*/

        
    }

    template <typename T>
    void Swap(std::vector<T>& vals, unsigned int a, unsigned int b) const
    {
        T c = vals[a];
        vals[a] = vals[b];
        vals[b] = c;
    }

    bool Lexico(int threadId)
    {
        // https://www.quora.com/How-would-you-explain-an-algorithm-that-generates-permutations-using-lexicographic-ordering

        // STEP 1
        int largestI = -1;
        for (int i = 1; i < threadsOrder[threadId].size() - 1; ++i)
        {
            if (threadsOrder[threadId][i] < threadsOrder[threadId][i + 1]) largestI = i;
        }
        if (largestI == -1) { founded = true ; return true; }


        // STEP 2
        int largestJ = -1;
        for (int j = 1; j < threadsOrder[threadId].size(); ++j)
        {
            if (threadsOrder[threadId][largestI] < threadsOrder[threadId][j]) largestJ = j;
        }


        // STEP 3
        Swap<int>(threadsOrder[threadId], largestI, largestJ);


        // STEP 4: reverse from largestI + 1, to the end
        std::reverse(threadsOrder[threadId].begin() + largestI + 1, threadsOrder[threadId].end());
        
        return false;
    }

    std::list<int> GetThLexicoOrder(std::list<int> order, unsigned long long permutation)
    {
        std::list<int> result;

        int nFactorial = order.size() - 1;
        unsigned long long totalFactorial = factorial(nFactorial);
        int actualIndice = -1;
        while (nFactorial >= 0)
        {
            actualIndice = permutation / totalFactorial;
            permutation = permutation % totalFactorial;
            totalFactorial = factorial(--nFactorial);

            auto actualValue = order.begin();
            std::advance(actualValue, actualIndice);
            result.push_back(*actualValue);
            order.erase(actualValue);
        }

        return result;
    }

    void genOutputFile()
    {
        for (int j = 0; j < numThreads; ++j)
        {
            if (threadsBestDist[j] < bestExactDist)
            {
                bestExactDist = threadsBestDist[j];
                bestExactOrder = threadsBestOrder[j];
            }
        }
        makeFile = true;
        auto fileOutputName = fileName.substr(0, fileName.find_first_of('.'));
        double percentage = (((double)countPermutations * 100.0) / (double)threadsTotalPermutations);
        fileOutputName += "_results.txt";
        std::cout << "\nOutput file: " << fileOutputName << "\n";

        std::ofstream results(fileOutputName);
        results << "-------------------------------------------------------------------\n";
        results << "------------------------- Exact Algorithm -------------------------\n";
        results << "Exact Time Taken: " << std::to_string(durationExact.count()) << "s\n";
        results << "Percentage Complete: " << percentage << "%\n";
        std::string o{}, a{};
        for (int i = 0; i < bestExactOrder.size(); i++)
            o += std::to_string(bestExactOrder[i]) + ", ";
        for (int i = 0; i < bestAproxOrder.size(); i++)
            a += std::to_string(bestAproxOrder[i]) + ", ";
        results << "Best Exact Order: " << o << "\n";
        results << "Minimal Exact Distance: " << bestExactDist << "\n";
        results << "-------------------------------------------------------------------\n";
        results << "------------------------- 2-Aprox Algorithm -----------------------\n";
        results << "Aprox Time Taken: " << durationAprox << "\n";
        results << "Best Aprox Order: " << a << "\n";
        results << "Minimal Aprox Distance: " << bestAproxDist << "\n";
        results << "--------------------------------------------------------------------\n";

        results.close();
    }

    void OnUpdate(float fElapsedTime) override
    {
        Draw();
        
        unsigned long long restPermutations = 0;
        #pragma omp parallel
        {
            int threadId = omp_get_thread_num();
            for (int i = 0; i < subSteps; ++i)
            {
                if (founded) break;

                if(Lexico(threadId)) restPermutations = i + 1;
                glm::vec2 dist = calcExactDist(threadId);

                if (dist.x < threadsBestDist[threadId])
                {
                    threadsBestOrder[threadId] = threadsOrder[threadId];
                    threadsBestDist[threadId] = dist.x;
                }
                if (dist.y < threadsBestDisplayDist[threadId])
                {
                    threadsBestDisplayOrder[threadId] = threadsOrder[threadId];
                    threadsBestDisplayDist[threadId] = dist.y;
                }
            }

            #pragma omp single
            {
                if (!founded)
                    countPermutations += subSteps;
                else
                {
                    if (!makeFile) 
                    {
                        countPermutations += restPermutations;
                        for (int j = 0; j < numThreads; ++j)
                        {
                            if (threadsBestDist[j] < bestExactDist)
                            {
                                bestExactDist = threadsBestDist[j];
                                bestExactOrder = threadsBestOrder[j];
                            }
                            
                            if (threadsBestDisplayDist[j] < bestDisplayDist)
                            {
                                bestDisplayDist = threadsBestDisplayDist[j];
                                bestDisplayOrder = threadsBestDisplayOrder[j];
                            }

                        }

                        threadsBestDist[0] = bestExactDist;
                        threadsBestDisplayOrder[0] = bestDisplayOrder;

                        genOutputFile(); 
                        makeFile = true; 
                    }
                }
            }
        }
    }

    void Draw()
    {
        for (const auto& city : cities)
        {
            DrawPoint(city.x, city.y);
        }

        for (int i = 0; i < threadsOrder[0].size() - 1; ++i)
        {
            if (!founded)
                DrawLine(cities[threadsOrder[0][i]].x, cities[threadsOrder[0][i]].y, cities[threadsOrder[0][i + 1]].x, cities[threadsOrder[0][i + 1]].y, { 0.0f, 1.0f, 1.0f, 1.0f });
        }
        
        for (int i = 0; i < threadsBestDisplayOrder[0].size() - 1; ++i)
        {
            DrawLine(cities[threadsBestDisplayOrder[0][i]].x, cities[threadsBestDisplayOrder[0][i]].y, cities[threadsBestDisplayOrder[0][i + 1]].x, cities[threadsBestDisplayOrder[0][i + 1]].y, { 0.0f, 1.0f, 0.0f, 1.0f });
        }
        DrawLine(cities[threadsBestDisplayOrder[0][0]].x, cities[threadsBestDisplayOrder[0][0]].y, cities[threadsBestDisplayOrder[0][threadsBestDisplayOrder[0].size() - 1]].x, cities[threadsBestDisplayOrder[0][threadsBestDisplayOrder[0].size() - 1]].y, { 0.0f, 1.0f, 0.0f, 1.0f });
    }

    void OnImGuiRender() override
    {
        std::string c{};
        std::string o{};
        std::string a{};

        ImGui::Combo(" ", &selectedFile, fileNames, IM_ARRAYSIZE(fileNames));
        ImGui::SameLine();
        if (ImGui::Button("Reset"))
            reset();

        if (ImGui::Button("Gen Result File"))
            genOutputFile();

        if(!founded) durationExact = std::chrono::high_resolution_clock::now() - start;
        ImGui::Text("Exact Time taked: %.2f s", durationExact.count());

        double percentage = (((double)countPermutations * 100.0f) / (double)threadsTotalPermutations);
        ImGui::Text("Completeness: %.6f%%", percentage);
        if (founded)
        {
            ImGui::SameLine();
            ImGui::Text("FINISHED!");
        }

        for (const auto& ord : threadsOrder[0])
            c += std::to_string(ord) + ", ";
        ImGui::Text("Order: \n%s\n", c.c_str());

        ImGui::Separator();

        ImGui::Text("Smallest Exact Dist: %.2f", threadsBestDist[0]);
        for (int i = 0; i < order.size(); i++)
            o += std::to_string(bestExactOrder[i]) + ", ";
        ImGui::Text("Best Exact Order: \n%s\n", o.c_str());
        
        ImGui::Separator();

        ImGui::Text("Aprox Time Take: %.6f", durationAprox);
        ImGui::Text("Smallest Aprox Dist: %.2f", bestAproxDist);
        for (int i = 0; i < bestAproxOrder.size(); i++)
            a += std::to_string(bestAproxOrder[i]) + ", ";
        ImGui::Text("Best Aprox Order: \n%s", a.c_str());
        
    }
};


int main(int argc, char** argv)
{
    LGE::Application Demo;
    Demo.RegisterScene<TSP>("TSP");
    Demo.Run ();

    return 0;
}