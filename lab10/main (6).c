/*
 * main.c
 * 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#define NUM_THREADS 10
#define MAX_SIZE 100

double f1(double x){
	double y = x*x + 5;
	return y;
}

struct Thread1 {
    double a;              
    double b;              
    double (*fp)(double);  
};

struct Thread2 {
    int n;                  
    double x_start;         
    double x_end;           
    double (*fp)(double);   
};

struct Thread3 {
    int row;
    int col;
    int data[MAX_SIZE][MAX_SIZE];
};


void *thread_routine1(void *arg) { 
    struct Thread1 *params = (struct Thread1 *)arg;          /* Argümanları struct türüne dönüştürme */
    double a = params->a;                                    
    double b = params->b;  
    double (*f)(double) = params->fp;                        

    double result = ((f(a) + f(b)) / 2.0) * (b - a);         

    double *result_ptr = malloc(sizeof(double));             /* Sonucu heap'e yazdırıyoruz */
    *result_ptr = result;
 
    return (void *)result_ptr;                               /* Sonucun adresini döndür */
}


double sum ;                                              
pthread_mutex_t mutex;                                       /* Global sum ve mutex değişkeni */

void *thread_routine2(void *arg) { 
    struct Thread2 *params= (struct Thread2 *)arg;           
    
    int n = params->n;                                       /* İhtiyaç duyulan parametreleri aldık */
    double x_start = params->x_start;
    double x_end = params->x_end;
    double (*f)(double) = params->fp;
    
    double h = (x_end - x_start) / n;                        /* Belirtilen aralıktaki yamukların alanı hesaplanıyor */
    double local_sum = 0;
    for (int i = 0; i < n; ++i) 
    {
        double x0 = x_start + i * h;
        double x1 = x0 + h;
        local_sum += h * (f(x0) + f(x1)) / 2;
    }

    pthread_mutex_lock(&mutex);                              
    sum += local_sum;
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);                                      
}


/* Dosyadan matris okumasını yapıyoruz */
struct Thread3 read_matrix(const char *filename) {
    struct Thread3 matrix;
    FILE *file = fopen(filename, "r");
    if (file == NULL) 
    {
        perror("Dosya açılırken hata!!!");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%d %d", &matrix.row, &matrix.col);

    for (int i = 0; i < matrix.row; i++) 
    {
        for (int j = 0; j < matrix.col; j++) 
        {
            fscanf(file, "%d", &matrix.data[i][j]);
        }
    }

    fclose(file);
    return matrix;
}



void print_matrix(struct Thread3 matrix) {
    printf("Matris %dx%d:\n", matrix.row, matrix.col);
    for (int i = 0; i < matrix.row; i++) 
    {
        for (int j = 0; j < matrix.col; j++) 
        {
            printf("%d ", matrix.data[i][j]);
        }
        printf("\n");
    }
}

/* Matris çarpımı bulma */
void multiply_matrix(struct Thread3 matrix1, struct Thread3 matrix2) {
    
    struct Thread3 result;
    result.row = matrix1.row;
    result.col = matrix2.col;

    for (int i = 0; i < matrix1.row; i++) 
    {
        for (int j = 0; j < matrix2.col; j++) 
        {
            int sum = 0;
            for (int k = 0; k < matrix1.col; k++) 
            {
                sum += matrix1.data[i][k] * matrix2.data[k][j];
            }
            result.data[i][j] = sum;
        }
    }
    printf("Matris çarpımı:\n");
    print_matrix(result);
}

int main(int argc, char **argv){
    /* Durum 1 */
    struct Thread1 thread1;                                /* Struct değişkeni oluşturup parametreleri atamalarını yaptık */
    thread1.a = 0;
    thread1.b = 10;
    thread1.fp = f1;

    pthread_t tid1;                                           
    pthread_create(&tid1, NULL, thread_routine1, &thread1);    /* Thread'i oluşturma */

    void *value;                                             
    pthread_join(tid1, &value);
    double *result_ptr = (double *)value;                    /* Oluşturulan pointer adresini joine verdik */
    double result = *result_ptr;
    printf("the integral f1(x): %lf\n", result);

    free(result_ptr);                                        
    
    
    /* Durum 2 */
    pthread_mutex_init(&mutex, NULL);                        /* Mutexi başlattık */
    struct Thread2 array[NUM_THREADS];                       /* Thread argümanı dizisi oluşturma */

    double interval = 20.0 / NUM_THREADS;                    /* Aralığı array eleman sayısına bölüp uzunluğu bul */
    
    pthread_t threads[NUM_THREADS];                          
    for (int i = 0; i < NUM_THREADS; ++i) 
    {
        array[i].n = 1000000;    /* Böldüğümüz yamuk sayısı */
        array[i].x_start = i * interval;
        array[i].x_end = (i + 1) * interval;
        array[i].fp = f1;
        pthread_create(&threads[i], NULL, thread_routine2, &array[i]);
    }
    for (int i = 0; i < NUM_THREADS; ++i) 
    {
    pthread_join(threads[i], NULL);
    }
    printf("the integral f2(x): %lf\n", sum);
    pthread_mutex_destroy(&mutex);                           
    
    
    /* Durum 3 */
    struct Thread3 mat1 = read_matrix(argv[1]);               
    struct Thread3 mat2 = read_matrix(argv[2]);

    print_matrix(mat1);
    print_matrix(mat2);

    multiply_matrix(mat1, mat2);                           

    return 0;
}





