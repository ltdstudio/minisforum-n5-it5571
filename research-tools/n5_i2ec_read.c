/* Read-only IT5571 I2EC diagnostic for the Minisforum N5. */
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>

#define I2EC_INDEX 0x2e
#define I2EC_DATA  0x2f

static void i2ec_select(uint8_t reg)
{
	outb(0x2e, I2EC_INDEX);
	outb(reg, I2EC_DATA);
}

static void i2ec_write_address_reg(uint8_t reg, uint8_t value)
{
	i2ec_select(reg);
	outb(0x2f, I2EC_INDEX);
	outb(value, I2EC_DATA);
}

static uint8_t i2ec_read(uint16_t address)
{
	i2ec_write_address_reg(0x11, address >> 8);
	i2ec_write_address_reg(0x10, address & 0xff);
	i2ec_select(0x12);
	outb(0x2f, I2EC_INDEX);
	return inb(I2EC_DATA);
}

int main(int argc, char **argv)
{
	unsigned long address, count = 1, i;
	char *end;

	if (argc < 2 || argc > 3) {
		fprintf(stderr, "usage: %s ADDRESS [COUNT]\n", argv[0]);
		return 2;
	}
	address = strtoul(argv[1], &end, 0);
	if (*end || address > 0xffff) {
		fprintf(stderr, "invalid address\n");
		return 2;
	}
	if (argc == 3) {
		count = strtoul(argv[2], &end, 0);
		if (*end || count == 0 || count > 256 || address + count > 0x10000) {
			fprintf(stderr, "invalid count\n");
			return 2;
		}
	}
	if (ioperm(I2EC_INDEX, 2, 1)) {
		fprintf(stderr, "ioperm: %s\n", strerror(errno));
		return 1;
	}
	for (i = 0; i < count; i++) {
		if ((i & 15) == 0)
			printf("%04lx:", address + i);
		printf(" %02x", i2ec_read(address + i));
		if ((i & 15) == 15 || i + 1 == count)
			putchar('\n');
	}
	return 0;
}
