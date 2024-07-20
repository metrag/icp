#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define MAX_GROUPS 10
#define MAX_STUDENTS 100

typedef struct {
    int *scores;
    int count;
    double *group_avg;
    double *total_avg;
    int shm_id_mutex;
} GroupData;

void calculate_group_average(GroupData* data) {
    int sum = 0;
    //суммируем оценки студентов
    for (int i = 0; i < data->count; i++) {
        sum += data->scores[i];
    }
    //вычисляем средний балл группы
    double avg = (double)sum / data->count;
    *(data->group_avg) = avg;

    //блокируем доступ к разделяемой памяти для общего среднего балла
    int *mutex = (int *)shmat(data->shm_id_mutex, NULL, 0);
    while (__sync_lock_test_and_set(mutex, 1)) {
       
    }
    *(data->total_avg) += avg;
    __sync_lock_release(mutex);
    shmdt(mutex);
}

int main() {
    int shm_id_avg = shmget(IPC_PRIVATE, sizeof(double), IPC_CREAT | 0666);
    double *total_avg = (double *)shmat(shm_id_avg, NULL, 0); //создаем разделяемую память для общего среднего балла
    *total_avg = 0.0;

    int shm_id_mutex = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    int *mutex = (int *)shmat(shm_id_mutex, NULL, 0); //создаем разделяемую память для мьютекса
    *mutex = 0;

    int num_groups;
    printf("Введите количество групп: ");
    scanf("%d", &num_groups);

    GroupData group_data[MAX_GROUPS];
    double group_averages[MAX_GROUPS];

    for (int i = 0; i < num_groups; i++) {
        int num_students;
        printf("Введите количество студентов в группе %d: ", i + 1);
        scanf("%d", &num_students);

        //ввод данных групп
        group_data[i].scores = (int *)malloc(num_students * sizeof(int)); //выделяем память для оценок студентов
        group_data[i].count = num_students;
        group_data[i].group_avg = &group_averages[i];
        group_data[i].total_avg = total_avg;
        group_data[i].shm_id_mutex = shm_id_mutex;

        printf("Введите оценки студентов группы %d:\n", i + 1);
        for (int j = 0; j < num_students; j++) {
            scanf("%d", &group_data[i].scores[j]);
        }

        if (fork() == 0) {
            calculate_group_average(&group_data[i]);
            exit(0);
        }
    }

    for (int i = 0; i < num_groups; i++) {
        wait(NULL); // Ожидаем завершения дочернего процесса
        printf("Средний балл группы %d: %.2f\n", i + 1, group_averages[i]); // Выводим средний балл группы
    }

    *total_avg /= num_groups;
    printf("Общий средний балл: %.2f\n", *total_avg);

    for (int i = 0; i < num_groups; i++) {
        free(group_data[i].scores);
    }

    shmdt(total_avg); // Отсоединяем разделяемую память для общего среднего балла от адресного пространства процесса
    shmctl(shm_id_avg, IPC_RMID, NULL); // Удаляем сегмент разделяемой памяти для общего среднего балла
    shmdt(mutex); // Отсоединяем разделяемую память для мьютекса от адресного пространства процесса
    shmctl(shm_id_mutex, IPC_RMID, NULL); // Удаляем сегмент разделяемой памяти для мьютекса

    return 0;
}