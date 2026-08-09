/*
 * ec_probe.c — ITE IT5571 EC RAM 探测工具 (标准 ACPI EC 协议)
 * 用法:
 *   ec_probe dump                # 读全部 256 字节 EC RAM
 *   ec_probe read 0xNN           # 读单字节
 *   ec_probe write 0xNN 0xVV     # 写单字节 (危险!)
 *   ec_probe watch REG [DELAY]   # 持续监视某寄存器 (默认 1 秒)
 * 编译(Mac): zig cc -target x86_64-linux-musl -static -O2 -o ec_probe ec_probe.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/io.h>

#define EC_SC   0x66
#define EC_DATA 0x62

#define RD_EC   0x80
#define WR_EC   0x81

#define MAX_WAIT 20000  /* 轮询次数上限 */

static inline unsigned char inb_p(unsigned short port) {
    unsigned char v;
    __asm__ __volatile__("inb %1, %0" : "=a"(v) : "dN"(port));
    return v;
}
static inline void outb_p(unsigned short port, unsigned char v) {
    __asm__ __volatile__("outb %0, %1" : : "a"(v), "dN"(port));
}

/* 等待 IBF (bit1) 清空 —— 允许发命令 */
static int wait_ibf_clear(void) {
    int i;
    for (i = 0; i < MAX_WAIT; i++) {
        if (!(inb_p(EC_SC) & 0x02)) return 0;
        usleep(10);
    }
    return -1;
}

/* 等待 OBF (bit0) 置位 —— 数据可读 */
static int wait_obf_set(void) {
    int i;
    for (i = 0; i < MAX_WAIT; i++) {
        if (inb_p(EC_SC) & 0x01) return 0;
        usleep(10);
    }
    return -1;
}

static int ec_read_byte(unsigned char reg, unsigned char *val) {
    if (wait_ibf_clear()) return -1;
    outb_p(EC_SC, RD_EC);
    if (wait_ibf_clear()) return -1;
    outb_p(EC_DATA, reg);
    if (wait_obf_set()) return -1;
    *val = inb_p(EC_DATA);
    return 0;
}

static int ec_write_byte(unsigned char reg, unsigned char val) {
    if (wait_ibf_clear()) return -1;
    outb_p(EC_SC, WR_EC);
    if (wait_ibf_clear()) return -1;
    outb_p(EC_DATA, reg);
    if (wait_ibf_clear()) return -1;
    outb_p(EC_DATA, val);
    usleep(1000);
    return 0;
}

static void dump_all(void) {
    int r, c;
    unsigned char v;
    printf("EC RAM dump (256 bytes):\n");
    printf("      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
    for (r = 0; r < 16; r++) {
        printf("%02X: ", r * 16);
        for (c = 0; c < 16; c++) {
            if (ec_read_byte(r * 16 + c, &v) == 0)
                printf("%02X ", v);
            else
                printf("?? ");
        }
        printf("\n");
    }
    int nz = 0;
    for (r = 0; r < 256; r++) {
        if (ec_read_byte(r, &v) == 0 && v != 0) nz++;
    }
    printf("\n非零字节数: %d/256\n", nz);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s dump|read 0xNN|write 0xNN 0xVV|watch REG [sec]\n", argv[0]);
        return 1;
    }

    if (ioperm(0x60, 8, 1) < 0) {
        perror("ioperm (需要 root 或 CAP_SYS_RAWIO)");
        return 1;
    }

    unsigned char st = inb_p(EC_SC);
    printf("EC_SC 状态寄存器: 0x%02X (IBF=%d OBF=%d)\n", st, (st >> 1) & 1, st & 1);

    if (strcmp(argv[1], "dump") == 0) {
        dump_all();
    } else if (strcmp(argv[1], "read") == 0 && argc >= 3) {
        int reg = (int)strtol(argv[2], NULL, 16);
        unsigned char v;
        if (ec_read_byte(reg, &v) == 0)
            printf("EC[0x%02X] = 0x%02X (%d)\n", reg, v, v);
        else
            printf("读取超时 (EC 无响应?)\n");
    } else if (strcmp(argv[1], "write") == 0 && argc >= 4) {
        int reg = (int)strtol(argv[2], NULL, 16);
        int val = (int)strtol(argv[3], NULL, 16);
        unsigned char v;
        printf("写入 EC[0x%02X] = 0x%02X ...\n", reg, val);
        if (ec_write_byte(reg, val) == 0) {
            usleep(50000);
            if (ec_read_byte(reg, &v) == 0)
                printf("回读 EC[0x%02X] = 0x%02X (若与写入值不同, 可能是只读寄存器或被固件覆盖)\n", reg, v);
            else
                printf("写入成功(回读超时)\n");
        } else {
            printf("写入超时!\n");
        }
    } else if (strcmp(argv[1], "cmd") == 0 && argc >= 3) {
        /* 盲试 EC 扩展命令: cmd 0xNN 0xAA 0xBB [0xCC] */
        int c = (int)strtol(argv[2], NULL, 16);
        int a1 = argc >= 4 ? (int)strtol(argv[3], NULL, 16) : -1;
        int a2 = argc >= 5 ? (int)strtol(argv[4], NULL, 16) : -1;
        printf("发命令 0x%02X", c);
        if (a1 >= 0) printf(" arg=0x%02X", a1);
        if (a2 >= 0) printf(" 0x%02X", a2);
        printf("\n");
        if (wait_ibf_clear()) { printf("超时1\n"); return 1; }
        outb_p(EC_SC, c);
        if (wait_ibf_clear()) { printf("超时2\n"); return 1; }
        if (a1 >= 0) {
            outb_p(EC_DATA, a1);
            if (wait_ibf_clear()) { printf("超时3\n"); return 1; }
        }
        if (a2 >= 0) {
            outb_p(EC_DATA, a2);
            if (wait_ibf_clear()) { printf("超时4\n"); return 1; }
        }
        /* 读响应 (最多 4 字节) */
        usleep(10000);
        for (int i = 0; i < 4; i++) {
            unsigned char st = inb_p(EC_SC);
            if (!(st & 0x01)) break;
            printf("响应[%d] = 0x%02X\n", i, inb_p(EC_DATA));
        }
    } else if (strcmp(argv[1], "watch") == 0 && argc >= 3) {
        int reg = (int)strtol(argv[2], NULL, 16);
        int delay = argc >= 4 ? atoi(argv[3]) : 1;
        unsigned char prev = 0xFF, v;
        printf("监视 EC[0x%02X] (每 %d 秒, Ctrl-C 退出):\n", reg, delay);
        while (1) {
            if (ec_read_byte(reg, &v) == 0) {
                if (v != prev) {
                    printf("变化: EC[0x%02X] = 0x%02X (%d)\n", reg, v, v);
                    prev = v;
                }
            }
            sleep(delay);
        }
    } else {
        fprintf(stderr, "未知命令\n");
        return 1;
    }
    return 0;
}
