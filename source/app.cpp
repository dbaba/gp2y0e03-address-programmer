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
    button.mode(PullUp);
    led = 1;
    while (button);
    led = 0;
}

void app_start(int, char**) {
    pc.baud(BAUD_RATE);
    const uint8_t new_address_data = NEW_ADDRESS_DATA;
    const uint8_t new_address = new_address_data * 0x10;
    pc.printf("** Welcome to Address Programmer!\n** I'll update the GP2Y0E03 I2C address from [0x80](Write) to [0x%02X](Write)\n", new_address);
    pc.printf("** You cannot revert the change once the address is updated\n** Push Button1 to continue!\n");
    wait_for_button1();
    pc.printf("+++++++++++++++++++++++++++++++++++\n");
    pc.printf("    L E T ' S   S T A R T ! ! !\n");
    pc.printf("+++++++++++++++++++++++++++++++++++\n");
    e_fuse_run(&pc, new_address_data);
    pc.printf("===================================\n");
    pc.printf("           F I N I S H\n");
    pc.printf("===================================\n");
    minar::Scheduler::postCallback(blinky).period(minar::milliseconds(500));
}
