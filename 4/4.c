#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_PAIRS 100
#define MSG_SIZE sizeof(struct msgbuf) - sizeof(long)

struct msgbuf {
    long mtype; // Тип сообщения
    int pairs[MAX_PAIRS][2]; // Пары чисел
    int count; // Количество пар
};

void find_pairs(int *arr, int n, int start, int end, int k, int msgid) {
    struct msgbuf message;
    message.mtype = 1; // Устанавливаем тип сообщения
    message.count = 0;

    printf("Дочерний процесс %d обрабатывает элементы с %d по %d\n", getpid(), start, end);

    for (int i = start; i < end; i++) {
        for (int j = 0; j < n; j++) { // Проверяем все элементы массива
            if (arr[j] - arr[i] == k) {
                message.pairs[message.count][0] = arr[i];
                message.pairs[message.count][1] = arr[j];
                message.count++;
                printf("Найдена пара: (%d, %d)\n", arr[i], arr[j]);
            }
        }
    }

    printf("Дочерний процесс %d найдено пар: %d\n", getpid(), message.count);
    
    // Проверка успешности отправки сообщения
    if (msgsnd(msgid, &message, MSG_SIZE, 0) == -1) {
        perror("Ошибка при отправке сообщения");
        exit(1);
    }
}

int main() {
    int n, k;

    // Вводим размер массива и значение k
    printf("Введите размер массива (n > 3): ");
    scanf("%d", &n);
    if (n <= 3) {
        printf("Размер массива должен быть больше 3.\n");
        return 1;
    }

    printf("Введите значение k: ");
    scanf("%d", &k);

    int *arr = (int *)malloc(n * sizeof(int)); // Выделяем память для массива
    printf("Введите элементы массива:\n");
    for (int i = 0; i < n; i++) {
        scanf("%d", &arr[i]);
    }

    int msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT); // Создаем очередь сообщений
    if (msgid == -1) {
        perror("Ошибка при создании очереди сообщений");
        free(arr);
        return 1;
    }

    for (int i = 0; i < 4; i++) {
        pid_t pid = fork();
        if (pid < 0) { // Проверка на ошибку при создании процесса
            perror("Ошибка при создании дочернего процесса");
            free(arr);
            exit(1);
        }
        if (pid == 0) { // Дочерний процесс
            int start = i * (n / 4);
            int end = (i == 3) ? n : (i + 1) * (n / 4); // Обработка последнего процесса
            find_pairs(arr, n, start, end, k, msgid);
            exit(0);
        }
    }

    // Родительский процесс
    int max_product = -1;
    int best_a = 0, best_b = 0;

    for (int i = 0; i < 4; i++) {
        struct msgbuf message;
        // Проверка успешности получения сообщения
        if (msgrcv(msgid, &message, MSG_SIZE, 1, 0) == -1) {
            perror("Ошибка при получении сообщения");
            free(arr);
            exit(1);
        }

        for (int j = 0; j < message.count; j++) {
            int a = message.pairs[j][0];
            int b = message.pairs[j][1];
            int product = abs(a) * abs(b);
            if (product > max_product) {
                max_product = product;
                best_a = a;
                best_b = b;
            }
        }
    }

    for (int i = 0; i < 4; i++) {
        wait(NULL); // Ожидаем завершения всех дочерних процессов
    }

    msgctl(msgid, IPC_RMID, NULL); // Удаляем очередь сообщений
    free(arr); // Освобождаем память

    if (max_product == -1) {
        printf("Пары не найдены.\n");
    } else {
        printf("Пара с максимальным значением |a| * |b|: (%d, %d)\n", best_a, best_b);
    }

    return 0;
}