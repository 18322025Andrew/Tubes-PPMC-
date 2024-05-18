#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_LEN_STRING 255
#define M_PI 3.14159265358979323846

typedef struct Node {
    char nama_kota[MAX_LEN_STRING];
    double lintang;
    double bujur;
    struct Node* next;
} Node;

typedef struct Ant {
    int* tour;
    int tour_length;
    int* visited;
    double tour_distance;
} Ant;

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

Node* input_file(char* file_name) {
    Node *Linked_list_kota = NULL;

    FILE* stream = fopen(file_name, "r");
    if (stream == NULL) {
        printf("File tidak ditemukan, ");
        return NULL;
    }

    char line[MAX_LEN_STRING];
    char kota_temp[MAX_LEN_STRING];
    double lintang, bujur;

    while (fgets(line, sizeof(line), stream)) {
        char* token = strtok(line, ",");
        if (token != NULL) {
            strcpy(kota_temp, token);
            token = strtok(NULL, ",");
            if (token != NULL) {
                lintang = atof(token);
                token = strtok(NULL, ",");
                if (token != NULL) {
                    bujur = atof(token);
                    add(&Linked_list_kota, lintang, bujur, kota_temp);
                }
            }
        }
    }

    fclose(stream);
    return Linked_list_kota;
}

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

int calculate_cities(Node *daftar_kota) {
    int jumlah_kota = 0;
    Node *temp = daftar_kota;
    while (temp != NULL) {
        jumlah_kota++;
        temp = temp->next;
    }

    return jumlah_kota;
}

void make_cities_arrOfNode(Node *daftar_kota, Node *cities[], int jumlah_kota) {
    Node *temp = daftar_kota;
    for (int i = 0; i < jumlah_kota; i++) {
        cities[i] = temp;
        temp = temp->next;
    }
}

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

void initialize_ant(Ant* ant, int numCities) {
    ant->tour = (int*)malloc(numCities * sizeof(int));
    ant->visited = (int*)malloc(numCities * sizeof(int));
    if (ant->tour == NULL || ant->visited == NULL) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < numCities; i++) {
        ant->visited[i] = 0;
        ant->tour[i] = -1;
    }
    ant->tour_length = 0;
    ant->tour_distance = 0.0;
}

void free_ant(Ant* ant) {
    free(ant->tour);
    free(ant->visited);
}

int select_next_city(Ant* ant, int numCities, double pheromones[numCities][numCities], double distances[numCities][numCities], double alpha, double beta) {
    int current_city = ant->tour[ant->tour_length - 1];
    double total_prob = 0.0;
    double probabilities[numCities];

    for (int i = 0; i < numCities; i++) {
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
    for (int i = 0; i < numCities; i++) {
        cumulative_prob += probabilities[i] / total_prob;
        if (!ant->visited[i] && cumulative_prob >= rand_val) {
            return i;
        }
    }

    return -1;
}

void update_ant_tour(Ant* ant, int city) {
    ant->tour[ant->tour_length++] = city;
    ant->visited[city] = 1;
}

void calculate_tour_distance(Ant* ant, int numCities, double distances[numCities][numCities]) {
    ant->tour_distance = 0.0;
    for (int i = 0; i < numCities - 1; i++) {
        int city1 = ant->tour[i];
        int city2 = ant->tour[i + 1];
        ant->tour_distance += distances[city1][city2];
    }
    int last_city = ant->tour[numCities - 1];
    int start_city = ant->tour[0];
    ant->tour_distance += distances[last_city][start_city];
}

void print_bestRoute(Node *cities[], Ant* ant, int numCities) {
    printf("Best route found:\n");
    for (int i = 0; i < numCities; i++) {
        printf("%s", cities[ant->tour[i]]->nama_kota);
        if (i < numCities - 1) {
            printf(" -> ");
        }
    }
    printf(" -> %s", cities[ant->tour[0]]->nama_kota);
    printf("\nBest route distance: %lf km\n", ant->tour_distance);
}

void ant_colony_optimization(Node* cities[], int numCities, double distances[numCities][numCities], double pheromones[numCities][numCities], int numAnts, double alpha, double beta, double evaporation_rate, int max_iterations, int startIndex) {
    Ant ants[numAnts];
    for (int i = 0; i < numAnts; i++) {
        initialize_ant(&ants[i], numCities);
    }

    for (int iter = 0; iter < max_iterations; iter++) {
        for (int i = 0; i < numAnts; i++) {
            initialize_ant(&ants[i], numCities);
            update_ant_tour(&ants[i], startIndex);

            while (ants[i].tour_length < numCities) {
                int next_city = select_next_city(&ants[i], numCities, pheromones, distances, alpha, beta);
                if (next_city == -1) break;
                update_ant_tour(&ants[i], next_city);
            }
            
            update_ant_tour(&ants[i], startIndex);
            calculate_tour_distance(&ants[i], numCities, distances);
        }

        for (int i = 0; i < numCities; i++) {
            for (int j = 0; j < numCities; j++) {
                pheromones[i][j] *= (1 - evaporation_rate);
            }
        }

        for (int i = 0; i < numAnts; i++) {
            for (int j = 0; j < numCities; j++) {
                int city1 = ants[i].tour[j];
                int city2 = ants[i].tour[(j + 1) % numCities];
                pheromones[city1][city2] += 1.0 / ants[i].tour_distance;
                pheromones[city2][city1] = pheromones[city1][city2];
            }
        }
    }

    Ant* best_ant = &ants[0];
    for (int i = 1; i < numAnts; i++) {
        if (ants[i].tour_distance < best_ant->tour_distance) {
            best_ant = &ants[i];
        }
    }

    print_bestRoute(cities, best_ant, numCities);

    for (int i = 0; i < numAnts; i++) {
        free_ant(&ants[i]);
    }
}

int find_city_index(Node* cities[], int numCities, const char* city_name) {
    for (int i = 0; i < numCities; i++) {
        if (strcmp(cities[i]->nama_kota, city_name) == 0) {
            return i;
        }
    }
    return -1;
}

int main() {
    char file_name[MAX_LEN_STRING];
    printf("Masukkan File Map: ");
    scanf("%s", file_name);
    Node *daftar_kota = input_file(file_name);
    if (daftar_kota == NULL) {
        printf("Data Kosong !");
        return 0;
    }

    int jumlah_kota = calculate_cities(daftar_kota);
    Node *cities[jumlah_kota];
    make_cities_arrOfNode(daftar_kota, cities, jumlah_kota);

    double distances[jumlah_kota][jumlah_kota];
    make_distanceMatrices(cities, jumlah_kota, distances);

    double pheromones[jumlah_kota][jumlah_kota];
    for (int i = 0; i < jumlah_kota; i++) {
        for (int j = 0; j < jumlah_kota; j++) {
            pheromones[i][j] = 1.0;
        }
    }

    int numAnts = jumlah_kota+1;
    double alpha = 1.0;
    double beta = 2.0;
    double evaporation_rate = 0.5;
    int max_iterations = 100;

    char initial_city[MAX_LEN_STRING];
    printf("Enter the name of the initial city: ");
    getchar();  // mengambil karakter newline dari input sebelumnya
    fgets(initial_city, MAX_LEN_STRING, stdin);
    initial_city[strcspn(initial_city, "\n")] = '\0';

    int startIndex = find_city_index(cities, jumlah_kota, initial_city);
    if (startIndex == -1) {
        printf("City not found\n");
        return EXIT_FAILURE;
    }
    // Jalankan algoritma Ant Colony Optimization dan hitung waktu eksekusi
    clock_t start_time = clock();
    ant_colony_optimization(cities, jumlah_kota, distances, pheromones, numAnts, alpha, beta, evaporation_rate, max_iterations, startIndex);
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

    return 0;
}
