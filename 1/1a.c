#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

/*
1 2 3 3 2 1
>> -4 8 -4
*/

// вычисление каждого компонента
void vect_mult(int id, int *a, int *b, int *res) {
    if (id == 0) {  
        res[0] = a[1] * b[2] - a[2] * b[1];
    } else if (id == 1) {
        res[1] = a[2] * b[0] - a[0] * b[2];
    } else if (id == 2) {
        res[2] = a[0] * b[1] - a[1] * b[0];
    }
}

int main() {
    // ввод векторов
    int* a = (int*)malloc(3 * sizeof(int));
    int* b = (int*)malloc(3 * sizeof(int));
    const int len = 3;  

    for(int i = 0; i < len; i++){
        scanf("%d", &a[i]);
    }
    for(int i = 0; i < len; i++){
        scanf("%d", &b[i]);
    }

    // использование разделяемой памяти для результата
    int* result = mmap(NULL, 3 * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    for(int i = 0; i < len; i++){
        result[i] = 0;
    }

    /*
    mmap - позволяет процессам работать с одной и той же памятью, 
    что позволяет избежать копирования данных между процессами
    */

    // создание процессов
    pid_t pids[3];
    int status;

    // в каждом процессе вычисляем один компонент
    for (int i = 0; i < 3; i++) {
        if ((pids[i] = fork()) == 0) {
            vect_mult(i, a, b, result);
            exit(0);
        }
    }

    // ждем завершения всех процессов
    for (int i = 0; i < 3; i++) {
        waitpid(pids[i], &status, 0);
    }

    printf("Векторное произведение: [%d, %d, %d]\n", result[0], result[1], result[2]);

    // освобождение разделяемой памяти
    munmap(result, 3 * sizeof(int));
    free(a);
    free(b);

    return 0;
}