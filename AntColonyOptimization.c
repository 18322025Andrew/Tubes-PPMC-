// Reference by: https://learn.microsoft.com/en-us/archive/msdn-magazine/2012/february/test-run-ant-colony-optimization & https://github.com/arjun-krishna/TSP/blob/master/aco.cpp 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_LEN_STRING 255
#define M_PI 3.14159265358979323846

// Definisi struktur untuk node dalam linked list yang merepresentasikan sebuah kota
typedef struct Node {
    char nama_kota[MAX_LEN_STRING]; // Nama kota
    double lintang; 
    double bujur;
    struct Node* next;  // Pointer ke node berikutnya
} Node;

// Definisi struktur untuk semut dalam algoritma ACO
typedef struct Ant {
    int* tour; // Array yang merepresentasikan rute semut
    int tour_length; // Panjang rute
    int* visited; // Array untuk melacak kota yang sudah dikunjungi
    double tour_distance; // Total jarak rute
} Ant;

// Fungsi menambahkan node kota ke dalam linked list
int add(Node **head, double data_lintang, double data_bujur, char nama[]) {
    Node *temp = (Node*)malloc(sizeof(Node));
    if (temp == NULL) {
        printf("Memory allocation failed\n");
        return -1;
    }

    temp->bujur = data_bujur;
    temp->lintang = data_lintang;
    strcpy(temp->nama_kota, nama);
    temp->next = *head;
    *head = temp;

    return 0;
}

// Fungsi membaca data kota dari file dan membuat linked list kota
Node* input_file(char* file_name) {
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

// Fungsi menghitung jarak Haversine antara dua titik koordinat geografis
double haversine(double lat1, double lon1, double lat2, double lon2) {
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    double a = pow(sin(dLat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dLon / 2), 2);
    double rad = 6371;
    double c = 2 * asin(sqrt(a));

    return rad * c;
}

// Fungsi untuk menghitung jumlah kota dalam linked list
int calculate_cities(Node *daftar_kota) {
    if (daftar_kota == NULL) {
        return 0;
    }
    return 1 + calculate_cities(daftar_kota->next);
}

// Fungsi mengubah linked list kota menjadi array node
void array_node(Node *daftar_kota, Node *kota[], int jumlah_kota) {
    Node **temp = &daftar_kota;
    for (int i = 0; i < jumlah_kota; i++) {
        kota[i] = *temp;
        *temp = (*temp)->next;
    }
}

// Fungsi membuat matriks jarak menggunakan jarak Haversine antara setiap pasangan kota
void distance_matrices(Node *kota[], int jumlah_kota, double distances[jumlah_kota][jumlah_kota]) {
    for (int i = 0; i < jumlah_kota; i++) {
        for (int j = 0; j < jumlah_kota; j++) {
            distances[i][j] = (i == j) ? 0 : haversine(kota[i]->lintang, kota[i]->bujur, kota[j]->lintang, kota[j]->bujur);
        }
    }
}

// Fungsi menginisialisasi semut
void initialize_ant(Ant* ant, int jumlah_kota) {
    ant->tour = (int*)malloc(jumlah_kota * sizeof(int));
    ant->visited = (int*)malloc(jumlah_kota * sizeof(int));
    if (ant->tour == NULL || ant->visited == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < jumlah_kota; i++) {
        ant->visited[i] = 0;
        ant->tour[i] = -1;
    }
    ant->tour_length = 0;
    ant->tour_distance = 0.0;
}

// Fungsi membebaskan memori yang dialokasikan untuk semut
void free_ant(Ant* ant) {
    free(ant->tour);
    free(ant->visited);
}

// Fungsi memilih kota berikutnya yang akan dikunjungi oleh semut berdasarkan feromon dan jarak
int next_city(Ant* ant, int jumlah_kota, double pheromones[jumlah_kota][jumlah_kota], double distances[jumlah_kota][jumlah_kota], double alpha, double beta) {
    int current_city = ant->tour[ant->tour_length - 1];
    double total_prob = 0.0;
    double probabilities[jumlah_kota];

    for (int i = 0; i < jumlah_kota; i++) {
        if (!ant->visited[i]) {
            double pheromone = pow(pheromones[current_city][i], alpha);
            double visibility = 1.0 / distances[current_city][i];
            probabilities[i] = pheromone * pow(visibility, beta);
            total_prob += probabilities[i];
        } else {
            probabilities[i] = 0.0;
        }
    }

    if (total_prob == 0.0) {
        return -1;
    }

    double rand_val = (double)rand() / RAND_MAX;
    double cumulative_prob = 0.0;
    for (int i = 0; i < jumlah_kota; i++) {
        cumulative_prob += probabilities[i] / total_prob;
        if (!ant->visited[i] && cumulative_prob >= rand_val) {
            return i;
        }
    }

    return -1;
}

// Fungsi memperbarui rute semut dengan kota berikutnya
void update_ant_tour(Ant* ant, int kota) {
    ant->tour[ant->tour_length++] = kota;
    ant->visited[kota] = 1;
}

// Fungsi menghitung total jarak dari rute semut
void calculate_tour_distance(Ant* ant, int jumlah_kota, double distances[jumlah_kota][jumlah_kota]) {
    ant->tour_distance = 0.0;
    for (int i = 0; i < jumlah_kota - 1; i++) {
        int kota1 = ant->tour[i];
        int kota2 = ant->tour[i + 1];
        ant->tour_distance += distances[kota1][kota2];
    }
    int last_city = ant->tour[jumlah_kota - 1];
    int start_city = ant->tour[0];
    ant->tour_distance += distances[last_city][start_city];
}

// Fungsi mencetak rute terbaik yang ditemukan oleh semut
void display_best_route(Node *kota[], Ant* ant, int jumlah_kota) {
    printf("Best route found:\n");
    for (int i = 0; i < jumlah_kota; i++) {
        printf("%s", kota[ant->tour[i]]->nama_kota);
        if (i < jumlah_kota - 1) {
            printf(" -> ");
        }
    }
    printf(" -> %s", kota[ant->tour[0]]->nama_kota);
    printf("\nBest route distance: %1f km\n", ant->tour_distance);
}

// Fungsi utama eksekusi Ant Colony Optimization
void ant_colony_optimization(Node* kota[], int jumlah_kota, double distances[jumlah_kota][jumlah_kota], double pheromones[jumlah_kota][jumlah_kota], int Ants, double alpha, double beta, double evaporation_rate, int max_iterations, int startIndex) {
    Ant ants[Ants];
    for (int i = 0; i < Ants; i++) {
        initialize_ant(&ants[i], jumlah_kota);
    }

    for (int iter = 0; iter < max_iterations; iter++) {
        for (int i = 0; i < Ants; i++) {
            initialize_ant(&ants[i], jumlah_kota);
            update_ant_tour(&ants[i], startIndex);

            while (ants[i].tour_length < jumlah_kota) {
                int nextCity = next_city(&ants[i], jumlah_kota, pheromones, distances, alpha, beta);
                if (nextCity == -1) break;
                update_ant_tour(&ants[i], nextCity);
            }
            
            update_ant_tour(&ants[i], startIndex);
            calculate_tour_distance(&ants[i], jumlah_kota, distances);
        }
        // Evaporasi Feromon
        for (int i = 0; i < jumlah_kota; i++) {
            for (int j = 0; j < jumlah_kota; j++) {
                pheromones[i][j] *= (1 - evaporation_rate);
            }
        }
        // Memperbarui feromon berdasarkan tur semut
        for (int i = 0; i < jumlah_kota; i++) {
            for (int j = 0; j < jumlah_kota; j++) {
                int kota1 = ants[i].tour[j];
                int kota2 = ants[i].tour[(j + 1) % jumlah_kota];
                pheromones[kota1][kota2] += 1.0 / ants[i].tour_distance;
                pheromones[kota2][kota1] = pheromones[kota1][kota2];
            }
        }
    }
    // Menemukan rute terbaik secara keseluruhan
    Ant* best_ant = &ants[0];
    for (int i = 1; i < Ants; i++) {
        if (ants[i].tour_distance < best_ant->tour_distance) {
            best_ant = &ants[i];
        }
    }

    display_best_route(kota, best_ant, jumlah_kota);
    // Membebaskan alokasi memori 
    for (int i = 0; i < jumlah_kota; i++) {
        free_ant(&ants[i]);
    }
}

// Fungsi menentukan indeks kota berdasarkan nama kota
int city_index(Node* kota[], int jumlah_kota, const char* nama) {
    for (int i = 0; i < jumlah_kota; i++) {
        if (!strcmp(kota[i]->nama_kota, nama)) {
            return i;
        }
    }
    return -1;
}

// Fungsi Utama Program
int main() {
    static int run_counter = 0;

    while (1) {
        run_counter++;

        char file_name[MAX_LEN_STRING];
        printf("#%d\nEnter list of cities file name: ", run_counter);
        scanf("%s", file_name);
        Node *daftar_kota = input_file(file_name);
        if (daftar_kota == NULL) {
            printf("File tidak ditemukan !\n");
            continue;
        }

        int jumlah_kota = calculate_cities(daftar_kota);
        Node *kota[jumlah_kota];
        array_node(daftar_kota, kota, jumlah_kota);

        double distances[jumlah_kota][jumlah_kota];
        distance_matrices(kota, jumlah_kota, distances);

        double pheromones[jumlah_kota][jumlah_kota];
        for (int i = 0; i < jumlah_kota; i++) {
            for (int j = 0; j < jumlah_kota; j++) {
                pheromones[i][j] = 1.0;
            }
        }

        int Ants = jumlah_kota + 1;
        double alpha = 1.0;
        double beta = 2.0;
        double evaporation_rate = 0.5;
        int max_iterations = 100;

        char initial_city[MAX_LEN_STRING];
        printf("Enter the name of the initial city: ");
        getchar();  // mengambil karakter newline dari input sebelumnya
        fgets(initial_city, MAX_LEN_STRING, stdin);
        initial_city[strcspn(initial_city, "\n")] = '\0';

        int startIndex = city_index(kota, jumlah_kota, initial_city);
        if (startIndex == -1) {
            printf("City not found\n");
            return EXIT_FAILURE;
        }

        // Jalankan algoritma Ant Colony Optimization dan hitung waktu eksekusi
        clock_t start_time = clock();
        ant_colony_optimization(kota, jumlah_kota, distances, pheromones, Ants, alpha, beta, evaporation_rate, max_iterations, startIndex);
        clock_t end_time = clock();

        // Hitung waktu eksekusi
        double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;

        // Cetak waktu hasil eksekusi
        printf("Time elapsed: %.10f s\n", time_spent);

        Node* temp;
        while (daftar_kota != NULL) {
            temp = daftar_kota;
            daftar_kota = daftar_kota->next;
            free(temp);
        }

        char choice;
        printf("Do you want to input a new file? (y/n): ");
        scanf(" %c", &choice);

        if (choice == 'n' || choice == 'N') {
            break;
        }
    }

    return 0;
}
