#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define ALPHA 1         // Pheromone trail importance factor
#define pi 3.14159265358979323846
#define r 6371
#define MAX_CHAR 255
#define MAX_CITY 15

int size;
int vis[15], permutation[15], n;
int best_route[15];
float min_cost = 1e9;
char namaKota[15][255];
float matriks[15][15];

int open_init(char *namaFile, float matriks[15][15], char namaKota[15][255], int *n){
    *n = 0;
    // Pembukaan dan pengecekan keberadaan file
    FILE *file = fopen(namaFile, "r");
    if (file == NULL){
        printf("File tidak ada!");
        return 0;
    }
    // Inisiasi variable lokal yang akan digunakan untuk fungsi open_init
    char line[255];
    float latitude[15];
    float longitude[15];
    // Pembacaan file
    while (fgets(line, 255, file)){
        // Pemasukan data dari baris yang terbaca ke dalam masing masing variable dengan tipe data sesuai
        strcpy(namaKota[*n], strtok(line, ","));
        latitude[*n] = atof(strtok(NULL, ","));
        longitude[*n] = atof(strtok(NULL, "\n"));
        
        // printf("\n%s %f %f", namaKota[*n], latitude[*n], longitude[*n]);
        *n += 1;
    }
    
    // Pengecekan apakah jumlah kota sesuai ketentuan (6 <= jumlah kota <= 15)
    if (*n < 5 || *n > 14){
        printf("\nJumlah kota tidak sesuai!");
        return 0;
    }
    // Pengisian adjecent matriks, yang berisi jarak tiap titik ke titik lainnya
    for (int i = 0; i < *n; i++){
        for (int j = 0; j < *n; j++){
            // Komponen 1 : kolom, komponen 2 : baris
            matriks[i][j] = 2 * r * asin(sqrt(pow(sin((latitude[j]-latitude[i])*pi/360), 2) + cos(latitude[j]*pi/180) * cos(latitude[i]*pi/180) * pow(sin((longitude[j]-longitude[i])*pi/360), 2)));
        }
    }
    return 1;
}

void generate(int x) {
    if(x >= n) { // basis rekursi
        float cost = 0;
        for(int i = 1; i < n; ++i) {
            cost += matriks[permutation[i - 1]][permutation[i]];
        }

        cost += matriks[permutation[n - 1]][permutation[0]];

        if(cost < min_cost) { // update minimum cost and the best route
            min_cost = cost;
            for(int i = 0; i < n; ++i) {
                best_route[i] = permutation[i];
            }
        }
    } else {
        for(int i = 0; i < n; ++i) {
            if(vis[i]) continue;
            vis[i] = 1;
            permutation[x] = i;
            generate(x + 1);
            vis[i] = 0;
        }
    }
}

void find_starting_point(char *point, int *index) {
    for(int i = 0; i < n; ++i) {
        if(strcmp(point, namaKota[i]) == 0) {
            *index = i;
            return;
        }
    }
    *index = -1; // Tidak ditemukan
}

void output_best_route(){
    puts("Best route found:");
    for(int i = 0; i < n; ++i) {
        printf("%s -> ", namaKota[best_route[i]]);
    }

    printf("%s\n", namaKota[best_route[0]]);
    printf("Best route distance: %.5f km\n", min_cost);
}

int main() {
    char namaFile[255];
    n = 0;

    // Input File
    printf("Enter list of cities file name: ");
    scanf("%s", namaFile);


    if (open_init(namaFile, matriks, namaKota, &n) == 0){
        return 0;
    }

    size = n; // Update size to be the number of cities read

    int bestTour = (int)malloc(size * sizeof(int));
    float bestTourLength = -1;

    char startingCity[100];
    printf("Enter starting point: ");
    getchar(); // Menghilangkan karakter newline tersisa dari input sebelumnya
    fgets(startingCity, sizeof(startingCity), stdin);
    startingCity[strcspn(startingCity, "\n")] = '\0'; // Menghapus newline dari input

    // Find the index of the starting city
    int startingIndex;
    find_starting_point(startingCity, &startingIndex);

    // Handling case if the starting city is not found
    if (startingIndex == -1) {
        printf("Starting city not found\n");
        return 0;
    }

    vis[startingIndex] = 1; // node starting point ditandai
    permutation[0] = startingIndex;
    clock_t start_time = clock();
    generate(1);
    clock_t end_time = clock();
    output_best_route();

    double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    // Cetak waktu hasil eksekusi
    printf("Time elapsed: %.10f s\n", time_spent);

    return 0;
}
