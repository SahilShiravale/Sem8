#include <iostream>
#include <omp.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <limits>
#include <chrono>

using namespace std;

// Sequential Bubble Sort for small arrays
void bubbleSortSequential(vector<int> &arr) {
    int n = arr.size();
    for(int i = 0; i < n-1; ++i) {
        bool swapped = false;
        for(int j = 0; j < n-i-1; ++j) {
            if(arr[j] > arr[j+1]) {
                swap(arr[j], arr[j+1]);
                swapped = true;
            }
        }
        if(!swapped) break;
    }
}

// Parallel Bubble Sort with early termination
void bubbleSortParallel(vector<int> &arr) {
    int n = arr.size();
    int threshold = max(100, 50 * omp_get_max_threads()); // Dynamic threshold
    if(n <= threshold) {
        bubbleSortSequential(arr);
        return;
    }
    bool swapped = true;
    #pragma omp parallel
    {
        for(int i = 0; i < n && swapped; ++i) {
            swapped = false;
            #pragma omp for reduction(||:swapped)
            for(int j = i % 2; j < n-1; j += 2) {
                if(arr[j] > arr[j+1]) {
                    swap(arr[j], arr[j+1]);
                    swapped = true;
                }
            }
        }
    }
}

// Function to validate integer input
int getValidInteger(const string& prompt, int minVal, int maxVal) {
    int value;
    cout << prompt;
    while(!(cin >> value) || value < minVal || value > maxVal) {
        cout << "Invalid input. Enter a number between " << minVal << " and " << maxVal << ": ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return value;
}

// Function to print time complexity
void printTimeComplexity(int size, int threshold) {
    cout << "\nTime Complexity Analysis:\n";
    cout << "- Array size: " << size << ", Threshold for sequential sort: " << threshold << "\n";
    if(size <= threshold) {
        cout << "- Using sequential bubble sort\n";
    } else {
        cout << "- Using parallel bubble sort\n";
    }
    cout << "- Best case: O(n) when array is already sorted (n = " << size << ")\n";
    cout << "- Average/Worst case: O(n²) (n = " << size << ")\n";
    cout << "- Note: Parallelism reduces runtime by a factor up to the number of threads, but asymptotic complexity remains the same.\n";
}

int main() {
    // Get array size from user
    int SIZE = getValidInteger("Enter the size of the array (1-1000000): ", 1, 1000000);
    vector<int> arr(SIZE);

    // Get input method (random or manual)
    char inputMethod;
    cout << "Generate random array (r) or manual input (m)? ";
    cin >> inputMethod;
    while(inputMethod != 'r' && inputMethod != 'R' && inputMethod != 'm' && inputMethod != 'M') {
        cout << "Invalid choice. Enter 'r' for random or 'm' for manual: ";
        cin >> inputMethod;
    }

    if(inputMethod == 'r' || inputMethod == 'R') {
        srand(time(0));
        for(int i = 0; i < SIZE; ++i) {
            arr[i] = rand() % 100000;
        }
    } else {
        cout << "Enter " << SIZE << " integers:\n";
        for(int i = 0; i < SIZE; ++i) {
            while(!(cin >> arr[i])) {
                cout << "Invalid input. Enter an integer: ";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
    }

    // Calculate threshold for time complexity reporting
    int threshold = max(100, 50 * omp_get_max_threads());

    // High-resolution timing
    auto start = chrono::high_resolution_clock::now();
    bubbleSortParallel(arr);
    auto end = chrono::high_resolution_clock::now();
    double time_par_bubble = chrono::duration<double>(end - start).count();

    cout << fixed << setprecision(6);
    cout << "Parallel Bubble Sort Time: " << time_par_bubble << " seconds\n";

    // Print sorted array for small inputs
    if(SIZE <= 10) {
        cout << "Sorted array: [";
        for(int i = 0; i < SIZE; ++i) {
            cout << arr[i];
            if(i < SIZE-1) cout << ", ";
        }
        cout << "]\n";
    }

    // Verify if array is sorted
    bool isSorted = true;
    for(int i = 0; i < SIZE-1; ++i) {
        if(arr[i] > arr[i+1]) {
            isSorted = false;
            break;
        }
    }
    cout << (isSorted ? "Array is sorted\n" : "Array is not sorted\n");

    // Print time complexity
    printTimeComplexity(SIZE, threshold);

    return 0;
}

/*Enter the size of the array (1-1000000): 5
Generate random array (r) or manual input (m)? m
Enter 5 integers:
5
2
9
1
6
Parallel Bubble Sort Time: 0.000003 seconds
Sorted array: [1, 2, 5, 6, 9]
Array is sorted

Time Complexity Analysis:
- Array size: 5, Threshold for sequential sort: 400
- Using sequential bubble sort
- Best case: O(n) when array is already sorted (n = 5)
- Average/Worst case: O(n┬▓) (n = 5)
- Note: Parallelism reduces runtime by a factor up to the number of threads, but asymptotic complexity remains the same.
*/