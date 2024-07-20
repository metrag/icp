#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define MAX_N 100

// Функция для вычисления суммы элементов столбца
int column_sum(int matrix[MAX_N][MAX_N], int n, int col) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += matrix[i][col];
    }
    return sum;
}

int main() {
    int n;
    printf("Введите размер матрицы n: ");
    scanf("%d", &n);

    int matrix[MAX_N][MAX_N];
    printf("Введите элементы матрицы:\n"); //по строчно
        for (int j = 0; j < n; j++) {
            scanf("%d", &matrix[i][j]);
        }
    }

    
    // Создание разделяемой памяти для хранения суммы особых элементов и их количества
    int shm_id_sum = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    int shm_id_count = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    int *shared_sum = (int *)shmat(shm_id_sum, NULL, 0);
    int *shared_count = (int *)shmat(shm_id_count, NULL, 0);
    *shared_sum = 0;
    *shared_count = 0;

    pid_t pids[MAX_N];

    for (int col = 0; col < n; col++) {
        if ((pids[col] = fork()) == 0) {
            // Дочерний процесс
            int col_sum = column_sum(matrix, n, col);
            for (int i = 0; i < n; i++) {
                if (matrix[i][col] > col_sum - matrix[i][col]) {
                    // Использование разделяемой памяти для обновления суммы и количества
                    __sync_fetch_and_add(shared_sum, matrix[i][col]); // Обновляем сумму    
                    __sync_fetch_and_add(shared_count, 1); // Увеличиваем счетчик
                }
            }
            exit(0);
        }
    }

    // Ожидание завершения всех дочерних процессов
    for (int i = 0; i < n; i++) {
        waitpid(pids[i], NULL, 0);
    }

    if (*shared_count > 0) {
        double average = (double)*shared_sum / *shared_count;
        printf("Среднее арифметическое особых элементов: %.2f\n", average);
    } else {
        printf("Особых элементов не найдено.\n");
    }

    // Освобождение разделяемой памяти
    shmdt(shared_sum); // Отсоединение разделяемой памяти от адресного пространства процесса
    shmdt(shared_count);
    shmctl(shm_id_sum, IPC_RMID, NULL); // Удаление разделяемого сегмента памяти
    shmctl(shm_id_count, IPC_RMID, NULL);

    return 0;
}