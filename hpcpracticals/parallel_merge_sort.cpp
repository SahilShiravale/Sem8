#include <iostream>
#include <omp.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <limits>
#include <chrono>

using namespace std;

// Sequential Merge Sort for small arrays
void mergeSortSequential(vector<int>& arr, int left, int right, vector<int>& temp) {
    if(left < right) {
        int mid = left + (right - left) / 2; // Avoid overflow
        mergeSortSequential(arr, left, mid, temp);
        mergeSortSequential(arr, mid + 1, right, temp);
        
        // Merge using temporary array
        int n1 = mid - left + 1;
        int n2 = right - mid;
        for(int i = 0; i < n1; ++i) temp[left + i] = arr[left + i];
        
        int i = 0, j = mid + 1, k = left;
        while(i < n1 && j <= right) {
            arr[k++] = (temp[left + i] <= arr[j]) ? temp[left + i++] : arr[j++];
        }
        while(i < n1) arr[k++] = temp[left + i++];
    }
}

// Parallel Merge Sort with tasks
void mergeSortParallel(vector<int>& arr, int left, int right, vector<int>& temp, int depth = 0) {
    const int threshold = 1000; // Threshold for sequential sort
    const int max_depth = 4;   // Limit task recursion depth
    
    if(right - left + 1 <= threshold || depth >= max_depth) {
        mergeSortSequential(arr, left, right, temp);
        return;
    }
    
    if(left < right) {
        int mid = left + (right - left) / 2;
        #pragma omp task if(depth < max_depth)
        mergeSortParallel(arr, left, mid, temp, depth + 1);
        #pragma omp task if(depth < max_depth)
        mergeSortParallel(arr, mid + 1, right, temp, depth + 1);
        #pragma omp taskwait
        
        // Merge using temporary array
        int n1 = mid - left + 1;
        int n2 = right - mid;
        for(int i = 0; i < n1; ++i) temp[left + i] = arr[left + i];
        
        int i = 0, j = mid + 1, k = left;
        while(i < n1 && j <= right) {
            arr[k++] = (temp[left + i] <= arr[j]) ? temp[left + i++] : arr[j++];
        }
 
        while(i < n1) arr[k++] = temp[left + i++];
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
void printTimeComplexity() {
    cout << "\nTime Complexity Analysis:\n";
    cout << "- Sequential Merge Sort: O(n log n) for all cases\n";
    cout << "- Parallel Merge Sort: O(n log n) total work, O((n log n)/p) wall-clock time\n";
    cout << "  where n is the array size and p is the number of processors\n";
}

int main() {
    // Get array size from user
    int SIZE = getValidInteger("Enter the size of the array (1-1000000): ", 1, 1000000);
    vector<int> arr(SIZE);
    vector<int> temp(SIZE); // Temporary array for merging

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

    // High-resolution timing
    auto start = chrono::high_resolution_clock::now();
    #pragma omp parallel
    {
        #pragma omp single
        mergeSortParallel(arr, 0, SIZE - 1, temp);
    }
    auto end = chrono::high_resolution_clock::now();
    double time_par_merge = chrono::duration<double>(end - start).count();

    cout << fixed << setprecision(6);
    cout << "Parallel Merge Sort Time: " << time_par_merge << " seconds\n";

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
    printTimeComplexity();

    return 0;
}

/*$ ./parallel_merge_sort.exe
Enter the size of the array (1-1000000): 5
Generate random array (r) or manual input (m)? m
Enter 5 integers:
5
2
9
1
6
Parallel Merge Sort Time: 0.002983 seconds
Sorted array: [1, 2, 5, 6, 9]
Array is sorted

Time Complexity Analysis:
- Sequential Merge Sort: O(n log n) for all cases
- Parallel Merge Sort: O(n log n) total work, O((n log n)/p) wall-clock time
  where n is the array size and p is the number of processors
*/