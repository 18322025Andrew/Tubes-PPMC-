#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<math.h>
#include <time.h>

#include "input.h"
#include "haversine.h"
#include "struct.h"

bool next_permutation(int* arr, int n);
void swap(int* a, int* b);
void reverse(int* arr, int start, int end);

//fungsi menghitung jumlah kota
int amount_vertex(int *jumlah_kota, Node *linked){
  Node *temp = linked;
  while(temp != NULL){
    (*jumlah_kota) ++ ;
    temp = temp->next;
  }
  return(0);
}

//fungsi menghitung
int fill_vertex(Node* linked, char *kota[]){
  Node *temp = linked;
  int i = 0;
  while(temp != NULL){
    strcpy(kota[i], temp->nama_kota);
    temp = temp->next;
    i++;
  }
  return(0);
}

//fungsi mengisi array nama kota dan nilai lintang dan bujur
int find_lintang_bujur(Node *data, int indeks, char *kota[], double *lintang, double *bujur){
  char nama[MAX_LEN_STRING];
  strcpy(nama, kota[indeks]);

  Node *temp = data;

  while (temp != NULL)
  {
    if(strcmp(nama, temp->nama_kota) == 0){
      (*lintang) = temp->lintang;
      (*bujur) = temp->bujur;
      return(0);
    }
    temp = temp->next;
  }
  
  return(0);
}

//fungsi mencari indeks kota awal di array vertex
int find_indeks(char *kota[], char *nama_kota, int jumlah_kota){
  int indeks;
  for(int i = 0; i < jumlah_kota; i ++){
    if(strcmp(kota[i], nama_kota) == 0){
      indeks = i;
    }
  }
  return(indeks);
}

//fungsi memasukkan jarah antar kota ke matriks
int insert_matriks(double **matriks, Node *linked, int jumlah_kota, char *nama_kota[]){
  double lintang1, bujur1, lintang2, bujur2;
  for(int i = 0; i < jumlah_kota; i ++){
    //bagian mencari lintang dan bujur
    find_lintang_bujur(linked, i, nama_kota, &lintang1, &bujur1);
    for(int j = 0; j < jumlah_kota; j ++){
      if(i == j){matriks[i][j] = 0;
      }
      else{
        find_lintang_bujur(linked, j, nama_kota,&lintang2, &bujur2);
        //bagian isi
        matriks[i][j] = haversine(lintang1, bujur1, lintang2, bujur2);
      }
    }
  } 
  return(0);
}

//fungsi menghitung jarak total di array path
double calculate_total_distance(double **Matriks, int *path, int jumlah_kota) {
    double total_length = 0;
    for (int i = 0; i < jumlah_kota - 1; i++) {
        total_length += Matriks[path[i]][path[i + 1]];
    }
    total_length += Matriks[path[jumlah_kota - 1]][path[0]];
    return total_length;
}

//algoritma ilp
void tsp_ilp(double **Matriks, int jumlah_kota, int start_index, int *best_path, double *min_length) {
    //inisialisaisi array permutasi
    int perm[jumlah_kota - 1];
    int perm_index = 0;
    for (int i = 0; i < jumlah_kota; i++) {
        if (i != start_index) {
            perm[perm_index++] = i;
        }
    }

    *min_length = INFINITY;
    int current_path[jumlah_kota];
    do {
        current_path[0] = start_index;
        for (int i = 0; i < jumlah_kota - 1; i++) {
            current_path[i + 1] = perm[i];
        }

        double current_length = calculate_total_distance(Matriks, current_path, jumlah_kota);
        if (current_length < *min_length) {
            *min_length = current_length;
            // bagian mengisi elemen array current path ke best path 
            memcpy(best_path, current_path, jumlah_kota * sizeof(int));
        }
    } while (next_permutation(perm, jumlah_kota - 1));
}

//fungsi print array path
void print_path(int *path, int jumlah_kota, char *vertex[]) {
    printf("Path: ");
    for (int i = 0; i < jumlah_kota; i++) {
        printf("%s -> ", vertex[path[i]]);
    }
    printf("%s\n", vertex[path[0]]);
}

//fungsi permutasi
bool next_permutation(int* arr, int n) {
    int i = n - 2;
    while (i >= 0 && arr[i] >= arr[i + 1]) {
        i--;
    }
    if (i < 0) {
        return false;
    }
    int j = n - 1;
    while (arr[j] <= arr[i]) {
        j--;
    }
    swap(&arr[i], &arr[j]);
    reverse(arr, i + 1, n - 1);
    return true;
}

//fungsi tukar posisi
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

//fungsi mengubah urutan array
void reverse(int* arr, int start, int end) {
    while (start < end) {
        swap(&arr[start], &arr[end]);
        start++;
        end--;
    }
}

int main(){
  Node *linked_data = input_file();

  int jumlah_vertex = 0; 
  if(linked_data == NULL){return(0);}

  amount_vertex(&jumlah_vertex, linked_data);

  char *vertex[jumlah_vertex];
  for (int i = 0; i < jumlah_vertex; i++) {
    vertex[i] = malloc(MAX_LEN_STRING * sizeof(char));
    if (vertex[i] == NULL) {
      // Memory allocation failed
      printf("Memory allocation failed for vertex[%d]\n", i);
      return 1; // Terminate program
    }
  }

  fill_vertex(linked_data, vertex);
  //alokasi memori ke matrik 
  double **Matriks = (double **)malloc(jumlah_vertex * sizeof(double *));
  for (int i = 0; i < jumlah_vertex; i++) {
      Matriks[i] = (double *)malloc(jumlah_vertex * sizeof(double));
  }

  insert_matriks(Matriks, linked_data, jumlah_vertex, vertex);
  
  char nama[MAX_LEN_STRING];
  printf("Initial city : ");
  scanf(" %[^\r\n]", nama);
  int initial_indeks = find_indeks(vertex, nama, jumlah_vertex);

  if (initial_indeks == -1) {
      printf("City not found.\n");
      return 1;
  }

  int best_path[jumlah_vertex];
  double min_length;

  clock_t t;
  t = clock();
  tsp_ilp(Matriks, jumlah_vertex, initial_indeks, best_path, &min_length);//algo ilp
  t = clock() - t;
  // print
  print_path(best_path, jumlah_vertex, vertex);
  printf("Minimum path length: %.10f\n", min_length);
  double time_taken = ((double)t)/CLOCKS_PER_SEC;
  //print waktu
  printf("ILP algorithm took %f s to excute\n", time_taken);

  // free capasity
  for (int i = 0; i < jumlah_vertex; i++) {
      free(Matriks[i]);
      free(vertex[i]);
  }
  free(Matriks);
  free(linked_data);
  return(0);
}
