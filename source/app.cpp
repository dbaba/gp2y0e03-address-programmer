#include "mbed-drivers/mbed.h"
#include "e_fuse/e_fuse.h"

/*
 * 0x00 - 0x0f (0x08 is the preset address data, not an I2C address)
 * TODO Change HERE!
 */
#define NEW_ADDRESS_DATA 0x08

#define BAUD_RATE 115200

DigitalOut led(LED1);
DigitalIn button(BUTTON1);
Serial pc(SERIAL_TX, SERIAL_RX);

minar::callback_handle_t blinky_task;

static void blinky(void) {
    led = !led;
}

static void wait_for_button1(void) {
    led = 1;
    while (button);
    led = 0;
}

static void run_sensor_app(uint8_t new_address) {
    uint16_t distance = gp2y0e03_read_distance(new_address);
    pc.printf("Distance: %d\r\n", distance);
}

static void run_programmer(uint8_t new_address_data, uint8_t new_address) {
    pc.printf("** Welcome to Address Programmer!\r\n** I'll update the GP2Y0E03 I2C address from [0x80](Write) to [0x%02X](Write)\r\n", new_address);
    pc.printf("** You cannot revert the change once the address is updated\r\n** Push Button1 to continue!\r\n");
    wait_for_button1();
    pc.printf("+++++++++++++++++++++++++++++++++++\r\n");
    pc.printf("    L E T ' S   S T A R T ! ! !\r\n");
    pc.printf("+++++++++++++++++++++++++++++++++++\r\n");
    e_fuse_run(&pc, new_address_data);
    pc.printf("===================================\r\n");
    pc.printf("           F I N I S H\r\n");
    pc.printf("===================================\r\n");
}

void app_start(int, char**) {
    pc.baud(BAUD_RATE);
    button.mode(PullUp);
    const uint8_t new_address_data = NEW_ADDRESS_DATA;
    const uint8_t new_address = new_address_data * 0x10;
    if (!button) {
        mbed::util::Event i2cEvent(mbed::util::FunctionPointer1<void, uint8_t>(run_sensor_app).bind(new_address));
        minar::Scheduler::postCallback(i2cEvent).period(minar::milliseconds(40));
    } else {
        run_programmer(new_address_data, new_address);
    }
    minar::Scheduler::postCallback(blinky).period(minar::milliseconds(500));
}
