// Library yang digunakan
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

// Inisiasi awal variabel
#define Max_length 100 // Jumlah kata kota
#define radius_earth 6371 // Jari-jari bumi
#define maxCities 520 // Banyak kota yang dapat ditampung dilihat dari data list-coordinates-indonesian.csv

// Data struct yang digunakan
typedef struct Node {
    char nama_kota[Max_length];
    double lintang;
    double bujur;
    struct Node* next;
} Node;

// Data struct untuk mengetahui
typedef struct {
    int path[maxCities];
    double cost;
} pathNode;

// Fungsi untuk menambah node
int add(Node **head, double data_lintang, double data_bujur, char nama[]) {
    Node *temp = (Node*)malloc(sizeof(Node));
    if (!temp) {
        return 1; // Failed to allocate memory
    }

    temp->bujur = data_bujur;
    temp->lintang = data_lintang;
    strcpy(temp->nama_kota, nama);
    temp->next = *head;
    *head = temp;

    return 0;
}

// Fungsi untuk menerima input data csv
Node* input_file() {
    Node *Linked_list_kota = NULL;

    char file_name[Max_length];
    printf("Enter list of cities file name: ");
    scanf("%s", file_name);

    FILE* stream = fopen(file_name, "r");
    if (stream == NULL) {
        printf("File tidak ditemukan\n");
        return NULL;
    }

    char line[Max_length];
    while (fgets(line, Max_length, stream)) {
        char *kota_temp = strtok(line, ",");
        char *token_lintang = strtok(NULL, ",");
        char *token_bujur = strtok(NULL, "\n");

        double lintang, bujur;
        sscanf(token_lintang, "%lf", &lintang);
        sscanf(token_bujur, "%lf", &bujur);

        add(&Linked_list_kota, lintang, bujur, kota_temp);
    }

    fclose(stream);
    return Linked_list_kota;
}

// Fungsi untuk menghitung jarak antar kedua titik koordinat
double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    double a = sin(dLat / 2) * sin(dLat / 2) + cos(lat1) * cos(lat2) * sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return radius_earth * c;
}

// Fungsi untuk menghitung jarak total antara
double calculateTotalDistance(int *path, Node *cities[], int numCities) {
    double totalDistance = 0;
    for (int i = 0; i < numCities; ++i) {
        totalDistance += haversine(cities[path[i]]->lintang, cities[path[i]]->bujur,
                                   cities[path[(i + 1) % numCities]]->lintang, cities[path[(i + 1) % numCities]]->bujur);
    }
    return totalDistance;
}

// Fungsi untuk menentukan urutan
int findCityIndex(Node *cities[], int numCities, char *cityName) {
    for (int i = 0; i < numCities; ++i) {
        if (strcmp(cities[i]->nama_kota, cityName) == 0) {
            return i;
        }
    }
    return -1;
}

// Fungsi untuk menentukan jarak terpendek dari kota-kota yang ada
void bfs(Node *cities[], int numCities, int startCityIndex) {
    int queueSize = 1;
    int maxQueueSize = 1 << (numCities - 1); // 2^(numCities-1)
    pathNode *queue = (pathNode *)malloc(maxQueueSize * sizeof(pathNode));
    pathNode bestPath;
    bestPath.cost = DBL_MAX;

    for (int i = 0; i < numCities; ++i) {
        queue[0].path[i] = -1;
    }
    queue[0].path[0] = startCityIndex;
    queue[0].cost = 0;

    while (queueSize > 0) {
        pathNode current = queue[--queueSize];

        int depth = 0;
        while (depth < numCities && current.path[depth] != -1) ++depth;

        if (depth == numCities) {
            double cost = calculateTotalDistance(current.path, cities, numCities);
            if (cost < bestPath.cost) {
                bestPath = current;
                bestPath.cost = cost;
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

            pathNode newNode = current;
            newNode.path[depth] = i;
            newNode.cost = current.cost + haversine(cities[current.path[depth - 1]]->lintang, cities[current.path[depth - 1]]->bujur, cities[i]->lintang, cities[i]->bujur);
            queue[queueSize++] = newNode;
        }
    }

    printf("Best Route: ");
    for (int i = 0; i < numCities; ++i) {
        printf("%s -> ", cities[bestPath.path[i]]->nama_kota);
    }
    printf("%s\n", cities[bestPath.path[0]]->nama_kota);
    printf("Best route distance: %.5f km\n", bestPath.cost);

    free(queue);
}

// Menampilkan data
void print(Node* hasil) {
    Node* display = hasil;
    while (display != NULL) {
        printf("%s \t: %f\t|| %f\n", display->nama_kota, display->lintang, display->bujur);
        display = display->next;
    }
}

// Fungsi utama
int main() {
    // Menerima input berupa data csv
    Node* linkedListKota = input_file();
    if (linkedListKota == NULL) {
        return 1;
    }

    Node* cities[maxCities];
    int numCities = 0;
    Node* current = linkedListKota;
    while (current != NULL && numCities < maxCities) {
        cities[numCities++] = current;
        current = current->next;
    }

    char startCity[Max_length];
    printf("Enter starting point: ");
    getchar();
    fgets(startCity, Max_length, stdin);
    startCity[strcspn(startCity, "\n")] = '\0';

    int startCityIndex = findCityIndex(cities, numCities, startCity);
    if (startCityIndex == -1) {
        printf("Kota tidak dapat ditemukan, silahkan coba lagi.\n");
        return 1;
    }

    bfs(cities, numCities, startCityIndex);

    return 0;
}
