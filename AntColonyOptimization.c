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

// Definisi struct Ant untuk mewakili semut
typedef struct Ant {
    int* tour;           // Jalur yang ditempuh oleh semut
    int tour_length;     // Panjang jalur
    int* visited;        // Kota yang sudah dikunjungi
    double tour_distance; // Jarak total jalur
} Ant;

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
Node* input_file(char file_name[]) {
    Node *Linked_list_kota = NULL;

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

// Fungsi untuk menginisialisasi semut
void initialize_ant(Ant* ant, int numCities) {
    ant->tour = (int*)malloc(numCities * sizeof(int));
    ant->visited = (int*)malloc(numCities * sizeof(int));
    for (int i = 0; i < numCities; i++) {
        ant->visited[i] = 0;
        ant->tour[i] = -1;
    }
    ant->tour_length = 0;
    ant->tour_distance = 0.0;
}

// Fungsi untuk membebaskan memori yang dialokasikan untuk semut
void free_ant(Ant* ant) {
    free(ant->tour);
    free(ant->visited);
}

// Fungsi untuk memilih kota selanjutnya yang akan dikunjungi oleh semut
int select_next_city(Ant* ant, int numCities, double pheromones[numCities][numCities], double distances[numCities][numCities], double alpha, double beta) {
    int current_city = ant->tour[ant->tour_length - 1];
    double total_prob = 0.0;
    double probabilities[numCities];
    for (int i = 0; i < numCities; i++) {
        if (!ant->visited[i]) {
            double pheromone = pow(pheromones[current_city][i], alpha);
            double visibility = 1.0 / distances[current_city][i];
            double prob = pheromone * visibility;
            probabilities[i] = prob;
            total_prob += prob;
        } else {
            probabilities[i] = 0.0;
        }
    }

    // Roulette wheel selection
    double rand_val = (double)rand() / RAND_MAX;
    double cumulative_prob = 0.0;
    for (int i = 0; i < numCities; i++) {
        cumulative_prob += probabilities[i] / total_prob;
        if (!ant->visited[i] && cumulative_prob >= rand_val) {
            return i;
        }
    }
    return -1; // If no city is selected
}

// Fungsi untuk memperbarui jejak semut
void update_ant_tour(Ant* ant, int city) {
    ant->tour[ant->tour_length++] = city;
    ant->visited[city] = 1;
}

// Fungsi untuk menghitung jarak total dari jalur semut
void calculate_tour_distance(Ant* ant, int numCities, double distances[numCities][numCities]) {
    ant->tour_distance = 0.0;
    for (int i = 0; i < numCities - 1; i++) {
        int city1 = ant->tour[i];
        int city2 = ant->tour[i + 1];
        ant->tour_distance += distances[city1][city2];
    }
}

// Fungsi untuk mencetak rute terbaik
void print_bestRoute(Node *cities[], Ant* ant, int numCities) {
    printf("Best route found:\n");
    for (int i = 0; i < numCities; i++) {
        printf("%s", cities[ant->tour[i]]->nama_kota);
        if (i < numCities - 1) {
            printf(" -> ");
        }
    }
    printf("\nBest route distance: %lf km\n", ant->tour_distance);
}

// Algoritma Ant Colony Optimization untuk mencari rute terbaik
void ant_colony_optimization(Node* cities[], int numCities, double distances[numCities][numCities], double pheromones[numCities][numCities], int numAnts, double alpha, double beta, double evaporation_rate, int max_iterations, int startIndex) {
    // Inisialisasi semut
    Ant ants[numAnts];
    for (int i = 0; i < numAnts; i++) {
        initialize_ant(&ants[i], numCities);
    }

    // Iterasi ACO
    for (int iter = 0; iter < max_iterations; iter++) {
        // Perbarui jejak setiap semut
        for (int i = 0; i < numAnts; i++) {
            // Mulai dari kota awal yang dimasukkan pengguna
            ants[i].tour[0] = startIndex;
            ants[i].visited[startIndex] = 1;
            ants[i].tour_length = 1;

            // Lanjutkan perjalanan semut
            while (ants[i].tour_length < numCities) {
                int next_city = select_next_city(&ants[i], numCities, pheromones, distances, alpha, beta);
                if (next_city == -1) {
                    break;
                }
                update_ant_tour(&ants[i], next_city);
            }

            // Kembali ke kota awal
            ants[i].tour[numCities - 1] = ants[i].tour[0];

            // Hitung jarak total tour
            calculate_tour_distance(&ants[i], numCities, distances);
        }

        // Update pheromone trail
        for (int i = 0; i < numCities; i++) {
            for (int j = 0; j < numCities; j++) {
                pheromones[i][j] *= (1 - evaporation_rate); // Evaporate pheromone
            }
        }

        // Deposit pheromone based on ant tours
        for (int i = 0; i < numAnts; i++) {
            for (int j = 0; j < numCities - 1; j++) {
                int city1 = ants[i].tour[j];
                int city2 = ants[i].tour[j + 1];
                pheromones[city1][city2] += (1 / ants[i].tour_distance);
                pheromones[city2][city1] += (1 / ants[i].tour_distance); // Pheromones are symmetric
            }
        }
    }

    // Cari semut dengan tour terpendek
    int best_ant_index = 0;
    for (int i = 1; i < numAnts; i++) {
        if (ants[i].tour_distance < ants[best_ant_index].tour_distance) {
            best_ant_index = i;
        }
    }

    // Cetak rute terbaik
    print_bestRoute(cities, &ants[best_ant_index], numCities);

    // Membebaskan memori yang dialokasikan untuk semut
    for (int i = 0; i < numAnts; i++) {
        free_ant(&ants[i]);
    }
}

// Fungsi utama
int main(void) {
    Node *daftar_kota;
    char file_name[MAX_LEN_STRING];
    printf("Masukkan File Map: ");
    scanf("%s", file_name);

    daftar_kota = input_file(file_name);

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

    // Inisialisasi matriks pheromone
    double pheromones[jumlah_kota][jumlah_kota];
    for (int i = 0; i < jumlah_kota; i++) {
        for (int j = 0; j < jumlah_kota; j++) {
            pheromones[i][j] = 0.1; // Initial pheromone level
        }
    }

    // Set parameters for ACO
    int numAnts = 10;
    double alpha = 1.0; // Pheromone factor
    double beta = 2.0; // Visibility factor
    double evaporation_rate = 0.5; // Evaporation rate
    int max_iterations = 100; // Maximum iterations

    char startingCity[MAX_LEN_STRING];
    printf("Enter starting point: ");
    getchar();  // mengambil karakter newline dari input sebelumnya
    fgets(startingCity, MAX_LEN_STRING, stdin);
    startingCity[strcspn(startingCity, "\n")] = '\0';

    // Handling case jika kota start tidak ada
    int startIndex = -1;
    for (int i = 0; i < jumlah_kota; i++) {
        if (strcmp(cities[i]->nama_kota, startingCity) == 0) {
            startIndex = i;
            break;
        }
    }

    if (startIndex == -1) {
        printf("Starting city not found\n");
        return 0;
    }

    // Jalankan algoritma Ant Colony Optimization dan hitung waktu eksekusi
    clock_t start_time = clock();
    ant_colony_optimization(cities, jumlah_kota, distances, pheromones, numAnts, alpha, beta, evaporation_rate, max_iterations, startIndex);
    clock_t end_time = clock();

    // Hitung waktu eksekusi
    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Cetak waktu hasil eksekusi
    printf("Time elapsed: %.10f s\n", time_spent);

    return 0;
}
