// Header yang digunakan
// Library 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <time.h> // Berguna untuk mengukur waktu eksekusi program

// Variabel global 
#define MAX_LEN 255
#define radius_earth 6371
#define MAX_CITIES 520

// Data struct yang digunakan
typedef struct Node {
    char nama_kota[MAX_LEN];
    double lintang;
    double bujur;
    struct Node* next;
} Node;

// Data struct untuk mengetahui
typedef struct {
    int path[MAX_CITIES];
    double minJarak;
} Journey;

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

    char file_name[MAX_LEN];
    printf("Enter list of cities file name: ");
    fgets("%s", file_name);

    FILE* stream = fopen(file_name, "r");
    if (stream == NULL) {
        printf("File tidak ditemukan\n");
        return NULL;
    }

    char line[MAX_LEN];
    while (fgets(line, MAX_LEN, stream)) {
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

// Fungsi untuk menghitung jarak total
double calculateTotalDistance(int *path, Node *cities[], int numCities) {
    double totalDistance = 0;
    for (int i = 0; i < numCities; ++i) {
        totalDistance += haversine(cities[path[i]]->lintang, cities[path[i]]->bujur,
                                   cities[path[(i + 1) % numCities]]->lintang, cities[path[(i + 1) % numCities]]->bujur);
    }
    return totalDistance;
}

// Fungsi untuk mencari kota dengan index
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
            double cost = calculateTotalDistance(current.path, cities, numCities);
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

// Menampilkan data
void print(Node* hasil) {
    Node* display = hasil;
    while (display != NULL) {
        printf("%s \t: %f\t|| %f\n", display->nama_kota, display->lintang, display->bujur);
        display = display->next;
    }
}

// Fungsi untuk menampilkan kota
void print(Node* hasil) {
    Node* display = hasil;
    while (display != NULL) {
        printf("%s \t: %f\t|| %f\n", display->nama_kota, display->lintang, display->bujur);
        display = display->next;
    }
}

// Fungsi utama
int main (){
    // Menerima input berupa data csv
    Node* linkedListKota = input_file();
    if (linkedListKota == NULL){
        return 1; // Hasil tidak sesuai
    }

    // Mengubah bentuk linked list menjadi sebuah array
    Node* city[MAX_CITIES];
    int numCities = 0;
    Node* numCitiesNow = linkedListKota;
    while (numCitiesNow != NULL && numCities < MAX_CITIES){
        city[numCities++] = numCitiesNow;
        numCitiesNow = numCitiesNow->next; 
    }

    char departure[MAX_LEN];
    printf("Enter starting point: ");
    getchar();
    fgets(departure, MAX_LEN, stdin);
    departure[strcspn(departure, "\n")] = '\0';

    int cityIndex = findCityIndex(city, numCities, departure);
    if (departure == -1){
        printf ("Kota tidak dapat ditemukan");
        return 1; // Hasil tidak sesuai 
    }


    // Menguji dan melihat waktu yang dibutuhkan program untuk menyelesaikan masalah
    // Memulai pengukuran waktu
    // clock_t start_time = clock();

    // Panggil fungsi BFS
    bfs(city, numCities, departure);

    // Akhiri pengukuran waktu
    // clock_t end_time = clock();
    // double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Tampilkan waktu eksekusi
    // printf("Waktu eksekusi BFS: %f detik\n", time_taken);

    return 0;
}
