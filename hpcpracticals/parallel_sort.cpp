#include <iostream>
#include <omp.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <algorithm>  // Added for min_element

using namespace std;

// Sequential Bubble Sort
void bubbleSortSequential(vector<int> &arr) {
    int n = arr.size();
    for(int i = 0; i < n-1; ++i)
        for(int j = 0; j < n-i-1; ++j)
            if(arr[j] > arr[j+1])
                swap(arr[j], arr[j+1]);
}

// Parallel Bubble Sort
void bubbleSortParallel(vector<int> &arr) {
    int n = arr.size();
    for(int i = 0; i < n; ++i) {
        #pragma omp parallel for
        for(int j = i % 2; j < n-1; j += 2) {
            if(arr[j] > arr[j+1])
                swap(arr[j], arr[j+1]);
        }
    }
}

// Merge utility
void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    vector<int> L(n1), R(n2);
    for(int i = 0; i < n1; ++i) L[i] = arr[left + i];
    for(int j = 0; j < n2; ++j) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while(i < n1 && j < n2)
        arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    while(i < n1) arr[k++] = L[i++];
    while(j < n2) arr[k++] = R[j++];
}

// Sequential Merge Sort
void mergeSortSequential(vector<int>& arr, int left, int right) {
    if(left < right) {
        int mid = (left + right) / 2;
        mergeSortSequential(arr, left, mid);
        mergeSortSequential(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

// Parallel Merge Sort
void mergeSortParallel(vector<int>& arr, int left, int right) {
    if(left < right) {
        int mid = (left + right) / 2;
        #pragma omp parallel sections
        {
            #pragma omp section
            mergeSortParallel(arr, left, mid);
            #pragma omp section
            mergeSortParallel(arr, mid + 1, right);
        }
        merge(arr, left, mid, right);
    }
}

int main() {
    const int SIZE = 10000;
    vector<int> arr1(SIZE), arr2(SIZE), arr3(SIZE), arr4(SIZE);

    srand(time(0));
    for(int i = 0; i < SIZE; ++i) {
        int val = rand() % 100000;
        arr1[i] = arr2[i] = arr3[i] = arr4[i] = val;
    }

    double t1, t2, time_seq_bubble, time_par_bubble, time_seq_merge, time_par_merge;

    t1 = omp_get_wtime();
    bubbleSortSequential(arr1);
    t2 = omp_get_wtime();
    time_seq_bubble = t2 - t1;
    cout << fixed << setprecision(6);
    cout << "Sequential Bubble Sort Time: " << time_seq_bubble << " seconds\n";

    t1 = omp_get_wtime();
    bubbleSortParallel(arr2);
    t2 = omp_get_wtime();
    time_par_bubble = t2 - t1;
    cout << "Parallel Bubble Sort Time:   " << time_par_bubble << " seconds\n";

    t1 = omp_get_wtime();
    mergeSortSequential(arr3, 0, SIZE - 1);
    t2 = omp_get_wtime();
    time_seq_merge = t2 - t1;
    cout << "Sequential Merge Sort Time: " << time_seq_merge << " seconds\n";

    t1 = omp_get_wtime();
    mergeSortParallel(arr4, 0, SIZE - 1);
    t2 = omp_get_wtime();
    time_par_merge = t2 - t1;
    cout << "Parallel Merge Sort Time:   " << time_par_merge << " seconds\n";

    cout << "\nEfficiency Summary:\n";

    if (time_seq_bubble < time_par_bubble)
        cout << "Sequential Bubble Sort is faster.\n";
    else
        cout << "Parallel Bubble Sort is faster.\n";

    if (time_seq_merge < time_par_merge)
        cout << "Sequential Merge Sort is faster.\n";
    else
        cout << "Parallel Merge Sort is faster.\n";

    cout << "\nBest Performer Overall: ";
    // Create a vector from the initializer list
    vector<double> times = {time_seq_bubble, time_par_bubble, time_seq_merge, time_par_merge};
    // Use min_element to find the minimum time
    double min_time = *min_element(times.begin(), times.end());
    
    if (min_time == time_seq_merge)
        cout << "Sequential Merge Sort \n";
    else if (min_time == time_par_merge)
        cout << "Parallel Merge Sort \n";
    else if (min_time == time_seq_bubble)
        cout << "Sequential Bubble Sort \n";
    else
        cout << "Parallel Bubble Sort \n";

    return 0;
}

/*$ ./parallel_sort.exe
Sequential Bubble Sort Time: 0.795000 seconds
Parallel Bubble Sort Time:   1.043000 seconds
Sequential Merge Sort Time: 0.007000 seconds
Parallel Merge Sort Time:   0.045000 seconds

Efficiency Summary:
Sequential Bubble Sort is faster.
Sequential Merge Sort is faster.

Best Performer Overall: Sequential Merge Sort
*/