#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h>

// Define city data structure
typedef struct {
    char name[50];
    double latitude;
    double longitude;
} City;

// Haversine formula for calculating distance between two points
double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
    double rad = 6371;
    double c = 2 * asin(sqrt(a));
    return rad * c;
}

// Calculate distances between all pairs of cities
void calculateDistances(const City cities[4], double distances[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i != j) {
                distances[i][j] = haversine(cities[i].latitude, cities[i].longitude,
                                            cities[j].latitude, cities[j].longitude);
            } else {
                distances[i][j] = 0.0; // Set distance to itself as 0
            }
        }
    }
}

// Swap function to be used for generating permutations
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Function to generate the next permutation
int next_permutation(int *start, int *end) {
    if (start == end) return 0;
    int *i = end - 1;
    while (i > start && *(i - 1) >= *i) --i;
    if (i == start) return 0;
    int *j = end - 1;
    while (*j <= *(i - 1)) --j;
    swap(i - 1, j);
    for (++i, --end; i < end; ++i, --end) swap(i, end);
    return 1;
}

// Brute-force approach to find the shortest TSP path
void findShortestTSPPath(const City cities[4], double distances[4][4], double *shortestDistance, int bestPermutation[4]) {
    int permutation[4] = {0, 1, 2, 3};
    *shortestDistance = DBL_MAX;

    do {
        double totalDistance = 0.0;
        for (int i = 0; i < 3; i++) {
            int currentCity = permutation[i];
            int nextCity = permutation[i + 1];
            totalDistance += distances[currentCity][nextCity];
        }
        // Optionally, add the distance from the last city back to the first to complete the loop
        totalDistance += distances[permutation[3]][permutation[0]];

        if (totalDistance < *shortestDistance) {
            *shortestDistance = totalDistance;
            memcpy(bestPermutation, permutation, 4 * sizeof(int));
        }
    } while (next_permutation(permutation, permutation + 4));
}

int main() {
    City cities[4];
    for (int i = 0; i < 4; ++i) {
        printf("Enter the name of city %d: ", i + 1);
        scanf("%s", cities[i].name);
        printf("Enter the latitude of %s: ", cities[i].name);
        scanf("%lf", &cities[i].latitude);
        printf("Enter the longitude of %s: ", cities[i].name);
        scanf("%lf", &cities[i].longitude);
    }

    // Calculate distances between all cities
    double distances[4][4];
    calculateDistances(cities, distances);

    // Find the shortest TSP path
    double shortestDistance;
    int bestPermutation[4];
    findShortestTSPPath(cities, distances, &shortestDistance, bestPermutation);

    // Print the shortest distance
    printf("Shortest TSP path distance: %.2f km\n", shortestDistance);

    // Print the order of cities in the shortest path
    printf("Order of cities in the shortest path: ");
    for (int i = 0; i < 4; ++i) {
        printf("%s -> ", cities[bestPermutation[i]].name);
    }
    // Print the first city again to show the loop
    printf("%s\n", cities[bestPermutation[0]].name);

    return 0;
}
