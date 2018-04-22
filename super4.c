#include <ftdi.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
struct ftdi_context ftdic;
unsigned char bits[1];
int ret;

/*
List connected TCTEC Relays
*/
void listRelays() {
    int ret, i, bRelayStates;
    struct ftdi_device_list *devlist, *curdev;
    char manufacturer[128], description[128], serial[128], buf[1];
    if ((ret = ftdi_usb_find_all(&ftdic, &devlist, 0x0403, 0x6001)) < 0) {
        fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", ret,
                ftdi_get_error_string(&ftdic));
        return;
    }
    printf("Number of FTDI devices found: %d\n", ret);
    for (curdev = devlist; curdev != NULL; i++) {
        if ((ret = ftdi_usb_get_strings(&ftdic, curdev->dev, manufacturer, 128,
                                        description, 128, serial, 128)) < 0) {
            fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", ret,
                    ftdi_get_error_string(&ftdic));
            return;
        }
        if (strncmp(description, "TCTEC USB RELAY", 15) == 0) {
            printf("Manufacturer: %s, Description: %s, Serial: %s\n\n",
                   manufacturer, description, serial);
        }
        curdev = curdev->next;
    }
    ftdi_list_free(&devlist);
    ftdi_deinit(&ftdic);
    return;
}

int readPins() {
    int pins;
    ret = ftdi_read_pins(&ftdic, bits);
    /*
            Bits will be returned as 0xF ?.Subtract off 0xF0 to get it to a 0 -
       F number. Subtract that from 15 to get a 1 - 15 number where 1 is on and
       0 is off for each of the pins
    */
    pins = (int)(0xF - (bits[0] - 0xF0));
    switch (pins) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 9:
    case 11:
    case 13:
    case 15:
        printf("LED 1: On\n");
        break;
    default:
        printf("LED 1: Off\n");
    }
    switch (pins) {
    case 2:
    case 3:
    case 6:
    case 7:
    case 10:
    case 11:
    case 14:
    case 15:
        printf("LED 2: On\n");
        break;
    default:
        printf("LED 2: Off\n");
    }
    switch (pins) {
    case 4:
    case 5:
    case 6:
    case 7:
    case 12:
    case 13:
    case 14:
    case 15:
        printf("LED 3: On\n");
        break;
    default:
        printf("LED 3: Off\n");
    }
    if (pins >= 8) {
        printf("LED 4: On\n");
    } else {
        printf("LED 4: Off\n");
    }
    return pins;
}

void setPins(int pins) {
    if (pins < 0 || pins > 15) {
        printf("Set value must be 0-15 (0b0000-0b1111).\n%i is an invalid "
               "entry\n\n",
               pins);
        return;
    }
    /*
    1) The type must be a char, so cast it
    2) 0xF0 is the base.
    3) On=0, Off=1, so subtract that from 0xF (15) to get the bits to write
    */
    bits[0] = (char)(0xF0 + (0xF - pins));
    // Written twice because of the buffer
    ret = ftdi_write_data(&ftdic, bits, 1);
    ret = ftdi_write_data(&ftdic, bits, 1);
}

void initBoard(char *serial) {
    // Init FTDI communication
    ftdi_init(&ftdic);
    if (0) {
        // Open
        ret = ftdi_usb_open(&ftdic, 0x0403, 0x6001);
    } else {
        // Open via serial number, to be implemented later.
        ret = ftdi_usb_open_desc(&ftdic, 0x0403, 0x6001, NULL, serial);
    }
    ret = ftdi_read_pins(&ftdic, bits);
    // Board has just been plugged in or power lost
    if (bits[0] == 0xFF) {
        // Prevents 0xFF from flipping all switches on
        // ftdi_enable_bitbang(&ftdic, 0xF0);
        ftdi_set_bitmode(&ftdic, 0xF0, BITMODE_BITBANG);
        setPins(0);
    }
    ftdi_set_bitmode(&ftdic, 0xFF, BITMODE_BITBANG);
    // ftdi_enable_bitbang(&ftdic, 0xFF);
}

void helpMessage() { printf("Help Message\n\n"); }

int main(int argc, char **argv) {
    static int read, mask;
    int pins;
    int c;
    int option_index = 0;
    char *serial = NULL;
    static struct option long_options[] = {
        {"mask", required_argument, 0, 'm'},
        {"serial", required_argument, 0, 's'},
        {"read", no_argument, 0, 'r'},
        {"list", no_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0} /* This is a filler for -1 */
    };
    if (argc == 1) {
        helpMessage();
        return 1;
    }
    while ((c = getopt_long(argc, argv, "m:rhls:", long_options,
                            &option_index)) != -1) {
        switch (c) {
        case 'l':
            listRelays();
            return 1;
            break;
        case 'r':
            read = 1;
            break;
        case 'm':
            mask = 1;
            pins = atoi(optarg);
            break;
        case 's':
            serial = optarg;
            break;
        case 'h':
            helpMessage();
            return 0;
            break;
        default:
            helpMessage();
            return 1;
            break;
        }
    }
    if (read || mask) {
        initBoard(serial);
    }
    if (mask) {
        setPins(pins);
    }
    if (read) {
        return readPins();
    }
    exit(0);
    return (0);
}
