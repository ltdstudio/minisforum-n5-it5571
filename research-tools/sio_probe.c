/*
 * sio_probe.c — ITE IT5571 Super I/O 配置空间探测
 * 用法:
 *   sio_probe id          # 读芯片 ID (0x20/0x21)
 *   sio_probe scan        # 扫描所有 LDN 的寄存器
 *   sio_probe tach        # 重点读 TACH 相关寄存器 (LDN 4)
 * 编译(Mac): zig cc -target x86_64-linux-musl -static -O2 -o sio_probe sio_probe.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>

#define SIO_IDX 0x2E
#define SIO_DAT 0x2F

static inline unsigned char inb_p(unsigned short port) {
    unsigned char v;
    __asm__ __volatile__("inb %1, %0" : "=a"(v) : "dN"(port));
    return v;
}
static inline void outb_p(unsigned short port, unsigned char v) {
    __asm__ __volatile__("outb %0, %1" : : "a"(v), "dN"(port));
}

static void sio_write(unsigned char reg, unsigned char val) {
    outb_p(SIO_IDX, reg);
    outb_p(SIO_DAT, val);
}
static unsigned char sio_read(unsigned char reg) {
    outb_p(SIO_IDX, reg);
    return inb_p(SIO_DAT);
}
static void sio_enter(void) {
    outb_p(SIO_IDX, 0x87);
    outb_p(SIO_IDX, 0x87);
}
static void sio_exit(void) {
    outb_p(SIO_IDX, 0xAA);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s id|scan|tach\n", argv[0]);
        return 1;
    }
    if (ioperm(0x2E, 2, 1) < 0) {
        perror("ioperm");
        return 1;
    }
    sio_enter();

    if (strcmp(argv[1], "id") == 0) {
        unsigned char idh = sio_read(0x20);
        unsigned char idl = sio_read(0x21);
        printf("芯片 ID: 0x%02X%02X\n", idh, idl);
    } else if (strcmp(argv[1], "scan") == 0) {
        unsigned char idh = sio_read(0x20);
        unsigned char idl = sio_read(0x21);
        printf("芯片 ID: 0x%02X%02X\n", idh, idl);
        /* 扫描 LDN 0-15 */
        for (int ldn = 0; ldn < 16; ldn++) {
            sio_write(0x07, ldn);
            unsigned char d = sio_read(0x30);  /* activate */
            if (d == 0) continue;
            printf("\n=== LDN %d (active=0x%02X) ===\n", ldn, d);
            /* 读前 0x30 个寄存器 */
            for (int r = 0x60; r <= 0x7F; r++) {
                unsigned char v = sio_read(r);
                if (v != 0x00 && v != 0xFF)
                    printf("  reg[0x%02X] = 0x%02X (%d)\n", r, v, v);
            }
        }
    } else if (strcmp(argv[1], "tach") == 0) {
        /* 扫多个 LDN 的完整寄存器 */
        for (int ldn = 0; ldn < 16; ldn++) {
            sio_write(0x07, ldn);
            unsigned char act = sio_read(0x30);
            if (act == 0) continue;
            printf("\n=== LDN %d (active=0x%02X) regs 0x00-0x7F ===\n", ldn, act);
            for (int r = 0x00; r <= 0x7F; r++) {
                unsigned char v = sio_read(r);
                if (v != 0x00 && v != 0xFF)
                    printf("  [0x%02X]=0x%02X(%d)  ", r, v, v);
            }
            printf("\n");
        }
    } else {
        fprintf(stderr, "未知命令\n");
    }
    sio_exit();
    return 0;
}
