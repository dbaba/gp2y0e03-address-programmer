#include "e_fuse/e_fuse.h"

/* I2C1 port, I2C Bus 1, with up to 16 devices */
I2C i2c(PB_9, PB_8);

DigitalOut vpp(VPP_PIN);

uint16_t gp2y0e03_read_distance(uint8_t address) {
    uint8_t register_address = 0x5E;
    uint8_t data[2];
    i2c.write(address, (char *) &register_address, 1);
    i2c.read(address, (char *) data, 2);
    return (((uint16_t) data[0] << 4) + (data[1] & 0x0f)) / 16 / 4;
}

/*
 * I2C Data writing
 */
static void _i2c_write(uint8_t register_address1, uint8_t register_address2) {
    uint8_t data[] = { register_address1, register_address2 };
    i2c.write(SOURCE_ADDRESS, (char *) data, 2);
}

/*
 * I2C Data reading
 */
static uint8_t _i2c_read(uint8_t register_address) {
    uint8_t data;
    i2c.write(SOURCE_ADDRESS, (char *) &register_address, 1);
    i2c.read(SOURCE_ADDRESS, (char *) &data, 1);
    return data;
}

/*
 * Vpp 3.3V Power Supply
 */
static void _vpp_on(void) {
    vpp = 1;
}

/*
 * Vpp Ground
 */
static void _vpp_off(void) {
    vpp = 0;
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 1
 * Data=0xFF is set in Address=0xEC.
 * 3.3V is applied in the Vpp terminal.
 */
static void e_fuse_stage1(Serial *pc) {
    pc->printf("stage 1\r\n");
    _i2c_write(0xEC, 0xFF);
    _vpp_on();
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 2
 * Data=0x00 is set in Address=0xC8.
 */
static void e_fuse_stage2(Serial *pc) {
    pc->printf("stage 2\r\n");
    _i2c_write(0xC8, 0x00);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 3
 * Data=0x45 is set in Address=0xC9.
 * + programming bit #: 5 => 5 - 1 = 4
 * + bank value: 5 => Bank E
 */
static void e_fuse_stage3(Serial *pc) {
    pc->printf("stage 3\r\n");
    _i2c_write(0xC9, 0x45);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 4
 * Data=0x01 is set in Address=0xCD.
 * (Data=0x01 for slave address being changed to 0x10(write) and 0x11(read))
 * @param new_address 0-15 (Default address is 8, 0x80 for writing and 0x81 for reading)
 */
static void e_fuse_stage4(Serial *pc, uint8_t new_address) {
    pc->printf("stage 4\r\n");
    _i2c_write(0xCD, new_address);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 5
 * Data=0x01 is set in Address=0xCA.
 * Wait for 500us.
 */
static void e_fuse_stage5(Serial *pc) {
    pc->printf("stage 5\r\n");
    _i2c_write(0xCA, 0x01);
    wait_us(500);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 6
 * Data=0x00 is set in Address=0xCA.
 * Vpp terminal is grounded.
 */
static void e_fuse_stage6(Serial *pc) {
    pc->printf("stage 6\r\n");
    _i2c_write(0xCA, 0x00);
    _vpp_off();
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 7
 * Data=0x00 is set in Address=0xEF.
 * Data=0x40 is set in Address=0xC8.
 * Data=0x00 is set in Address=0xC8.
 */
static void e_fuse_stage7(Serial *pc) {
    pc->printf("stage 7\r\n");
    _i2c_write(0xEF, 0x00);
    _i2c_write(0xC8, 0x40);
    _i2c_write(0xC8, 0x00);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 8
 * Data=0x06 is set in Address=0xEE.
 */
static void e_fuse_stage8(Serial *pc) {
    pc->printf("stage 8\r\n");
    _i2c_write(0xEE, 0x06);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 9
 * Data=0xFF is set in Address=0xEC.
 * Data=0x03 is set in Address=0xEF.
 * Read out the data in Address=0x27.
 * Data=0x00 is set in Address=0xEF.
 * Data=0x7F is set in Address=0xEC.
 *
 * @return 0 for success, 1 for failure : 0x27[4:0] & 0b10000(0x10)
 */
static uint8_t e_fuse_stage9(Serial *pc) {
    pc->printf("stage 9\r\n");
    // Table.20 List of E-Fuse program flow and setting value
    _i2c_write(0xEF, 0x00); // add this though it's missing in 12-6 Example of E-Fuse Programming
    _i2c_write(0xEC, 0xFF);
    _i2c_write(0xEF, 0x03);
    const uint8_t check_value = _i2c_read(0x27);
    const uint8_t check = check_value & 0x1f;
    pc->printf("Check 0x27[4:0] => %d\r\n", check);
    const uint8_t success = check & 0x10;
    // When lower 5bits data[4:0] is 00001, E-Fuse program is finished.
    // When lower 5bits data[4:0] is not 00001, go to stage10(bit replacement).
    _i2c_write(0xEF, 0x00);
    _i2c_write(0xEC, 0x7F);
    // Check Result
    return success;
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-1
 * Data=0xFF is set in Address=0xEC.
 * 3.3V is applied in Vpp terminal.
 */
static void e_fuse_stage10_1_1(Serial *pc) {
    pc->printf("stage 10-1-1\r\n");
    _i2c_write(0xEC, 0xFF);
    _vpp_on();
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-2
 * Data=0x37 is set in Address=0xC8.
 */
static void e_fuse_stage10_2_1(Serial *pc) {
    pc->printf("stage 10-2-1\r\n");
    _i2c_write(0xC8, 0x37);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-3
 * Data=0x74 is set in Address=0xC9.
 */
static void e_fuse_stage10_3_1(Serial *pc) {
    pc->printf("stage 10-3-1\r\n");
    _i2c_write(0xC9, 0x74);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-4
 * Data=0x04 is set in Address=0xCD.
 */
static void e_fuse_stage10_4_1(Serial *pc) {
    pc->printf("stage 10-4-1\r\n");
    _i2c_write(0xCD, 0x04);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-5
 * Data=0x01 is set in Address=0xCA.
 * Wait for 500us.
 */
static void e_fuse_stage10_5_1(Serial *pc) {
    pc->printf("stage 10-5-1\r\n");
    _i2c_write(0xCA, 0x01);
    wait_us(500);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-6
 * Data=0x00 is set in Address=0xCA.
 * Vpp terminal is grounded.
 */
static void e_fuse_stage10_6_1(Serial *pc) {
    pc->printf("stage 10-6-1\r\n");
    _i2c_write(0xCA, 0x00);
    _vpp_off();
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-1'
 * Data=0xFF is set in Address=0xEC.
 * 3.3V is applied in Vpp terminal.
 */
static void e_fuse_stage10_1_2(Serial *pc) {
    pc->printf("stage 10-1-2\r\n");
    _i2c_write(0xEC, 0xFF);
    _vpp_on();
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-2'
 * Data=0x3F is set in Address=0xC8.
 */
static void e_fuse_stage10_2_2(Serial *pc) {
    pc->printf("stage 10-2-2\r\n");
    _i2c_write(0xC8, 0x3F);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-3'
 * Data=0x04 is set in Address=0xC9.
 */
static void e_fuse_stage10_3_2(Serial *pc) {
    pc->printf("stage 10-3-2\r\n");
    _i2c_write(0xC9, 0x04);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-4'
 * Data=0x01 is set in Address=0xCD.
 */
static void e_fuse_stage10_4_2(Serial *pc) {
    pc->printf("stage 10-4-2\r\n");
    _i2c_write(0xCD, 0x01);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-5'
 * Data=0x01 is set in Address=0xCA.
 * Wait for 500us.
 */
static void e_fuse_stage10_5_2(Serial *pc) {
    pc->printf("stage 10-5-2\r\n");
    _i2c_write(0xCA, 0x01);
    wait_us(500);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-6'
 * Data=0x00 is set in Address=0xCA.
 * Vpp terminal is grounded.
 */
static void e_fuse_stage10_6_2(Serial *pc) {
    pc->printf("stage 10-6-2\r\n");
    _i2c_write(0xCA, 0x00);
    _vpp_off();
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-7
 * Data=0x00 is set in Address=0xEF.
 * Data=0x40 is set in Address=0xC8.
 * Data=0x00 is set in Address=0xC8.
 */
static void e_fuse_stage10_7(Serial *pc) {
    pc->printf("stage 10-7\r\n");
    _i2c_write(0xEF, 0x00);
    _i2c_write(0xC8, 0x40);
    _i2c_write(0xC8, 0x00);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-8
 * Data=0x06 is set in Address=0xEE.
 */
static void e_fuse_stage10_8(Serial *pc) {
    pc->printf("stage 10-8\r\n");
    _i2c_write(0xEE, 0x06);
}

/*
 * (Fig.40 E-Fuse Program Flow)
 * Stage 10-9
 * Data=0xFF is set in Address=0xEC.
 * Data=0x03 is set in Address=0xEF.
 * Read out the data in Address=0x18 and Address=0x19.
 */
static void e_fuse_stage10_9(Serial *pc) {
    pc->printf("stage 10-9\r\n");
    _i2c_write(0xEC, 0xFF);
    _i2c_write(0xEF, 0x03);
    const uint8_t bit_replacemnt_18 = _i2c_read(0x18);
    const uint8_t bit_replacemnt_19 = _i2c_read(0x19);
    pc->printf("Check 0x18 => %d\r\n", bit_replacemnt_18);
    pc->printf("Check 0x19 => %d\r\n", bit_replacemnt_19);
    if (bit_replacemnt_18 == 0x82 && bit_replacemnt_19 == 0x00) {
        pc->printf("Bit Replacement (stage 10) is SUCCESSFUL\r\n");
    } else {
        pc->printf("Bit Replacement (stage 10) is FAILURE\r\n");
    }
}

/*
 * 12-6 Example of E-Fuse Programming
 */
void e_fuse_run(Serial *pc, uint8_t new_address) {
    if (new_address == SOURCE_ADDRESS) {
        pc->printf("[ERROR] The new address must be other than 0x08!\r\n");
        return;
    }
    if (new_address > 0x0f) {
        pc->printf("[ERROR] The new address must be 0x0f or lower!\r\n");
        return;
    }

    _vpp_off();
    wait_us(500);

    e_fuse_stage1(pc);
    e_fuse_stage2(pc);
    e_fuse_stage3(pc);
    e_fuse_stage4(pc, new_address);
    e_fuse_stage5(pc);
    e_fuse_stage6(pc);
    e_fuse_stage7(pc);
    e_fuse_stage8(pc);
    const uint8_t result = e_fuse_stage9(pc);
    pc->printf("e_fuse_stage9():result => %d (0=success)\r\n", result);
    if (result) {
        e_fuse_stage10_1_1(pc);
        e_fuse_stage10_2_1(pc);
        e_fuse_stage10_3_1(pc);
        e_fuse_stage10_4_1(pc);
        e_fuse_stage10_5_1(pc);
        e_fuse_stage10_6_1(pc);

        e_fuse_stage10_1_2(pc);
        e_fuse_stage10_2_2(pc);
        e_fuse_stage10_3_2(pc);
        e_fuse_stage10_4_2(pc);
        e_fuse_stage10_5_2(pc);
        e_fuse_stage10_6_2(pc);

        e_fuse_stage10_7(pc);
        e_fuse_stage10_8(pc);
        e_fuse_stage10_9(pc);
    }
}
