#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_LEN_STRING 255
#define M_PI 3.14159265358979323846

// Definisi struct Node untuk menyimpan data kota
typedef struct Node {
    char nama_kota[MAX_LEN_STRING];
    double lintang;
    double bujur;
    struct Node* next;
} Node;

// Fungsi untuk menambahkan node baru ke linked list
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

// Fungsi untuk membaca data kota dari file dan membentuk linked list
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

    // Membaca setiap baris dari file
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

// Fungsi untuk menghitung jarak Haversine antara dua titik (lintang dan bujur)
double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    double a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
    double rad = 6371; // Radius bumi dalam kilometer
    double c = 2 * asin(sqrt(a));

    return rad * c;
}

// Fungsi untuk menghitung jumlah kota
int calculate_cities(Node *daftar_kota){
    int jumlah_kota = 0;
    Node *temp = daftar_kota;
    while (temp != NULL) {
        jumlah_kota++;
        temp = temp->next;
    }

    return jumlah_kota;
}

// Fungsi untuk membuat array node kota dari linked list
void make_cities_arrOfNode(Node *daftar_kota, Node *cities[], int jumlah_kota) {
    Node *temp = daftar_kota;
    for (int i = 0; i < jumlah_kota; i++) {
        cities[i] = temp;
        temp = temp->next;
    }
}

// Fungsi untuk membuat matriks jarak antar kota
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


// Fungsi untuk menemukan indeks kota berdasarkan nama kota
int find_city_index(Node* cities[], int numCities, char* cityName) {
    for (int i = 0; i < numCities; i++) {
        if (strcmp(cities[i]->nama_kota, cityName) == 0) {
            return i;
        }
    }
    return -1;
}

// Fungsi rekursif DFS untuk menemukan jalur terdekat dalam masalah TSP
void search_best_route(Node* cities[], int numCities, int visited[], int currPos, int count, double cost, double* minCost, double distances[numCities][numCities], int path[], int bestPath[], int startIndex) {
    if (count == numCities) {
        double returnCost = distances[currPos][startIndex];
        if (returnCost > 0 && (cost + returnCost < *minCost)) {
            *minCost = cost + returnCost;
            for (int i = 0; i < numCities; i++) {
                bestPath[i] = path[i];
            }
            bestPath[numCities] = startIndex; // Kembali ke kota awal
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

// Fungsi untuk mencetak rute terbaik
void print_bestRoute(Node *cities[], int bestPath[], int jumlah_kota, double minCost) {
    printf("Best route found:\n");
    for (int i = 0; i <= jumlah_kota; i++) {
        printf("%s", cities[bestPath[i]]->nama_kota);
        if (i < jumlah_kota) {
            printf(" -> ");
        }
    }
    printf("\nBest route distance: %lf km\n", minCost);
}


// Fungsi utama
int main(void) {
    Node *daftar_kota = input_file();

    if (daftar_kota == NULL) {
        return -1;
    }

    // Menghitung jumlah kota
    int jumlah_kota = calculate_cities(daftar_kota);
    
    // Membuat array node kota
    Node *cities[jumlah_kota];
    make_cities_arrOfNode(daftar_kota, cities, jumlah_kota);

    // Membuat matriks jarak antar kota
    double distances[jumlah_kota][jumlah_kota];
    make_distanceMatrices(cities, jumlah_kota, distances);


    char startingCity[MAX_LEN_STRING];
    printf("Enter starting point: ");
    getchar();  // mengambil karakter newline dari input sebelumnya
    fgets(startingCity, MAX_LEN_STRING, stdin);
    startingCity[strcspn(startingCity, "\n")] = '\0';


    // Cari index dari kota yang akan dicari
    int startIndex = find_city_index(cities, jumlah_kota, startingCity);

    // Handling case jika kota start tidak ada
    if (startIndex == -1) {
        printf("Starting city not found\n");
        return 0;
    }

    // Jika ada, maka jalankan algoritma dfs dan hitung waktu eksekusi
    int *visited = (int *)malloc(jumlah_kota * sizeof(int));
    for (int i = 0; i < jumlah_kota; i++) {
        visited[i] = 0; // Inisiasi variabel pada masing-masing elemen array
    }
    visited[startIndex] = 1; // Mulai dari kota yang dipilih

    int path[jumlah_kota + 1];  // Untuk menyimpan jalur saat ini, karena balik ke bandung, maka ditambah 1
    int bestPath[jumlah_kota + 1];  // Untuk menyimpan jalur terbaik, sama
    path[0] = startIndex;

    // Mulai DFS untuk mencari biaya minimum
    double minCost = INFINITY;
    clock_t start_time = clock();
    search_best_route(cities, jumlah_kota, visited, startIndex, 1, 0, &minCost, distances, path, bestPath, startIndex);
    clock_t end_time = clock();

    // Hitung waktu eksekusi
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Cetak rute terbaik
    print_bestRoute(cities, bestPath, jumlah_kota, minCost);

    // Cetak waktu hasil eksekusi
    printf("Time elapsed: %.10f s\n", time_spent);

    return 0;
}
