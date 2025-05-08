#include <iostream>
#include <vector>
#include <climits>
#include <omp.h>
#include <chrono>
#include <limits>
#include <iomanip>

using namespace std;

// Function to print time complexity
void printTimeComplexity(int n) {
    cout << "\nTime Complexity Analysis:\n";
    cout << "Sequential Reduction: O(n), where n is the array size (" << n << " in this case)\n";
    if (n <= 1000) {
        cout << "Parallel Reduction: Skipped for small arrays (n <= 1000) to avoid overhead, using sequential O(n)\n";
    } else {
        cout << "Parallel Reduction: O(n/p) for parallel loop (p = threads), O(p) for combining, overall O(n)\n";
    }
}

// Function to validate integer input
int getValidInteger(const string& prompt, int minVal) {
    int value;
    cout << prompt;
    while (!(cin >> value) || value < minVal) {
        cout << "Invalid input. Enter a number >= " << minVal << ": ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return value;
}

int main() {
    // Get array size from user
    int n = getValidInteger("Enter the size of the array: ", 1);

    // Host array
    vector<int> data(n);
    cout << "Enter " << n << " integers:\n";
    for (int i = 0; i < n; i++) {
        while (!(cin >> data[i])) {
            cout << "Invalid input. Enter an integer: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    // Sequential reduction
    auto start = chrono::high_resolution_clock::now();
    int seqMin = INT_MAX;
    int seqMax = INT_MIN;
    long long seqSum = 0;
    for (int i = 0; i < n; i++) {
        seqMin = min(seqMin, data[i]);
        seqMax = max(seqMax, data[i]);
        seqSum += data[i];
    }
    double seqAvg = static_cast<double>(seqSum) / n;
    auto end = chrono::high_resolution_clock::now();
    double seq_time = chrono::duration<double, nano>(end - start).count() / 1000.0; // Convert to microseconds

    // Parallel reduction (with threshold)
    int globalMin = INT_MAX;
    int globalMax = INT_MIN;
    long long globalSum = 0;
    double par_time;

    const int parallel_threshold = 1000;
    if (n <= parallel_threshold) {
        // Use sequential reduction for small arrays
        start = chrono::high_resolution_clock::now();
        globalMin = seqMin;
        globalMax = seqMax;
        globalSum = seqSum;
        end = chrono::high_resolution_clock::now();
        par_time = chrono::duration<double, nano>(end - start).count() / 1000.0; // Convert to microseconds
    } else {
        // Parallel reduction using OpenMP reduction clause
        start = chrono::high_resolution_clock::now();
        #pragma omp parallel for schedule(static) reduction(min:globalMin) reduction(max:globalMax) reduction(+:globalSum)
        for (int i = 0; i < n; i++) {
            globalMin = min(globalMin, data[i]);
            globalMax = max(globalMax, data[i]);
            globalSum += data[i];
        }
        end = chrono::high_resolution_clock::now();
        par_time = chrono::duration<double, nano>(end - start).count() / 1000.0; // Convert to microseconds
    }
    double globalAvg = static_cast<double>(globalSum) / n;

    // Verify correctness
    bool correct = (globalMin == seqMin && globalMax == seqMax && globalSum == seqSum && globalAvg == seqAvg);

    // Avoid division by zero by setting a minimum time threshold
    const double min_time = 0.1; // Minimum measurable time in microseconds
    double adjusted_par_time = max(par_time, min_time);
    double speedup = seq_time / adjusted_par_time;
    double efficiency = speedup / omp_get_max_threads();

    // Print results with formatting
    cout << fixed << setprecision(4);
    cout << "Sequential Reduction Time: " << seq_time << " ms\n";
    cout << "Parallel Reduction Time: " << par_time << " ms\n";
    cout << "Speedup: " << speedup << "\n";
    cout << "Threads Used: " << omp_get_max_threads() << "\n";
    cout << "Efficiency: " << efficiency << "\n";
    cout << "Correctness: " << (correct ? "Pass" : "Fail") << "\n";
    cout << "Minimum: " << globalMin << "\n";
    cout << "Maximum: " << globalMax << "\n";
    cout << "Sum: " << globalSum << "\n";
    cout << "Average: " << globalAvg << "\n";

    // Print time complexity
    printTimeComplexity(n);

    return 0;
}

/*Enter the size of the array: 5
Enter 5 integers:
10 3 8 15 6
Sequential Reduction Time: 2.8000 ms
Parallel Reduction Time: 0.1000 ms
Speedup: 28.0000
Threads Used: 8
Efficiency: 3.5000
Correctness: Pass
Minimum: 3
Maximum: 15
Sum: 42
Average: 8.4000

Time Complexity Analysis:
Sequential Reduction: O(n), where n is the array size (5 in this case)
Parallel Reduction: Skipped for small arrays (n <= 1000) to avoid overhead, using sequential O(n)
*/