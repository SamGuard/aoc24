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
            }
            if (out_num) {
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
    if (print) printf("A=%16llx\n", cpu->A);
    while (cpu->PC < num_ins) {
        int PC = cpu->PC;
        if (print && ins[PC] == 5) {
            // printf("PC: %d, INS: (%d,%d), A: %lld, B: %lld, C: %lld\n", PC, (opcode_t)ins[PC],
            //        ins[PC + 1], cpu->A, cpu->B, cpu->C);
            // printf("A=%12llx -> %lld\n", cpu->A, cpu->A % 8);
        }
        do_ins(cpu, (opcode_t)ins[PC], ins[PC + 1], out_str, out, &out_len);
        cpu->PC += 2;
    }
    if (print) {
        // printf("A=%16llx\n", cpu->A);
        printf("%s\n", out_str);
    }

    return out_len;
}

int f(cpu_state_t cpu, const int *const ins, const int num_ins, int shift, uint64_t *io_A) {
    if (shift < 0) {
        return 1;
    }
    print = 0;
    uint64_t A = *io_A;
    int out[32], index = (int)shift / 3;
    for (uint64_t cwn = 0x0; cwn < 8; cwn++) {
        A &= (0xffffffffffff ^ 0b111LL << shift);
        A |= cwn << shift;
        printf("Trying A = %12llx, cwn = %lld\n", (long long)A, (long long)cwn);
        cpu.A = A;
        int out_len = go(cpu, ins, num_ins, out);
        if (index >= out_len) {
            continue;
        }
        if (out[index] != ins[index]) {
            continue;
        }
        printf("Found %d at shift %d\n", out[index], shift);
        {
            // Go try this val
            uint64_t test_A = A;
            if (f(cpu, ins, num_ins, shift - 3, &test_A)) {
                *io_A = test_A;
                return 1;
            }
        }
    }
    printf("Failed, going back a level %d\n", shift + 3);
    return 0;
}

void work_back(const cpu_state_t cpu_in, const int *const ins, const int num_ins) {
    print = 1;
    uint64_t A = 0x000000000000;
    int res = f(cpu_in, ins, num_ins, 45, &A);
    if (res) {
        printf("Part2 Answer: %lld\n", (long long)A);
    } else {
        printf("NOT Part2 Answer: %lld\n", (long long)A);
    }
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
    work_back(cpu, ins, num_ins);
    print = 0;
    // generate(cpu, ins, num_ins);

    fclose(f);
    return 0;
}