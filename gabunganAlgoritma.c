#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <time.h>

#define MAX_LEN_STRING 255
#define M_PI 3.14159265358979323846
#define MAX_CITIES 20

typedef struct Node {
    char nama_kota[MAX_LEN_STRING];
    double lintang;
    double bujur;
    struct Node* next;
} Node;

typedef struct {
    int cityIndex;
    int count;
    double cost;
    bool visited[MAX_CITIES];
} StackItem;

typedef struct {
    int path[MAX_CITIES];
    double minJarak;
} Journey;

typedef struct {
    int *tour;
    int *visited;
    int tour_length;
    double tour_distance;
} Ant;

// Function to add a new node to the linked list
int add(Node **head, double data_lintang, double data_bujur, char nama[]) {
    Node *temp;
    temp = (Node*)malloc(sizeof(Node));
    temp->bujur = data_bujur;
    temp->lintang = data_lintang;
    strcpy(temp->nama_kota, nama);

    if (*head == NULL) {
        temp->next = NULL;
        *head = temp;
    } else {
        temp->next = *head;
        *head = temp;
    }
    return 0;
}

// Function to read city data from file and form a linked list
Node* input_file() {
    Node *Linked_list_kota = NULL;

    char file_name[MAX_LEN_STRING];
    printf("Enter list of cities file name: ");
    scanf("%s", file_name);

    FILE* stream = fopen(file_name, "r");

    if (stream == NULL) {
        printf("File tidak ditemukan\n");
        return NULL;
    }

    char line[MAX_LEN_STRING];
    char tempLine[MAX_LEN_STRING];
    char token_bujur[MAX_LEN_STRING];
    char token_lintang[MAX_LEN_STRING];
    char kota_temp[MAX_LEN_STRING];
    double lintang, bujur;

    while (fgets(line, MAX_LEN_STRING, stream)) {
        strcpy(tempLine, line);
        strcpy(kota_temp, strtok(tempLine, ","));
        strcpy(token_lintang, strtok(NULL, ","));
        strcpy(token_bujur, strtok(NULL, "\n"));
        sscanf(token_bujur, "%lf", &bujur);
        sscanf(token_lintang, "%lf", &lintang);
        add(&Linked_list_kota, lintang, bujur, kota_temp);
    }

    fclose(stream);
    return Linked_list_kota;
}

// Haversine distance calculation
double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
    double rad = 6371; // Earth's radius in kilometers
    double c = 2 * asin(sqrt(a));

    return rad * c;
}

// Function to count the number of cities
int calculate_cities(Node *daftar_kota){
    int jumlah_kota = 0;
    Node *temp = daftar_kota;
    while (temp != NULL) {
        jumlah_kota++;
        temp = temp->next;
    }

    return jumlah_kota;
}

// Function to create an array of city nodes from the linked list
void make_cities_arrOfNode(Node *daftar_kota, Node *cities[], int jumlah_kota) {
    Node *temp = daftar_kota;
    for (int i = 0; i < jumlah_kota; i++) {
        cities[i] = temp;
        temp = temp->next;
    }
}

// Function to create a distance matrix between cities
void make_distanceMatrices(Node *cities[], int jumlah_kota, double distances[jumlah_kota][jumlah_kota]) {
    for (int i = 0; i < jumlah_kota; i++) {
        for (int j = 0; j < jumlah_kota; j++) {
            if (i == j) {
                distances[i][j] = 0;
            } else {
                distances[i][j] = haversine(cities[i]->lintang, cities[i]->bujur, cities[j]->lintang, cities[j]->bujur);
            }
        }
    }
}

// Function to find the index of a city based on its name
int find_city_index(Node* cities[], int numCities, char* cityName) {
    for (int i = 0; i < numCities; i++) {
        if (strcmp(cities[i]->nama_kota, cityName) == 0) {
            return i;
        }
    }
    return -1;
}

// Recursive DFS function to find the shortest path in TSP
void search_best_route(Node* cities[], int numCities, int visited[], int currPos, int count, double cost, double* minCost, double distances[numCities][numCities], int path[], int bestPath[], int startIndex) {
    if (count == numCities) {
        double returnCost = distances[currPos][startIndex];
        if (returnCost > 0 && (cost + returnCost < *minCost)) {
            *minCost = cost + returnCost;
            for (int i = 0; i < numCities; i++) {
                bestPath[i] = path[i];
            }
            bestPath[numCities] = startIndex; // Return to start city
        }
        return;
    }

    for (int i = 0; i < numCities; i++) {
        if (!visited[i] && distances[currPos][i] > 0) {
            visited[i] = 1;
            path[count] = i;
            search_best_route(cities, numCities, visited, i, count + 1, cost + distances[currPos][i], minCost, distances, path, bestPath, startIndex);
            visited[i] = 0;
        }
    }
}

// BFS function
void bfs(Node *cities[], int numCities, int startCityIndex) {
    int queueSize = 1;
    int maxQueueSize = 1 << (numCities - 1); // 2^(numCities-1)
    Journey *queue = (Journey *)malloc(maxQueueSize * sizeof(Journey));
    Journey bestPath;
    bestPath.minJarak = DBL_MAX;

    for (int i = 0; i < numCities; ++i) {
        queue[0].path[i] = -1;
    }
    queue[0].path[0] = startCityIndex;
    queue[0].minJarak = 0;

    while (queueSize > 0) {
        Journey current = queue[--queueSize];

        int depth = 0;
        while (depth < numCities && current.path[depth] != -1) ++depth;

        if (depth == numCities) {
            double cost = 0;
            for (int i = 1; i < numCities; ++i) {
                cost += haversine(cities[current.path[i - 1]]->lintang, cities[current.path[i - 1]]->bujur, cities[current.path[i]]->lintang, cities[current.path[i]]->bujur);
            }
            cost += haversine(cities[current.path[numCities - 1]]->lintang, cities[current.path[numCities - 1]]->bujur, cities[startCityIndex]->lintang, cities[startCityIndex]->bujur);

            if (cost < bestPath.minJarak) {
                bestPath = current;
                bestPath.minJarak = cost;
            }
            continue;
        }

        for (int i = 0; i < numCities; ++i) {
            int found = 0;
            for (int j = 0; j < depth; ++j) {
                if (current.path[j] == i) {
                    found = 1;
                    break;
                }
            }
            if (found) continue;

            Journey newNode = current;
            newNode.path[depth] = i;
            newNode.minJarak = current.minJarak + haversine(cities[current.path[depth - 1]]->lintang, cities[current.path[depth - 1]]->bujur, cities[i]->lintang, cities[i]->bujur);
            queue[queueSize++] = newNode;
        }
    }

    printf("Best Route: ");
    for (int i = 0; i < numCities; ++i) {
        printf("%s -> ", cities[bestPath.path[i]]->nama_kota);
    }
    printf("%s\n", cities[bestPath.path[0]]->nama_kota);
    printf("Best route distance: %.5f km\n", bestPath.minJarak);

    free(queue);
}

// Brute Force
int vis[MAX_CITIES];
int bestTour[MAX_CITIES];
double bestCost = DBL_MAX;
int numCities;
double distance[MAX_CITIES][MAX_CITIES];

void brute_force_tsp(int level, double cost, int tour[]) {
    if (level == numCities) {
        if (cost + distance[tour[numCities - 1]][tour[0]] < bestCost) {
            bestCost = cost + distance[tour[numCities - 1]][tour[0]];
            memcpy(bestTour, tour, numCities * sizeof(int));
        }
        return;
    }
    for (int i = 0; i < numCities; i++) {
        if (!vis[i]) {
            vis[i] = 1;
            tour[level] = i;
            brute_force_tsp(level + 1, cost + distance[tour[level - 1]][i], tour);
            vis[i] = 0;
        }
    }
}

// Greedy Algorithm
void greedy_tsp(Node* cities[], int numCities, int startCityIndex) {
    bool visited[MAX_CITIES] = { false };
    int path[MAX_CITIES];
    double totalDistance = 0.0;
    int currentCity = startCityIndex;

    visited[currentCity] = true;
    path[0] = currentCity;

    for (int i = 1; i < numCities; i++) {
        int nearestCity = -1;
        double nearestDistance = DBL_MAX;

        for (int j = 0; j < numCities; j++) {
            if (!visited[j]) {
                double dist = haversine(cities[currentCity]->lintang, cities[currentCity]->bujur, cities[j]->lintang, cities[j]->bujur);
                if (dist < nearestDistance) {
                    nearestDistance = dist;
                    nearestCity = j;
                }
            }
        }

        path[i] = nearestCity;
        visited[nearestCity] = true;
        totalDistance += nearestDistance;
        currentCity = nearestCity;
    }

    totalDistance += haversine(cities[currentCity]->lintang, cities[currentCity]->bujur, cities[startCityIndex]->lintang, cities[startCityIndex]->bujur);

    printf("Greedy Algorithm Route: ");
    for (int i = 0; i < numCities; i++) {
        printf("%s -> ", cities[path[i]]->nama_kota);
    }
    printf("%s\n", cities[startCityIndex]->nama_kota);
    printf("Total Distance: %.5f km\n", totalDistance);
}

// Placeholder functions for ILP and ACO
void ilp_tsp(Node* cities[], int numCities) {
    printf("ILP TSP solution is not implemented yet.\n");
}

void aco_tsp(Node* cities[], int numCities) {
    printf("ACO TSP solution is not implemented yet.\n");
}

// Main Function
int main() {
    Node *daftar_kota = input_file();
    if (daftar_kota == NULL) {
        return 1;
    }

    int jumlah_kota = calculate_cities(daftar_kota);
    if (jumlah_kota > MAX_CITIES) {
        printf("Too many cities! Maximum number of cities is %d.\n", MAX_CITIES);
        return 1;
    }

    Node* cities[MAX_CITIES];
    make_cities_arrOfNode(daftar_kota, cities, jumlah_kota);

    double distances[MAX_CITIES][MAX_CITIES];
    make_distanceMatrices(cities, jumlah_kota, distances);

    char startCity[MAX_LEN_STRING];
    printf("Enter the starting city: ");
    scanf("%s", startCity);

    int startCityIndex = find_city_index(cities, jumlah_kota, startCity);
    if (startCityIndex == -1) {
        printf("Starting city not found in the list.\n");
        return 1;
    }

    printf("\nChoose the algorithm to solve TSP:\n");
    printf("1. Greedy Algorithm\n");
    printf("2. DFS\n");
    printf("3. BFS\n");
    printf("4. Brute Force\n");
    printf("5. ILP\n");
    printf("6. ACO\n");
    printf("Enter your choice: ");
    int choice;
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            greedy_tsp(cities, jumlah_kota, startCityIndex);
            break;
        case 2: {
            int visited[MAX_CITIES] = {0};
            int path[MAX_CITIES];
            int bestPath[MAX_CITIES];
            double minCost = DBL_MAX;
            path[0] = startCityIndex;
            visited[startCityIndex] = 1;
            search_best_route(cities, jumlah_kota, visited, startCityIndex, 1, 0, &minCost, distances, path, bestPath, startCityIndex);
            printf("DFS Algorithm Route: ");
            for (int i = 0; i < jumlah_kota; i++) {
                printf("%s -> ", cities[bestPath[i]]->nama_kota);
            }
            printf("%s\n", cities[bestPath[0]]->nama_kota);
            printf("Total Distance: %.5f km\n", minCost);
            break;
        }
        case 3:
            bfs(cities, jumlah_kota, startCityIndex);
            break;
        case 4: {
            int tour[MAX_CITIES];
            for (int i = 0; i < MAX_CITIES; i++) vis[i] = 0;
            vis[startCityIndex] = 1;
            tour[0] = startCityIndex;
            brute_force_tsp(1, 0, tour);
            printf("Brute Force Algorithm Route: ");
            for (int i = 0; i < jumlah_kota; i++) {
                printf("%s -> ", cities[bestTour[i]]->nama_kota);
            }
            printf("%s\n", cities[bestTour[0]]->nama_kota);
            printf("Total Distance: %.5f km\n", bestCost);
            break;
        }
        case 5:
            ilp_tsp(cities, jumlah_kota);
            break;
        case 6:
            aco_tsp(cities, jumlah_kota);
            break;
        default:
            printf("Invalid choice.\n");
            break;
    }

    return 0;
}
