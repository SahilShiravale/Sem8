#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <omp.h>
#include <chrono>

using namespace std;

// Class to represent an undirected graph using adjacency list
class Graph {
    int V; // Number of vertices
    vector<vector<int>> adj; // Adjacency list

public:
    Graph(int vertices) : V(vertices) {
        adj.resize(V);
    }

    // Function to add an edge to the graph (undirected)
    void addEdge(int v, int w) {
        adj[v].push_back(w);
        adj[w].push_back(v);
    }

    // Sequential BFS
    void sequentialBFS(int start) {
        vector<bool> visited(V, false);
        queue<int> q;

        visited[start] = true;
        q.push(start);

        while (!q.empty()) {
            int vertex = q.front();
            q.pop();
            cout << vertex << " ";

            for (int neighbor : adj[vertex]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
        cout << endl;
    }

    // Parallel BFS using OpenMP
    void parallelBFS(int start) {
        vector<bool> visited(V, false);
        queue<int> q;

        visited[start] = true;
        q.push(start);

        while (!q.empty()) {
            int levelSize = q.size();

            // Parallelize the processing of nodes at the current level
            #pragma omp parallel
            {
                queue<int> localQ; // Thread-local queue for next level
                #pragma omp for
                for (int i = 0; i < levelSize; i++) {
                    int vertex;
                    #pragma omp critical
                    {
                        vertex = q.front();
                        q.pop();
                    }
                    cout << vertex << " ";

                    for (int neighbor : adj[vertex]) {
                        bool shouldVisit = false;
                        #pragma omp critical
                        {
                            if (!visited[neighbor]) {
                                visited[neighbor] = true;
                                shouldVisit = true;
                            }
                        }
                        if (shouldVisit) {
                            localQ.push(neighbor);
                        }
                    }
                }

                // Merge thread-local queues into the main queue
                #pragma omp critical
                {
                    while (!localQ.empty()) {
                        q.push(localQ.front());
                        localQ.pop();
                    }
                }
            }
        }
        cout << endl;
    }

    // Sequential DFS
    void sequentialDFS(int start) {
        vector<bool> visited(V, false);
        stack<int> s;

        s.push(start);
        while (!s.empty()) {
            int vertex = s.top();
            s.pop();

            if (!visited[vertex]) {
                visited[vertex] = true;
                cout << vertex << " ";

                for (int neighbor : adj[vertex]) {
                    if (!visited[neighbor]) {
                        s.push(neighbor);
                    }
                }
            }
        }
        cout << endl;
    }

    // Parallel DFS using OpenMP
    void parallelDFS(int start) {
        vector<bool> visited(V, false);
        stack<int> s;

        s.push(start);
        while (!s.empty()) {
            int levelSize = s.size();

            // Parallelize the processing of nodes at the current stack level
            #pragma omp parallel
            {
                stack<int> localS; // Thread-local stack for next level
                #pragma omp for
                for (int i = 0; i < levelSize; i++) {
                    int vertex = -1; // Default value to indicate no vertex
                    bool proceed = false;
                    #pragma omp critical
                    {
                        if (!s.empty()) {
                            vertex = s.top();
                            s.pop();
                            proceed = true;
                        }
                    }

                    // Only proceed if we successfully popped a vertex
                    if (proceed) {
                        bool shouldProcess = false;
                        #pragma omp critical
                        {
                            if (!visited[vertex]) {
                                visited[vertex] = true;
                                shouldProcess = true;
                            }
                        }

                        if (shouldProcess) {
                            cout << vertex << " ";
                            for (int neighbor : adj[vertex]) {
                                bool shouldPush = false;
                                #pragma omp critical
                                {
                                    if (!visited[neighbor]) {
                                        visited[neighbor] = true;
                                        shouldPush = true;
                                    }
                                }
                                if (shouldPush) {
                                    localS.push(neighbor);
                                }
                            }
                        }
                    }
                }

                // Merge thread-local stacks into the main stack
                #pragma omp critical
                {
                    while (!localS.empty()) {
                        s.push(localS.top());
                        localS.pop();
                    }
                }
            }
        }
        cout << endl;
    }
};

int main() {
    // Create a graph with 6 vertices
    Graph g(6);

    // Add edges to form an undirected graph
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 3);
    g.addEdge(2, 4);
    g.addEdge(3, 4);
    g.addEdge(3, 5);
    g.addEdge(4, 5);

    int startVertex = 0;

    // Measure sequential BFS
    cout << "Sequential BFS: ";
    auto start = chrono::high_resolution_clock::now();
    g.sequentialBFS(startVertex);
    auto end = chrono::high_resolution_clock::now();
    double seq_bfs_time = chrono::duration<double, milli>(end - start).count();
    cout << "Time: " << seq_bfs_time << " ms" << endl;

    // Measure parallel BFS
    cout << "Parallel BFS: ";
    start = chrono::high_resolution_clock::now();
    g.parallelBFS(startVertex);
    end = chrono::high_resolution_clock::now();
    double par_bfs_time = chrono::duration<double, milli>(end - start).count();
    cout << "Time: " << par_bfs_time << " ms" << endl;

    // Measure sequential DFS
    cout << "Sequential DFS: ";
    start = chrono::high_resolution_clock::now();
    g.sequentialDFS(startVertex);
    end = chrono::high_resolution_clock::now();
    double seq_dfs_time = chrono::duration<double, milli>(end - start).count();
    cout << "Time: " << seq_dfs_time << " ms" << endl;

    // Measure parallel DFS
    cout << "Parallel DFS: ";
    start = chrono::high_resolution_clock::now();
    g.parallelDFS(startVertex);
    end = chrono::high_resolution_clock::now();
    double par_dfs_time = chrono::duration<double, milli>(end - start).count();
    cout << "Time: " << par_dfs_time << " ms" << endl;

    // Compute speedup
    cout << "Speedup (BFS): " << seq_bfs_time / par_bfs_time << endl;
    cout << "Speedup (DFS): " << seq_dfs_time / par_dfs_time << endl;

    // Compute efficiency
    int num_threads = omp_get_max_threads();
    cout << "Number of threads: " << num_threads << endl;
    cout << "Efficiency (BFS): " << (seq_bfs_time / par_bfs_time) / num_threads << endl;
    cout << "Efficiency (DFS): " << (seq_dfs_time / par_dfs_time) / num_threads << endl;

    return 0;
}