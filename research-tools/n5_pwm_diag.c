/* Temporary N5/IT5571 PMC2 PWM mapping diagnostic. */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/io.h>

#define PMC2_DATA 0x68
#define PMC2_SC   0x6c
#define PMC2_N5   0xd5
#define WAIT_LOOPS 20000

static inline uint8_t io_read8(uint16_t port)
{
    uint8_t value;
    __asm__ __volatile__("inb %1, %0" : "=a"(value) : "dN"(port));
    return value;
}

static inline void io_write8(uint16_t port, uint8_t value)
{
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "dN"(port));
}

static int wait_bit(uint8_t mask, int want_set)
{
    int i;
    for (i = 0; i < WAIT_LOOPS; i++) {
        if (!!(io_read8(PMC2_SC) & mask) == want_set)
            return 0;
        usleep(10);
    }
    return -1;
}

static void drain_output(void)
{
    int i;
    for (i = 0; i < 16 && (io_read8(PMC2_SC) & 1); i++)
        (void)io_read8(PMC2_DATA);
}

static int pmc2_begin(uint8_t subcommand)
{
    drain_output();
    if (wait_bit(2, 0))
        return -1;
    io_write8(PMC2_SC, PMC2_N5);
    if (wait_bit(2, 0))
        return -1;
    io_write8(PMC2_DATA, subcommand);
    if (wait_bit(2, 0))
        return -1;
    return 0;
}

static int pmc2_action(uint8_t subcommand)
{
    return pmc2_begin(subcommand);
}

static int pmc2_write(uint8_t subcommand, uint8_t value)
{
    if (pmc2_begin(subcommand))
        return -1;
    io_write8(PMC2_DATA, value);
    return wait_bit(2, 0);
}

static int set_channel(unsigned int channel, uint8_t pwm)
{
    static const uint8_t curve1[] = {0x23, 0x25, 0x27, 0x29};
    static const uint8_t curve4[] = {0x33, 0x35, 0x37, 0x39};
    unsigned int i;

    switch (channel) {
    case 1:
        for (i = 0; i < 4; i++)
            if (pmc2_write(curve1[i], pwm))
                return -1;
        return pmc2_action(0x20);
    case 2:
        if (pmc2_write(0x2f, pwm))
            return -1;
        return pmc2_action(0x2d);
    case 3:
        if (pmc2_write(0x2c, pwm))
            return -1;
        return pmc2_action(0x2a);
    case 4:
        for (i = 0; i < 4; i++)
            if (pmc2_write(curve4[i], pwm))
                return -1;
        return pmc2_action(0x30);
    default:
        return -1;
    }
}

static int set_auto(unsigned int channel)
{
    static const uint8_t disable[] = {0, 0x21, 0x2e, 0x2b, 0x31};
    if (channel < 1 || channel > 4)
        return -1;
    return pmc2_action(disable[channel]);
}

int main(int argc, char **argv)
{
    unsigned long channel, value;
    char *end;

    if (argc < 3) {
        fprintf(stderr, "usage: %s set CHANNEL VALUE|auto CHANNEL\n", argv[0]);
        return 2;
    }
    channel = strtoul(argv[2], &end, 0);
    if (*end || channel < 1 || channel > 4) {
        fprintf(stderr, "invalid channel\n");
        return 2;
    }
    if (ioperm(PMC2_DATA, 1, 1) || ioperm(PMC2_SC, 1, 1)) {
        fprintf(stderr, "ioperm: %s\n", strerror(errno));
        return 1;
    }
    if (!strcmp(argv[1], "auto")) {
        if (set_auto(channel)) {
            fprintf(stderr, "PMC2 timeout\n");
            return 1;
        }
        printf("channel %lu returned to EC automatic mode\n", channel);
        return 0;
    }
    if (strcmp(argv[1], "set") || argc != 4) {
        fprintf(stderr, "usage: %s set CHANNEL VALUE|auto CHANNEL\n", argv[0]);
        return 2;
    }
    value = strtoul(argv[3], &end, 0);
    if (*end || value > 255) {
        fprintf(stderr, "invalid PWM value\n");
        return 2;
    }
    if (set_channel(channel, value)) {
        fprintf(stderr, "PMC2 timeout\n");
        return 1;
    }
    printf("channel %lu manual PWM=%lu\n", channel, value);
    return 0;
}
