#include <iostream>
#include <vector>
#include <omp.h>
#include <stack>
#include <chrono>
#include <atomic>

using namespace std;

const int MAXN = 1e5;
vector<int> adj[MAXN + 5]; // adjacency list
atomic<bool> visited[MAXN + 5]; // mark visited nodes
vector<int> visitedOrder; // Store the order of visited nodes

// Function to print time complexity analysis
void printTimeComplexity(int n, int m) {
    cout << "\nTime Complexity Analysis:\n";
    cout << "Sequential DFS:\n";
    cout << "  - Time Complexity: O(V + E) = O(" << n << " + " << m << ")\n";
    cout << "    - Visiting each vertex once: O(V) = O(" << n << ")\n";
    cout << "    - Processing all edges: O(E) = O(" << m << ")\n";
    cout << "  - Space Complexity: O(V) = O(" << n << ") for stack and visited array\n";
    cout << "Parallel DFS:\n";
    cout << "  - Time Complexity: O(V + E/P) in ideal case, where P is number of threads\n";
    cout << "    - Actual performance depends on thread overhead and synchronization\n";
    cout << "    - Critical section for stack updates may reduce parallelism\n";
    cout << "  - Space Complexity: O(V) = O(" << n << ") for stack and visited array\n";
    cout << "Note: V = number of vertices (" << n << "), E = number of edges (" << m << ")\n";
}

// Sequential DFS (Iterative version)
void sequentialDFS(int start, int n) {
    for (int i = 1; i <= n; i++) visited[i] = false;
    visitedOrder.clear();

    stack<int> s;
    s.push(start);
    visited[start] = true;
    visitedOrder.push_back(start);

    while (!s.empty()) {
        int node = s.top();
        s.pop();

        for (int next_node : adj[node]) {
            if (!visited[next_node]) {
                visited[next_node] = true;
                visitedOrder.push_back(next_node);
                s.push(next_node);
            }
        }
    }
}

// Parallel DFS using OpenMP (Iterative version)
void parallelDFS(int start, int n, int numThreads) {
    omp_set_num_threads(numThreads);
    for (int i = 1; i <= n; i++) visited[i] = false;
    visitedOrder.clear();

    stack<int> s;
    s.push(start);
    visited[start] = true;
    visitedOrder.push_back(start);

    while (!s.empty()) {
        int node = s.top();
        s.pop();

        // Parallelizing the processing of neighbors
        #pragma omp parallel for
        for (int i = 0; i < adj[node].size(); i++) {
            int next_node = adj[node][i];
            if (!visited[next_node]) {
                visited[next_node] = true;
                #pragma omp critical
                {
                    visitedOrder.push_back(next_node);
                    s.push(next_node);
                }
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

    // Sequential DFS
    auto start = chrono::high_resolution_clock::now();
    sequentialDFS(start_node, n);
    auto end = chrono::high_resolution_clock::now();
    double seq_time = chrono::duration<double, milli>(end - start).count();

    vector<int> seqVisitedOrder = visitedOrder; // Store sequential order for comparison

    // Parallel DFS
    start = chrono::high_resolution_clock::now();
    parallelDFS(start_node, n, numThreads);
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
    cout << "Sequential DFS Time: " << seq_time << " ms\n";
    cout << "Parallel DFS Time: " << par_time << " ms\n";
    cout << "Speedup: " << (seq_time / par_time) << "\n";
    cout << "Threads Used: " << numThreads << "\n";
    cout << "Efficiency: " << (seq_time / par_time) / numThreads << "\n";
    cout << "Correctness: " << (correct ? "Pass" : "Fail") << "\n";

    // Print visited nodes
    cout << "Visited nodes: ";
    for (int node : visitedOrder) {
        cout << node << " ";
    }
    cout << "\n";

    // Print time complexity
    printTimeComplexity(n, m);

    return 0;
}
/*$ ./parallel_dfs.exe
Enter number of nodes, edges, and threads: 10 9 8
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
Sequential DFS Time: 0.0281 ms
Parallel DFS Time: 7.4499 ms
Speedup: 0.00377186
Threads Used: 8
Efficiency: 0.000471483
Correctness: Pass
Visited nodes: 1 2 3 4 5 6 7 8 9 10
*/


/*Enter number of nodes, edges, and threads: 15 14 12
Enter start node: 1
Enter 14 edges (format: u v):
1 2
2 3
3 4
4 5
5 6
6 7
7 8
8 9
9 10
10 11
11 12
12 13
13 14
14 15
Sequential DFS Time: 0.0162 ms
Parallel DFS Time: 3.9135 ms
Speedup: 0.00413952
Threads Used: 12
Efficiency: 0.00034496
Correctness: Pass
Visited nodes: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
*/

/*Enter number of nodes, edges, and threads: 10 9 4
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
Sequential DFS Time: 0.0154 ms
Parallel DFS Time: 4.059 ms
Speedup: 0.00379404
Threads Used: 4
Efficiency: 0.000948509
Correctness: Pass
Visited nodes: 1 2 3 4 5 6 7 8 9 10
*/

/*The claim that increasing the number of threads (e.g., to 50 threads) will automatically increase the efficiency is false, especially when dealing with graph traversal problems like DFS.

Here's why:

1. Parallel Overhead:
When you increase the number of threads beyond a certain point, the overhead from managing and synchronizing the threads can outweigh the potential benefits of parallelism. This is particularly true for problems that have a small amount of work per thread or have low parallelizability. In such cases, having more threads leads to:

Increased context switching: The system has to manage more threads, which can lead to unnecessary switching between them.

Synchronization overhead: The need to synchronize shared data (like visited nodes) between threads can become a bottleneck.

Resource contention: If your system doesn’t have enough physical cores to handle the threads, it can lead to slower performance due to resource contention.

2. Diminishing Returns:
As you add more threads, you may observe diminishing returns after a certain point. For example:

With 4 threads, you might see significant performance improvement.

With 50 threads, the system might not be able to take advantage of all the threads efficiently because the problem might not be parallel enough, or the system might not have enough CPU cores to handle all threads effectively.

3. Efficiency Formula:
Efficiency is defined as:

Efficiency
=
Speedup
Number of Threads
Efficiency= 
Number of Threads
Speedup
​
 
If the speedup you get from increasing the number of threads is less than proportional to the number of threads, the efficiency decreases. In many cases, the number of threads needed to reach optimal performance is far less than 50.

4. Empirical Example:
In your output with 4 threads:

Speedup: 0.00379

Efficiency: 0.000948509

If you increase the number of threads to 50, the speedup might not increase significantly, and efficiency could decrease further, possibly leading to negative effects on performance.

Conclusion:
Increasing the number of threads to 50 in your case will not necessarily improve efficiency and might actually make it worse, depending on the problem size and your system's architecture. It's essential to experiment with thread counts and find an optimal number based on your hardware and the problem's characteristics.

If your system has fewer than 50 physical cores, using more than 50 threads might not help and could hurt performance. You should monitor the speedup and efficiency as you adjust the number of threads to determine the best configuration.*/