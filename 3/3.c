#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

int sem_id;
int sum = 0;


//простое ли число
bool is_prime(int num) {
    if (num <= 1) return false;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) return false;
    }
    return true;
}

//проверка на палиндром
bool is_palindrome(int num) {
    char str[32]; //int 4 б по 8 = 32
    sprintf(str, "%d", num); //в бинарном виде, так просто не выводит

    int len = strlen(str);
    //сравнение первых и последних битов    
    for (int i = 0; i < len / 2; i++)
        if (str[i] != str[len - i - 1])
            return 0;
    return 1;
}

//контроль доступа к семафору
void sem_lock(int sem_id) {
    struct sembuf sb = {0, -1, 0};
    semop(sem_id, &sb, 1);
}

void sem_unlock(int sem_id) {
    struct sembuf sb = {0, 1, 0};
    semop(sem_id, &sb, 1);
}

void find_palindromic_primes(int A, int B) {
    for (int i = A; i <= B; i++) {
        if (is_prime(i) && is_palindrome(i)) {
            sem_lock(sem_id);
            sum += i;
            sem_unlock(sem_id);
        }
    }
}

int main() {
    int A, B;
    printf("Введите число A: ");
    scanf("%d", &A);
    printf("Введите число B: ");
    scanf("%d", &B);

    //создание семафора
    sem_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    //установка начального значения семафора
    semctl(sem_id, 0, SETVAL, 1);

    pid_t pid1, pid2;
    int mid = (A + B) / 2;

    if ((pid1 = fork()) == 0) {
        find_palindromic_primes(A, mid);
        exit(0);
    }

    if ((pid2 = fork()) == 0) {
        find_palindromic_primes(mid + 1, B);
        exit(0);
    }

    //ждем завершения дочерних процессов
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    semctl(sem_id, 0, IPC_RMID);

    printf("\nСумма найденных чисел: %d\n", sum);
    return 0;
}
