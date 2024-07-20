#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_LEN 1024

/*
int pipe(int pipefd[2]);

pipefd: Массив из двух целых чисел. 
После вызова pipe(), pipefd[0] будет использоваться для чтения из канала, 
а pipefd[1] — для записи в канал.
*/
int main() {
    int pipe1[2], pipe2[2], pipe3[2];
    pid_t pid1, pid2, pid3;
    char S[MAX_LEN], a, b;

    // Создаем каналы
    pipe(pipe1);
    pipe(pipe2);
    pipe(pipe3);

    // Первый процесс: ввод строки S
    if ((pid1 = fork()) == 0) {
        close(pipe1[0]); //закрытие чтения из канала
        printf("\nВведите строку S: ");
        fgets(S, MAX_LEN, stdin);
        write(pipe1[1], S, strlen(S) + 1); //запись в канал
        close(pipe1[1]); //закрытие записи в канал
        exit(0);
    }
    waitpid(pid1, NULL, 0); // Ожидаем завершения первого процесса

    // Второй процесс: ввод символа a
    if ((pid2 = fork()) == 0) {
        close(pipe2[0]);
        printf("\nВведите символ a: ");
        scanf(" %c", &a);
        write(pipe2[1], &a, sizeof(char));
        close(pipe2[1]);
        exit(0);
    }
    waitpid(pid2, NULL, 0); // Ожидаем завершения второго процесса

    // Третий процесс: ввод символа b
    if ((pid3 = fork()) == 0) {
        close(pipe3[0]);
        printf("\nВведите символ b: ");
        scanf(" %c", &b);
        write(pipe3[1], &b, sizeof(char));
        close(pipe3[1]);
        exit(0);
    }
    waitpid(pid3, NULL, 0); // Ожидаем завершения третьего процесса

    // Чтение данных из каналов после завершения всех процессов
    close(pipe1[1]); //закрытие записи в канал
    memset(S, 0, MAX_LEN); // обнуление строки S
    read(pipe1[0], S, MAX_LEN);
    close(pipe1[0]); //закрытие чтения из канала
    printf("Полученная строка S: %s\n", S); // вывод промежуточного значения

    close(pipe2[1]);
    read(pipe2[0], &a, sizeof(char));
    close(pipe2[0]);
    printf("Полученный символ a: %c\n", a); // вывод промежуточного значения

    close(pipe3[1]);
    read(pipe3[0], &b, sizeof(char));
    close(pipe3[0]);
    printf("Полученный символ b: %c\n", b); // вывод промежуточного значения

    // Заменяем символ a на символ b в строке S
    for (int i = 0; i < strlen(S); i++) {
        if (S[i] == a) {
            S[i] = b;
        }
    }

    printf("Результат: %s\n", S);

    return 0;
}