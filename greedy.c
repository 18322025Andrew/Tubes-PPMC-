#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <time.h>

#define MAX_LEN_STRING 255
#define EARTH_RADIUS 6371.0
#define Phi 3.14159265358979323846

// Struct Node berisi variabel yang akan digunakan dalam program
typedef struct Node {
    char nama_kota[MAX_LEN_STRING];
    double bujur;
    double lintang;
    struct Node* next;
} Node;

//Fungsi untuk mengkonversi derajat ke radian
double toRadians(double degree) {
    return degree * Phi / 180.0;
}

// Fungsi untuk menambahkan node ke dalam linked list
int add(Node **head, double data_lintang, double data_bujur, char nama[]) {
    Node *temp = (Node*)malloc(sizeof(Node));
    temp->bujur = data_bujur;
    temp->lintang = data_lintang;
    strcpy(temp->nama_kota, nama);
    temp->next = *head;
    *head = temp;
    return 0;
}

// Fungsi untuk mencetak linked list
void print(Node* hasil) {
    Node* display = hasil;
    while (display != NULL) {
        printf("%s \t: %f\t|| %f\n", display->nama_kota, display->lintang, display->bujur);
        display = display->next;
    }
}

// Fungsi untuk membaca data dari file eksternal serta mengisinya ke dalam linked list
Node* input_file() {
    Node *Linked_list_kota = NULL;

    // Meminta user untuk memasukkan nama file map yang diinginkan
    char file_name[MAX_LEN_STRING];
    printf("-----Welcome to TSP Program using Greedy Algorithm-----\n");
    printf("Enter list of cities file name: ");
    scanf("%s", file_name);

    // Membuka file map yang dimasukkan user
    FILE* stream = fopen(file_name, "r");
    if (stream == NULL) { // Conditional statement jika file tidak ditemukan
        printf("File not found\n"); // Pesan kesalahan jika file tidak ditemukan
        return NULL;
    }

    // Conditional statement jika file ditemukan dan dapat dibuka
    char line[MAX_LEN_STRING];
    while (fgets(line, sizeof(line), stream)) { 
        // Menerima input berupa string yang lebih dari 1 kata (contoh: Banda Aceh)
        char kota_temp[MAX_LEN_STRING];
        double lintang, bujur;

        // Memeriksa letak bujur dan lintang dari masing-masing kota
        sscanf(line, "%[^,],%lf,%lf", kota_temp, &lintang, &bujur);
        add(&Linked_list_kota, lintang, bujur, kota_temp);
    }

    // Menutup file map
    fclose(stream);
    return Linked_list_kota;
}

// Fungsi untuk menghitung jarak antara dua titik dengan rumus haversine
double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = toRadians(lat2 - lat1);
    double dLon = toRadians(lon2 - lon1);
    lat1 = toRadians(lat1);
    lat2 = toRadians(lat2);

    double a = pow(sin(dLat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dLon / 2), 2);
    double c = 2 * asin(sqrt(a));
    return EARTH_RADIUS * c;
}

// Fungsi iteratif menggunakan Algoritma Greedy untuk mencari kota terdekat yang belum dikunjungi
int findNearestCity(Node cities[], int n, int visited[], int currentCity) {
    double minDistance = DBL_MAX;
    int nearestCity = -1;
    for (int i = 0; i < n; i++) {
        // Conditional statement pada kota yang belum dikunjungi
        if (!visited[i]) {
            double distance = haversine(cities[currentCity].lintang, cities[currentCity].bujur,
                                        cities[i].lintang, cities[i].bujur);
            if (distance < minDistance) {
                minDistance = distance;
                nearestCity = i;
            }
        }
    }
    return nearestCity; // Mengembalikkan kota terdekat
}

// Fungsi untuk menyelesaikan permasalahan TSP dengan Algoritma Greedy
double solveTSP(Node cities[], int n, int startCity, int *route) {
    int *visited = (int *)calloc(n, sizeof(int));
    double totalDistance = 0.0;

    int currentCity = startCity;
    visited[currentCity] = 1;
    route[0] = currentCity;

    for (int i = 1; i < n; i++) {
        int nextCity = findNearestCity(cities, n, visited, currentCity);
        if (nextCity == -1) break;
        totalDistance += haversine(cities[currentCity].lintang, cities[currentCity].bujur,
                                   cities[nextCity].lintang, cities[nextCity].bujur);
        currentCity = nextCity;
        visited[currentCity] = 1;
        route[i] = currentCity;
    }

    // Menambahkan jarak dari kota terakhir ke kota pertama yang akan membentuk sebuah siklus perjalanan
    totalDistance += haversine(cities[currentCity].lintang, cities[currentCity].bujur,
                               cities[startCity].lintang, cities[startCity].bujur);

    free(visited);
    return totalDistance;
}

// Fungsi untuk menentukan lokasi awal untuk memulai pencarian masalah TSP
void findBestStartingCity(Node cities[], int n, int startCityIndex, double *bestDistance, int *bestRoute) {
    *bestDistance = solveTSP(cities, n, startCityIndex, bestRoute);
}

// Fungsi utama
int main() {
    clock_t start = clock(); // Waktu saat program mulai berjalan
    Node *cities_list = input_file();
    if (cities_list == NULL) return 1;

    // Menghitung jumlah kota
    int n = 0;
    Node *temp = cities_list;
    while (temp != NULL) {
        n++;
        temp = temp->next;
    }

    // Menyalin data dari linked list ke dalam sebuah array
    Node *cities = (Node *)malloc(n * sizeof(Node)); // Mengakses memori
    temp = cities_list;
    for (int i = 0; i < n; i++) {
        strcpy(cities[i].nama_kota, temp->nama_kota);
        cities[i].lintang = temp->lintang;
        cities[i].bujur = temp->bujur;
        temp = temp->next;
    }

    // Menerima input nama kota awal dari user
    char startCityName[MAX_LEN_STRING];
    printf("Enter starting point: ");
    getchar(); // Untuk mengambil karakter newline yang tersisa dari scanf sebelumnya
    fgets(startCityName, MAX_LEN_STRING, stdin);
    startCityName[strcspn(startCityName, "\n")] = '\0'; // Menghapus karakter newline

    // Mencari indeks dari kota pertama
    int startCityIndex = -1;
    for (int i = 0; i < n; i++) {
        if (strcmp(cities[i].nama_kota, startCityName) == 0) {
            startCityIndex = i;
            break;
        }
    }

    // Conditional statement
    if (startCityIndex == -1) {
        printf("Starting city not found in the list.\n");
        free(cities);
        return 1;
    }

    double bestDistance;
    int *bestRoute = (int *)malloc(n * sizeof(int));

    // Menentukan jarak terbaik dari rute yang sudah ditempuh
    findBestStartingCity(cities, n, startCityIndex, &bestDistance, bestRoute);

    // Mencetak rute terbaik yang sudah ditemukan
    printf("Best route found:\n");
    for (int i = 0; i < n; i++) {
        printf("%s -> ", cities[bestRoute[i]].nama_kota);
    }
    printf("%s\n", cities[startCityIndex].nama_kota);
    printf("Best route distance: %.5f km\n", bestDistance);

    // Membebaskan memori
    free(cities);
    free(bestRoute);
    while (cities_list != NULL) {
        Node *temp = cities_list;
        cities_list = cities_list->next;
        free(temp);
    }

    clock_t end = clock(); // Waktu ketika program selesai berjalan
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC; // Menghitung waktu eksekusi program
    printf("Time spent: %.5f ms\n", time_spent); // Mencetak waktu eksekusi

    return 0;
}
