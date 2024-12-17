#include <assert.h>
#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_THREADS 1

typedef enum {
    adv = 0,
    bxl = 1,
    bst = 2,
    jnz = 3,
    bxc = 4,
    out = 5,
    bdv = 6,
    cdv = 7,
} opcode_t;

typedef struct {
    long long int A, B, C, PC;
} cpu_state_t;

int print = 0;

long long int calc_combo(int x, const cpu_state_t *const cpu) {
    switch (x) {
        case 0:
        case 1:
        case 2:
        case 3:
            return x;
        case 4:
            return cpu->A;
        case 5:
            return cpu->B;
        case 6:
            return cpu->C;
        default:
            printf("Bad operand\n");
            exit(-1);
    }
}

void divide(cpu_state_t *cpu, long long int *out_reg, long long int combo) {
    if (combo == 0) {
        *out_reg = cpu->A;
        return;
    }
    if (combo > 0) {
        // printf("Combo too low %d\n", combo);
        *out_reg = cpu->A / (2 << (combo - 1));
    } else {
        *out_reg = cpu->A * (2 << ((-combo) - 1));
    }
}

int early_stop = 2 << 15;
void do_ins(cpu_state_t *cpu, opcode_t code, int rand, char *const out_str, int *const out_num,
            int *out_len) {
    switch (code) {
        case adv:
            divide(cpu, &cpu->A, calc_combo(rand, cpu));
            break;
        case bdv:
            divide(cpu, &cpu->B, calc_combo(rand, cpu));
            break;
        case cdv:
            divide(cpu, &cpu->C, calc_combo(rand, cpu));
            break;
        case bxl:
            cpu->B = cpu->B ^ rand;
            break;
        case bst:
            cpu->B = calc_combo(rand, cpu) % 8;
            break;
        case jnz:
            if (cpu->A == 0) {
                break;
            }
            cpu->PC = rand - 2;
            break;
        case bxc:
            cpu->B = cpu->B ^ cpu->C;
            break;
        case out: {
            int v = calc_combo(rand, cpu) % 8;
            if (print) {
                char tmp[8] = {0};
                sprintf(tmp, "%d,", v);
                strcat(out_str, tmp);
            } else {
                out_num[(*out_len)++] = v;
                if (*out_len > early_stop) {
                    cpu->PC = 2 << 15;
                    return;
                }
            }
            break;
        }
        default:
            printf("Bad ins\n");
            exit(-1);
    }
}

int go(cpu_state_t cpu_in, const int *const ins, const int num_ins, int *const out) {
    cpu_state_t *cpu = &cpu_in;
    char out_str[128] = {0};
    int out_len = 0;
    cpu->PC = 0;
    while (cpu->PC < num_ins) {
        int PC = cpu->PC;
        if (print) {
            printf("PC: %d, INS: (%d,%d), A: %lld, B: %lld, C: %lld\n", PC, (opcode_t)ins[PC],
                   ins[PC + 1], cpu->A, cpu->B, cpu->C);
        }
        do_ins(cpu, (opcode_t)ins[PC], ins[PC + 1], out_str, out, &out_len);
        if (out != NULL && out_len > 0 && out[out_len - 1] != ins[out_len - 1]) {
            return out_len;
        }
        cpu->PC += 2;
    }
    if (print) {
        printf("%s\n", out_str);
    }

    return out_len;
}

void generate(cpu_state_t cpu, const int *const ins, const int num_ins) {
    // Multithread
    long long int A = 0;
    int my_id = -1,
        *flag = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0),
        *num_threads = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *flag = MAX_THREADS;
    while (1) {
        int last_num_threads = *num_threads;
        int pid = fork();
        if (pid == 0) {
            my_id = (*num_threads)++;
            // printf("Thread %d created!\n", my_id);
            break;
        } else {
            while (last_num_threads == *num_threads) {
                usleep(100);
            }
            if (*num_threads == MAX_THREADS) {
                break;
            }
        }
    }

    if (my_id == -1) {
        while (*flag) {
            usleep(1000000);
        }
        return;
    }

    // A = my_id * (int)(((uint64_t)4294967293) / (uint64_t) MAX_THREADS);
    const long long limit = (2LL << 61);
    const long long int width = (long long int)(limit / (long long int) MAX_THREADS);
    const long long int start_point = my_id * width;
    A = start_point;
    printf("ID: %d %lld -> %f/100\n", my_id, A, 100 * (float)A / (float)(limit));

    print = 0;
    int out[256];
    early_stop = num_ins;
    do {
        cpu.A = A;
        int out_len = go(cpu, ins, num_ins, out);
        if (out_len > 2) {
            printf("A=%lld -> ", A);
            for (int i = 0; i < out_len && i < num_ins; i++) {
                printf("%d,", out[i]);
            }
            printf("\n");
        }
        if (out_len == num_ins) {
            int match = 1;
            for (int i = 0; i < num_ins; i++) {
                if (out[i] != ins[i]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                printf("\n\n|A = %lld|\n\n", A);
                *flag = 0;
                exit(0);
            }
        }
        A++;
        if (A % 1000000000 == 0) {
            printf("ID: %d %f/100\n", my_id, 100 * ((float)(A - start_point)) / (float)(limit));
        }
        if (*flag == 0) {
            exit(0);
        }
    } while (A <= start_point + width);
    // printf("Couldnt find an A:(");
    printf("ID: %d is done\n", my_id);
    (*flag)--;
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("input pls\n");
        return -1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("Oopsie daisies");
        return -1;
    }

    cpu_state_t cpu = {0};
    int ins[256], num_ins = 0;

    {
        char buff[256];
        fgets(buff, sizeof(buff), f);
        cpu.A = atoi(strchr(buff, ':') + 1);
        fgets(buff, sizeof(buff), f);
        cpu.B = atoi(strchr(buff, ':') + 1);
        fgets(buff, sizeof(buff), f);
        cpu.C = atoi(strchr(buff, ':') + 1);
        // Read in \n
        fgets(buff, sizeof(buff), f);
        printf("A: %lld, B: %lld, C: %lld\n", cpu.A, cpu.B, cpu.C);

        fgets(buff, sizeof(buff), f);
        char *token = buff;
        token = strtok(token, ":");
        while ((token = strtok(NULL, ",")) != NULL) {
            ins[num_ins++] = atoi(token);
        }
        for (int i = 0; i < num_ins; i++) {
            printf("%d,", ins[i]);
        }
        printf("\n");
    }

    printf("Part1 - Running Program...\n");
    print = 1;
    go(cpu, ins, num_ins, NULL);
    printf("Part2 - Running Program...\n");
    print = 0;
    generate(cpu, ins, num_ins);

    fclose(f);
    return 0;
}