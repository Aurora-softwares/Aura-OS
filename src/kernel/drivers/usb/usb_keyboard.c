#include "drivers/usb/usb_keyboard.h"

#include <stddef.h>
#include <string.h>

#include "drivers/pci.h"
#include "drivers/usb/uhci.h"
#include "drivers/usb/usb.h"
#include "libs/serial.h"
#include "libs/assembly.h"

struct usb_keyboard_state {
    struct uhci_controller* controller;
    uint8_t device_address;
    uint8_t interface_number;
    uint8_t endpoint_address;
    uint8_t endpoint_max_packet;
    uint8_t endpoint_interval;
    bool ready;
    uint8_t prev_keys[6];
    bool caps_lock;
};

static struct usb_keyboard_state g_keyboard __attribute__((aligned(16)));

static void delay_cycles(uint32_t cycles) {
    for (uint32_t i = 0; i < cycles; ++i) {
        io_wait();
    }
}

static bool usb_control_request(uint8_t address,
                                uint8_t bmRequestType,
                                uint8_t bRequest,
                                uint16_t wValue,
                                uint16_t wIndex,
                                void* buffer,
                                uint16_t length) {
    serial_write("usb_control_request: addr=");
    serial_write_uint(address);
    serial_write(" type=");
    serial_write_hex8(bmRequestType);
    serial_write(" req=");
    serial_write_hex8(bRequest);
    serial_write(" wValue=");
    serial_write_hex16(wValue);
    serial_write(" wIndex=");
    serial_write_hex16(wIndex);
    serial_write(" len=");
    serial_write_uint(length);
    serial_write("\n");
    struct uhci_controller* controller = g_keyboard.controller;
    if (!controller) {
        serial_write("usb_control_request: no controller\n");
        return false;
    }

    struct usb_setup_packet setup = {
        .bmRequestType = bmRequestType,
        .bRequest = bRequest,
        .wValue = wValue,
        .wIndex = wIndex,
        .wLength = length,
    };

    struct uhci_setup_data transfer = {
        .setup = &setup,
        .data = buffer,
        .length = length,
        .direction_in = (bmRequestType & 0x80) != 0,
    };

    bool ok = uhci_control_transfer(controller, address, &transfer);
    serial_write(ok ? "usb_control_request: success\n"
                    : "usb_control_request: failure\n");
    return ok;
}

static bool usb_get_descriptor(uint8_t address,
                               uint8_t descriptor_type,
                               uint8_t descriptor_index,
                               void* buffer,
                               uint16_t length) {
    serial_write("usb_get_descriptor: addr=");
    serial_write_uint(address);
    serial_write(" type=");
    serial_write_hex8(descriptor_type);
    serial_write(" index=");
    serial_write_hex8(descriptor_index);
    serial_write(" len=");
    serial_write_uint(length);
    serial_write("\n");
    uint16_t value = ((uint16_t)descriptor_type << 8) | descriptor_index;
    bool ok = usb_control_request(address,
                               0x80, // Device-to-host, standard, device
                               USB_REQ_GET_DESCRIPTOR,
                               value,
                               0,
                               buffer,
                               length);
    serial_write(ok ? "usb_get_descriptor: success\n"
                    : "usb_get_descriptor: failure\n");
    return ok;
}

static bool usb_set_address(uint8_t new_address) {
    serial_write("usb_set_address: new=");
    serial_write_uint(new_address);
    serial_write("\n");
    if (!usb_control_request(0,
                             0x00, // Host-to-device, standard, device
                             USB_REQ_SET_ADDRESS,
                             new_address,
                             0,
                             NULL,
                             0)) {
        serial_write("usb_set_address: request failed\n");
        return false;
    }

    // Allow the device time to switch to the new address.
    delay_cycles(50000);
    g_keyboard.device_address = new_address;
    serial_write("usb_set_address: completed\n");
    return true;
}

static bool parse_configuration(const uint8_t* buffer, uint16_t total_length) {
    serial_write("parse_configuration: start length=");
    serial_write_uint(total_length);
    serial_write("\n");
    if (total_length < sizeof(struct usb_configuration_descriptor)) {
        serial_write("parse_configuration: total_length too small\n");
        return false;
    }

    const struct usb_configuration_descriptor* config =
        (const struct usb_configuration_descriptor*)buffer;

    uint8_t configuration_value = config->bConfigurationValue;

    uint16_t offset = config->bLength;
    uint8_t interface_number = 0xFF;
    uint8_t endpoint_address = 0;
    uint16_t endpoint_size = 8;
    uint8_t interval = 10;

    while (offset + 1 < total_length) {
        uint8_t length = buffer[offset];
        uint8_t descriptor_type = buffer[offset + 1];

        if (length == 0) {
            break;
        }

        if (offset + length > total_length) {
            break;
        }

        if (descriptor_type == USB_DESCRIPTOR_TYPE_INTERFACE) {
            const struct usb_interface_descriptor* iface =
                (const struct usb_interface_descriptor*)(buffer + offset);
            if (iface->bInterfaceClass == USB_CLASS_HID &&
                iface->bInterfaceSubClass == USB_SUBCLASS_BOOT &&
                iface->bInterfaceProtocol == USB_PROTOCOL_KEYBOARD) {
                interface_number = iface->bInterfaceNumber;
                serial_write("parse_configuration: keyboard interface found number=");
                serial_write_uint(interface_number);
                serial_write("\n");
            } else {
                interface_number = 0xFF;
                serial_write("parse_configuration: interface skipped\n");
            }
        } else if (descriptor_type == USB_DESCRIPTOR_TYPE_ENDPOINT && interface_number != 0xFF) {
            const struct usb_endpoint_descriptor* endpoint =
                (const struct usb_endpoint_descriptor*)(buffer + offset);
            if ((endpoint->bEndpointAddress & 0x80) &&
                (endpoint->bmAttributes & 0x3) == USB_ENDPOINT_TYPE_INTERRUPT) {
                endpoint_address = endpoint->bEndpointAddress;
                endpoint_size = endpoint->wMaxPacketSize;
                interval = endpoint->bInterval;
                serial_write("parse_configuration: endpoint address=");
                serial_write_hex8(endpoint_address);
                serial_write(" size=");
                serial_write_uint(endpoint_size);
                serial_write(" interval=");
                serial_write_uint(interval);
                serial_write("\n");
                break;
            }
        }

        offset += length;
    }

    if (interface_number == 0xFF || endpoint_address == 0) {
        serial_write("parse_configuration: failed to locate interface/endpoint\n");
        return false;
    }

    if (!usb_control_request(g_keyboard.device_address,
                             0x00,
                             USB_REQ_SET_CONFIGURATION,
                              configuration_value,
                              0,
                              NULL,
                              0)) {
        serial_write("parse_configuration: set configuration failed\n");
        return false;
    }

    if (!usb_control_request(g_keyboard.device_address,
                             0x21, // Host-to-device, Class, Interface
                             USB_REQ_SET_PROTOCOL,
                              0, // Boot protocol
                              interface_number,
                              NULL,
                              0)) {
        serial_write("parse_configuration: set protocol failed\n");
        return false;
    }

    if (!usb_control_request(g_keyboard.device_address,
                             0x21,
                             USB_REQ_SET_IDLE,
                              0,
                              interface_number,
                              NULL,
                              0)) {
        serial_write("parse_configuration: set idle failed\n");
        return false;
    }

    g_keyboard.interface_number = interface_number;
    g_keyboard.endpoint_address = endpoint_address;
    g_keyboard.endpoint_max_packet = endpoint_size ? (uint8_t)endpoint_size : 8;
    g_keyboard.endpoint_interval = interval;
    serial_write("parse_configuration: success\n");
    return true;
}

static bool enumerate_keyboard(void) {
    serial_write("enumerate_keyboard: begin\n");
    uint8_t buffer[256];

    // Read initial device descriptor (first 8 bytes).
    if (!usb_get_descriptor(0, USB_DESCRIPTOR_TYPE_DEVICE, 0, buffer, 8)) {
        serial_write("enumerate_keyboard: failed to read initial device descriptor\n");
        return false;
    }
    serial_write("enumerate_keyboard: got initial descriptor\n");

    if (!usb_set_address(1)) {
        serial_write("enumerate_keyboard: failed to set address\n");
        return false;
    }
    serial_write("enumerate_keyboard: address set\n");

    if (!usb_get_descriptor(g_keyboard.device_address, USB_DESCRIPTOR_TYPE_DEVICE, 0, buffer,
                            sizeof(struct usb_device_descriptor))) {
        serial_write("enumerate_keyboard: failed to read full device descriptor\n");
        return false;
    }
    serial_write("enumerate_keyboard: got device descriptor\n");

    struct usb_device_descriptor device_descriptor;
    memcpy(&device_descriptor, buffer, sizeof(device_descriptor));

    // Get configuration descriptor header to determine total length.
    if (!usb_get_descriptor(g_keyboard.device_address, USB_DESCRIPTOR_TYPE_CONFIGURATION, 0,
                            buffer, sizeof(struct usb_configuration_descriptor))) {
        serial_write("enumerate_keyboard: failed to read configuration header\n");
        return false;
    }
    serial_write("enumerate_keyboard: got configuration header\n");

    struct usb_configuration_descriptor config_descriptor;
    memcpy(&config_descriptor, buffer, sizeof(config_descriptor));

    uint16_t total_length = config_descriptor.wTotalLength;
    serial_write("enumerate_keyboard: configuration total length=");
    serial_write_uint(total_length);
    serial_write("\n");
    if (total_length > sizeof(buffer)) {
        total_length = sizeof(buffer);
        serial_write("enumerate_keyboard: total length clamped\n");
    }

    if (!usb_get_descriptor(g_keyboard.device_address, USB_DESCRIPTOR_TYPE_CONFIGURATION, 0,
                            buffer, total_length)) {
        serial_write("enumerate_keyboard: failed to read full configuration descriptor\n");
        return false;
    }

    if (!parse_configuration(buffer, total_length)) {
        serial_write("enumerate_keyboard: failed to parse configuration\n");
        return false;
    }
    serial_write("enumerate_keyboard: configuration parsed\n");

    memset(g_keyboard.prev_keys, 0, sizeof(g_keyboard.prev_keys));
    g_keyboard.caps_lock = false;

    (void)device_descriptor; // Currently unused but kept for potential future logging.
    serial_write("enumerate_keyboard: completed successfully\n");
    return true;
}

bool usb_keyboard_init(void) {
    serial_write("usb_keyboard_init: start\n");
    memset(&g_keyboard, 0, sizeof(g_keyboard));

    struct pci_device device;
    serial_write("usb_keyboard_init: scanning for UHCI controller\n");
    if (!pci_find_by_class(0x0C, 0x03, 0x00, &device)) {
        serial_write("usb_keyboard_init: no UHCI controller found\n");
        return false;
    }
    serial_write("usb_keyboard_init: controller found\n");

    if (!uhci_controller_init(&device)) {
        serial_write("usb_keyboard_init: uhci_controller_init failed\n");
        return false;
    }
    serial_write("usb_keyboard_init: uhci_controller_init succeeded\n");

    g_keyboard.controller = uhci_controller_get();
    if (!g_keyboard.controller) {
        serial_write("usb_keyboard_init: uhci_controller_get returned NULL\n");
        return false;
    }
    serial_write("usb_keyboard_init: controller handle acquired\n");

    if (!enumerate_keyboard()) {
        serial_write("usb_keyboard_init: enumerate_keyboard failed\n");
        return false;
    }

    g_keyboard.ready = true;
    serial_write("usb_keyboard_init: success\n");
    return true;
}

static bool is_new_key(uint8_t usage) {
    if (usage == 0) {
        return false;
    }
    for (size_t i = 0; i < sizeof(g_keyboard.prev_keys); ++i) {
        if (g_keyboard.prev_keys[i] == usage) {
            return false;
        }
    }
    return true;
}

static char translate_usage(uint8_t usage, uint8_t modifiers, bool caps_lock) {
    const bool shift = (modifiers & ((1 << 1) | (1 << 5))) != 0;

    if (usage >= 0x04 && usage <= 0x1D) {
        char base = (char)('a' + (usage - 0x04));
        bool upper = shift ^ caps_lock;
        return upper ? (char)(base - 32) : base;
    }

    switch (usage) {
    case 0x1E: return shift ? '!' : '1';
    case 0x1F: return shift ? '@' : '2';
    case 0x20: return shift ? '#' : '3';
    case 0x21: return shift ? '$' : '4';
    case 0x22: return shift ? '%' : '5';
    case 0x23: return shift ? '^' : '6';
    case 0x24: return shift ? '&' : '7';
    case 0x25: return shift ? '*' : '8';
    case 0x26: return shift ? '(' : '9';
    case 0x27: return shift ? ')' : '0';
    case 0x28: return '\n';
    case 0x2A: return '\b';
    case 0x2B: return '\t';
    case 0x2C: return ' ';
    case 0x2D: return shift ? '_' : '-';
    case 0x2E: return shift ? '+' : '=';
    case 0x2F: return shift ? '{' : '[';
    case 0x30: return shift ? '}' : ']';
    case 0x31: return shift ? '|' : '\\';
    case 0x33: return shift ? ':' : ';';
    case 0x34: return shift ? '"' : '\'';
    case 0x35: return shift ? '~' : '`';
    case 0x36: return shift ? '<' : ',';
    case 0x37: return shift ? '>' : '.';
    case 0x38: return shift ? '?' : '/';
    default:
        break;
    }

    return 0;
}

bool usb_keyboard_poll_char(char* out_char) {
    if (!g_keyboard.ready || !out_char) {
        serial_write("usb_keyboard_poll_char: controller not ready or invalid output\n");
        return false;
    }

    uint8_t report[8] = {0};
    uint16_t packet_size = g_keyboard.endpoint_max_packet;
    if (packet_size == 0 || packet_size > sizeof(report)) {
        packet_size = (uint16_t)sizeof(report);
    }
    struct uhci_interrupt_transfer transfer = {
        .endpoint = g_keyboard.endpoint_address,
        .buffer = report,
        .length = packet_size,
    };

    if (!uhci_interrupt_poll(g_keyboard.controller, g_keyboard.device_address, &transfer)) {
        serial_write("usb_keyboard_poll_char: interrupt poll failed\n");
        return false;
    }

    uint8_t modifiers = report[0];
    uint8_t* keys = &report[2];

    char result = 0;
    bool caps_toggled = false;

    for (size_t i = 0; i < 6; ++i) {
        uint8_t usage = keys[i];
        if (usage == 0) {
            continue;
        }

        if (!is_new_key(usage)) {
            continue;
        }

        if (usage == 0x39) { // Caps Lock
            g_keyboard.caps_lock = !g_keyboard.caps_lock;
            caps_toggled = true;
            continue;
        }

        result = translate_usage(usage, modifiers, g_keyboard.caps_lock);
        if (result != 0) {
            break;
        }
    }

    memcpy(g_keyboard.prev_keys, keys, sizeof(g_keyboard.prev_keys));

    if (caps_toggled || result == 0) {
        serial_write("usb_keyboard_poll_char: no character produced\n");
        return false;
    }

    *out_char = result;
    serial_write("usb_keyboard_poll_char: produced char=");
    serial_write_hex8((uint8_t)result);
    serial_write("\n");
    return true;
}
