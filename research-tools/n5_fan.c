/*
 * n5_fan.c - Minisforum N5 / ITE IT5571 fan telemetry reader
 *
 * Reverse-engineered from F8NAA EC firmware V0.14.
 * The firmware exposes three cached 16-bit RPM values through PMC2 command
 * family 0xd5:
 *   TACH3: subcommands 0x14 (low), 0x15 (high)
 *   TACH2: subcommands 0x16 (low), 0x17 (high)
 *   TACH1: subcommands 0x18 (low), 0x19 (high)
 *
 * The currently requested PWM percentage remains available through the
 * standard ACPI EC byte at offset 0x34 (read command 0x80).
 *
 * Build on Linux:
 *   cc -O2 -Wall -Wextra -o n5_fan n5_fan.c
 * Run as root (or with CAP_SYS_RAWIO):
 *   ./n5_fan status
 *   ./n5_fan rpm
 *   ./n5_fan pwm
 *   ./n5_fan raw 0x18
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/io.h>

#define SIO_INDEX 0x2e
#define SIO_DATA  0x2f
#define SIO_LDN_REG 0x07
#define SIO_CHIP_ID_HI 0x20
#define SIO_CHIP_ID_LO 0x21
#define SIO_ACTIVATE_REG 0x30
#define SIO_IOBAD0_HI 0x60
#define SIO_IOBAD0_LO 0x61
#define SIO_IOBAD1_HI 0x62
#define SIO_IOBAD1_LO 0x63
#define LDN_PMC1 0x11
#define LDN_PMC2 0x12

#define ACPI_EC_READ 0x80
#define N5_PMC2_FAN_COMMAND 0xd5
#define N5_PWM_EC_OFFSET 0x34
#define WAIT_LOOPS 20000

struct pm_ports {
    uint16_t data;
    uint16_t command;
    uint8_t active;
};

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

static void sio_write(uint8_t reg, uint8_t value)
{
    io_write8(SIO_INDEX, reg);
    io_write8(SIO_DATA, value);
}

static uint8_t sio_read(uint8_t reg)
{
    io_write8(SIO_INDEX, reg);
    return io_read8(SIO_DATA);
}

static void sio_enter(void)
{
    io_write8(SIO_INDEX, 0x87);
    io_write8(SIO_INDEX, 0x87);
}

static void sio_exit(void)
{
    io_write8(SIO_INDEX, 0xaa);
}

static struct pm_ports read_pm_ports(uint8_t ldn)
{
    struct pm_ports ports;
    sio_write(SIO_LDN_REG, ldn);
    ports.active = sio_read(SIO_ACTIVATE_REG);
    ports.data = (uint16_t)((sio_read(SIO_IOBAD0_HI) << 8) |
                            sio_read(SIO_IOBAD0_LO));
    ports.command = (uint16_t)((sio_read(SIO_IOBAD1_HI) << 8) |
                               sio_read(SIO_IOBAD1_LO));
    return ports;
}

static int wait_ibf_clear(uint16_t status_port)
{
    int i;
    for (i = 0; i < WAIT_LOOPS; ++i) {
        if (!(io_read8(status_port) & 0x02))
            return 0;
        usleep(10);
    }
    return -1;
}

static int wait_obf_set(uint16_t status_port)
{
    int i;
    for (i = 0; i < WAIT_LOOPS; ++i) {
        if (io_read8(status_port) & 0x01)
            return 0;
        usleep(10);
    }
    return -1;
}

static void drain_output(const struct pm_ports *ports)
{
    int i;
    for (i = 0; i < 16 && (io_read8(ports->command) & 0x01); ++i)
        (void)io_read8(ports->data);
}

static int pm_fan_read(const struct pm_ports *ports, uint8_t subcommand,
                       uint8_t *value)
{
    drain_output(ports);
    if (wait_ibf_clear(ports->command) != 0)
        return -1;
    io_write8(ports->command, N5_PMC2_FAN_COMMAND);
    if (wait_ibf_clear(ports->command) != 0)
        return -1;
    io_write8(ports->data, subcommand);
    if (wait_obf_set(ports->command) != 0)
        return -1;
    *value = io_read8(ports->data);
    return 0;
}

static int acpi_ec_read(const struct pm_ports *pmc1, uint8_t reg,
                        uint8_t *value)
{
    drain_output(pmc1);
    if (wait_ibf_clear(pmc1->command) != 0)
        return -1;
    io_write8(pmc1->command, ACPI_EC_READ);
    if (wait_ibf_clear(pmc1->command) != 0)
        return -1;
    io_write8(pmc1->data, reg);
    if (wait_obf_set(pmc1->command) != 0)
        return -1;
    *value = io_read8(pmc1->data);
    return 0;
}

static int read_rpm(const struct pm_ports *pmc2, unsigned int tach,
                    uint16_t *rpm)
{
    static const uint8_t low_command[] = {0, 0x18, 0x16, 0x14};
    static const uint8_t high_command[] = {0, 0x19, 0x17, 0x15};
    uint8_t high_before, high_after, low;
    int attempt;

    if (tach < 1 || tach > 3)
        return -1;

    /* Avoid a torn 16-bit sample if firmware refreshes between commands. */
    for (attempt = 0; attempt < 4; ++attempt) {
        if (pm_fan_read(pmc2, high_command[tach], &high_before) != 0 ||
            pm_fan_read(pmc2, low_command[tach], &low) != 0 ||
            pm_fan_read(pmc2, high_command[tach], &high_after) != 0)
            return -1;
        if (high_before == high_after) {
            *rpm = (uint16_t)((high_before << 8) | low);
            return 0;
        }
    }
    return -1;
}

static void print_usage(const char *program)
{
    fprintf(stderr,
            "Usage: %s status|rpm|pwm|probe|raw COMMAND\n", program);
}

int main(int argc, char **argv)
{
    struct pm_ports pmc1, pmc2;
    uint16_t chip_id;
    uint8_t pwm, raw;
    uint16_t rpm[3];
    unsigned int i;

    if (argc < 2) {
        print_usage(argv[0]);
        return 2;
    }

    if (ioperm(SIO_INDEX, 2, 1) != 0) {
        fprintf(stderr, "ioperm(0x2e): %s (run as root)\n", strerror(errno));
        return 1;
    }

    sio_enter();
    chip_id = (uint16_t)((sio_read(SIO_CHIP_ID_HI) << 8) |
                         sio_read(SIO_CHIP_ID_LO));
    pmc1 = read_pm_ports(LDN_PMC1);
    pmc2 = read_pm_ports(LDN_PMC2);
    sio_exit();

    if (chip_id != 0x5571) {
        fprintf(stderr, "Unsupported Super I/O/EC chip ID 0x%04x; expected 0x5571\n",
                chip_id);
        return 1;
    }

    if (pmc1.data == 0 || pmc1.command == 0) {
        pmc1.data = 0x62;
        pmc1.command = 0x66;
    }
    if (pmc2.data == 0 || pmc2.command == 0) {
        pmc2.data = 0x68;
        pmc2.command = 0x6c;
    }

    if (ioperm(pmc1.data, 1, 1) != 0 ||
        ioperm(pmc1.command, 1, 1) != 0 ||
        ioperm(pmc2.data, 1, 1) != 0 ||
        ioperm(pmc2.command, 1, 1) != 0) {
        fprintf(stderr, "ioperm(PMC): %s (run as root)\n", strerror(errno));
        return 1;
    }

    if (strcmp(argv[1], "probe") == 0) {
        printf("ITE chip ID: 0x%04x\n", chip_id);
        printf("PMC1 active=0x%02x data=0x%04x command/status=0x%04x\n",
               pmc1.active, pmc1.data, pmc1.command);
        printf("PMC2 active=0x%02x data=0x%04x command/status=0x%04x\n",
               pmc2.active, pmc2.data, pmc2.command);
        return 0;
    }

    if (!(pmc1.active & 0x01) || !(pmc2.active & 0x01)) {
        fprintf(stderr,
                "Required PMC interface is disabled (PMC1=0x%02x, PMC2=0x%02x); run 'probe'\n",
                pmc1.active, pmc2.active);
        return 1;
    }

    if (strcmp(argv[1], "raw") == 0) {
        char *end;
        unsigned long command;
        if (argc != 3) {
            print_usage(argv[0]);
            return 2;
        }
        command = strtoul(argv[2], &end, 0);
        if (*end != '\0' || command > 0xff) {
            fprintf(stderr, "Invalid command: %s\n", argv[2]);
            return 2;
        }
        if (pm_fan_read(&pmc2, (uint8_t)command, &raw) != 0) {
            fprintf(stderr, "PMC2 command 0x%02x subcommand 0x%02lx timed out\n",
                    N5_PMC2_FAN_COMMAND, command);
            return 1;
        }
        printf("PMC2[0x%02x:0x%02lx] = 0x%02x (%u)\n",
               N5_PMC2_FAN_COMMAND, command, raw, raw);
        return 0;
    }

    if (strcmp(argv[1], "pwm") == 0) {
        if (acpi_ec_read(&pmc1, N5_PWM_EC_OFFSET, &pwm) != 0) {
            fprintf(stderr, "ACPI EC read of 0x34 timed out\n");
            return 1;
        }
        printf("PWM request: %u%% (EC[0x34]=0x%02x)\n", pwm, pwm);
        return 0;
    }

    if (strcmp(argv[1], "rpm") != 0 && strcmp(argv[1], "status") != 0) {
        print_usage(argv[0]);
        return 2;
    }

    for (i = 0; i < 3; ++i) {
        if (read_rpm(&pmc2, i + 1, &rpm[i]) != 0) {
            fprintf(stderr, "TACH%u RPM read timed out\n", i + 1);
            return 1;
        }
    }

    if (strcmp(argv[1], "status") == 0) {
        if (acpi_ec_read(&pmc1, N5_PWM_EC_OFFSET, &pwm) != 0) {
            fprintf(stderr, "ACPI EC read of 0x34 timed out\n");
            return 1;
        }
        printf("PWM request: %u%% (EC[0x34]=0x%02x)\n", pwm, pwm);
    }
    printf("TACH1: %u RPM\n", rpm[0]);
    printf("TACH2: %u RPM\n", rpm[1]);
    printf("TACH3: %u RPM\n", rpm[2]);
    return 0;
}
