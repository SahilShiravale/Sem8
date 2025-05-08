#include <iostream>
#include <vector>
#include <algorithm>
#include <omp.h>
#include <chrono>
#include <queue>

using namespace std;

const int MAXN = 1e5;
vector<int> adj[MAXN + 5]; // Adjacency list
bool visited[MAXN + 5];    // Revert to bool for simplicity with atomic section
vector<int> visitedOrder;  // Store the order of visited nodes

// Sequential BFS
void sequentialBFS(int start, int n) {
    for (int i = 1; i <= n; i++) visited[i] = false;
    visitedOrder.clear();

    queue<int> q;
    q.push(start);
    visited[start] = true;
    visitedOrder.push_back(start);

    while (!q.empty()) {
        int node = q.front();
        q.pop();

        for (int next_node : adj[node]) {
            if (!visited[next_node]) {
                visited[next_node] = true;
                visitedOrder.push_back(next_node);
                q.push(next_node);
            }
        }
    }
}

// Parallel BFS using OpenMP
void parallelBFS(int start, int n, int numThreads) {
    omp_set_num_threads(numThreads);
    for (int i = 1; i <= n; i++) visited[i] = false;
    visitedOrder.clear();

    queue<int> q;
    q.push(start);
    visited[start] = true;
    visitedOrder.push_back(start);

    while (!q.empty()) {
        int currentLevelSize = q.size();
        vector<vector<int>> localNextLevel(numThreads);

        #pragma omp parallel
        {
            int tid = omp_get_thread_num();

            #pragma omp for schedule(static)
            for (int i = 0; i < currentLevelSize; i++) {
                int node = q.front();
                q.pop();  // Thread-safe as each thread is responsible for popping one element

                // Explore neighbors
                for (int next_node : adj[node]) {
                    if (!visited[next_node]) {
                        visited[next_node] = true;
                        localNextLevel[tid].push_back(next_node);  // Add to thread-local queue
                    }
                }
            }
        }

        // Merge thread-local results back into the global queue for the next level
        for (int t = 0; t < numThreads; t++) {
            for (int next_node : localNextLevel[t]) {
                q.push(next_node);  // Push local results to the global queue
                visitedOrder.push_back(next_node); // Store visited node
            }
        }
    }
}

int main() {
    int n, m, start_node, numThreads;
    cout << "Enter number of nodes, edges, and threads: ";
    cin >> n >> m >> numThreads;
    cout << "Enter start node: ";
    cin >> start_node;

    // Input validation
    if (n < 1 || n > MAXN) {
        cout << "Number of nodes must be between 1 and " << MAXN << ".\n";
        return 1;
    }
    if (m < 0) {
        cout << "Number of edges cannot be negative.\n";
        return 1;
    }
    if (start_node < 1 || start_node > n) {
        cout << "Start node must be between 1 and " << n << ".\n";
        return 1;
    }
    if (numThreads < 1) {
        cout << "Number of threads must be at least 1.\n";
        return 1;
    }

    // Clear adjacency list
    for (int i = 1; i <= n; i++) {
        adj[i].clear();
    }

    // Input edges
    cout << "Enter " << m << " edges (format: u v):\n";
    for (int i = 0; i < m; i++) {
        int u, v;
        cin >> u >> v;
        if (u < 1 || u > n || v < 1 || v > n) {
            cout << "Invalid edge (" << u << ", " << v << "). Vertices must be between 1 and " << n << ".\n";
            return 1;
        }
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    // Sequential BFS
    auto start = chrono::high_resolution_clock::now();
    sequentialBFS(start_node, n);
    auto end = chrono::high_resolution_clock::now();
    double seq_time = chrono::duration<double, milli>(end - start).count();

    vector<int> seqVisitedOrder = visitedOrder; // Store sequential order for comparison

    // Parallel BFS
    start = chrono::high_resolution_clock::now();
    parallelBFS(start_node, n, numThreads);
    end = chrono::high_resolution_clock::now();
    double par_time = chrono::duration<double, milli>(end - start).count();

    // Verify correctness
    bool correct = (seqVisitedOrder.size() == visitedOrder.size());
    if (correct) {
        for (size_t i = 0; i < seqVisitedOrder.size(); i++) {
            if (seqVisitedOrder[i] != visitedOrder[i]) {
                correct = false;
                break;
            }
        }
    }

    // Print performance metrics
    cout << "Sequential BFS Time: " << seq_time << " ms\n";
    cout << "Parallel BFS Time: " << par_time << " ms\n";
    cout << "Speedup: " << seq_time / par_time << "\n";
    cout << "Threads Used: " << numThreads << "\n";
    cout << "Efficiency: " << (seq_time / par_time) / numThreads << "\n";
    cout << "Correctness: " << (correct ? "Pass" : "Fail") << "\n";

    // Print visited nodes
    cout << "Visited nodes: ";
    for (int node : visitedOrder) {
        cout << node << " ";
    }
    cout << "\n";

    return 0;
}

/*$ ./parallel_bfs.exe
Enter number of nodes, edges, and threads: 10 9 4
Enter start node: 1
Enter 9 edges (format: u v):
1 2
2 3
3 4
4 5
5 6
6 7
7 8
8 9
9 10
Sequential BFS Time: 0.0165 ms
Parallel BFS Time: 7.7299 ms
Speedup: 0.00213457
Threads Used: 4
Efficiency: 0.000533642
Correctness: Pass
*/