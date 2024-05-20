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
int permutation[MAX_CITIES];
int best_route[MAX_CITIES];
double min_cost;
double matriks[MAX_CITIES][MAX_CITIES];

void brute_force(Node* cities[], int jumlah_kota, int posisi) {
    if (posisi == jumlah_kota) {
        double cost = 0;
        for (int i = 0; i < jumlah_kota; i++) {
            if (i == 0) {
                cost += matriks[jumlah_kota][permutation[i]];
            } else {
                cost += matriks[permutation[i-1]][permutation[i]];
            }
        }
        cost += matriks[permutation[jumlah_kota-1]][jumlah_kota];

        if (cost < min_cost) {
            min_cost = cost;
            for (int i = 0; i < jumlah_kota; i++) {
                best_route[i] = permutation[i];
            }
        }
    } else {
        for (int i = 0; i < jumlah_kota; i++) {
            if (!vis[i]) {
                vis[i] = 1;
                permutation[posisi] = i;
                brute_force(cities, jumlah_kota, posisi + 1);
                vis[i] = 0;
            }
        }
    }
}

void solve_brute_force(Node* cities[], int jumlah_kota) {
    min_cost = DBL_MAX;
    memset(vis, 0, sizeof(vis));
    brute_force(cities, jumlah_kota, 0);

    printf("Best Route: ");
    for (int i = 0; i < jumlah_kota; i++) {
        printf("%s -> ", cities[best_route[i]]->nama_kota);
    }
    printf("%s\n", cities[0]->nama_kota);
    printf("Best route distance: %.5f km\n", min_cost);
}

// ILP 
void ilp(Node* cities[], int jumlah_kota) {
    // Placeholder for ILP TSP solver
    printf("ILP TSP solver not implemented.\n");
}

// Main function to execute the selected algorithm
int main() {
    Node *daftar_kota = input_file();
    if (!daftar_kota) {
        return 1;
    }

    int jumlah_kota = calculate_cities(daftar_kota);
    Node *cities[jumlah_kota];
    make_cities_arrOfNode(daftar_kota, cities, jumlah_kota);

    double distances[jumlah_kota][jumlah_kota];
    make_distanceMatrices(cities, jumlah_kota, distances);

    char starting_city[MAX_LEN_STRING];
    printf("Enter the starting city: ");
    scanf("%s", starting_city);

    int startIndex = find_city_index(cities, jumlah_kota, starting_city);
    if (startIndex == -1) {
        printf("Starting city not found.\n");
        return 1;
    }

    int choice;
    printf("Choose TSP solving method:\n");
    printf("1. DFS\n");
    printf("2. BFS\n");
    printf("3. Brute Force\n");
    printf("4. ILP\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice == 1) {
        int visited[jumlah_kota];
        int path[jumlah_kota + 1]; // Include start city at the end
        int bestPath[jumlah_kota + 1];
        double minCost = DBL_MAX;

        memset(visited, 0, sizeof(visited));
        visited[startIndex] = 1;
        path[0] = startIndex;
        search_best_route(cities, jumlah_kota, visited, startIndex, 1, 0, &minCost, distances, path, bestPath, startIndex);

        printf("Best Route: ");
        for (int i = 0; i < jumlah_kota; i++) {
            printf("%s -> ", cities[bestPath[i]]->nama_kota);
        }
        printf("%s\n", cities[bestPath[0]]->nama_kota);
        printf("Best route distance: %.5f km\n", minCost);
    } else if (choice == 2) {
        bfs(cities, jumlah_kota, startIndex);
    } else if (choice == 3) {
        solve_brute_force(cities, jumlah_kota);
    } else if (choice == 4) {
        ilp(cities, jumlah_kota);
    } else {
        printf("Invalid choice.\n");
    }

    return 0;
}
