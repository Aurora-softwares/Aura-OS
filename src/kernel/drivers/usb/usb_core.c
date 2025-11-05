#include "drivers/usb/usb_core.h"

#include <stdbool.h>
#include <stdint.h>
#include "assembly.h"
#include "drivers/pci.h"
#include "drivers/usb/uhci.h"
#include "drivers/usb/usb.h"
#include "libk/string.h"
#include "serial.h"

#define USB_MAX_CONTROLLERS 16
#define USB_DESCRIPTOR_BUFFER_SIZE 256
#define USB_ADDRESS_START 1

struct usb_port_summary {
    bool connected;
    bool low_speed;
    bool enumerated;
    uint8_t address;
    struct usb_device_descriptor descriptor;
};

struct usb_controller_summary {
    struct pci_device pci;
    uint8_t port_count;
    bool initialized;
    struct usb_port_summary ports[UHCI_MAX_PORTS];
};

static struct usb_controller_summary g_controllers[USB_MAX_CONTROLLERS];
static size_t g_controllers_used = 0;
static size_t g_controllers_detected = 0;
static bool g_usb_ready = false;

static void delay_cycles(uint32_t cycles) {
    for (uint32_t i = 0; i < cycles; ++i) {
        io_wait();
    }
}

static void reset_summaries(void) {
    memset(g_controllers, 0, sizeof(g_controllers));
    g_controllers_used = 0;
    g_controllers_detected = 0;
    g_usb_ready = false;
}

static bool usb_control_request(struct uhci_controller* controller,
                                uint8_t address,
                                uint8_t bmRequestType,
                                uint8_t bRequest,
                                uint16_t wValue,
                                uint16_t wIndex,
                                void* buffer,
                                uint16_t length) {
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
        .direction_in = (bmRequestType & USB_DIR_IN) != 0,
    };

    return uhci_control_transfer(controller, address, &transfer);
}

static bool usb_get_descriptor(struct uhci_controller* controller,
                               uint8_t address,
                               uint8_t descriptor_type,
                               uint8_t descriptor_index,
                               void* buffer,
                               uint16_t length) {
    uint16_t value = ((uint16_t)descriptor_type << 8) | descriptor_index;
    return usb_control_request(controller,
                               address,
                               0x80, // Device-to-host, standard, device
                               USB_REQ_GET_DESCRIPTOR,
                               value,
                               0,
                               buffer,
                               length);
}

static bool usb_set_address(struct uhci_controller* controller, uint8_t new_address) {
    if (!usb_control_request(controller,
                             0,
                             0x00, // Host-to-device, standard, device
                             USB_REQ_SET_ADDRESS,
                             new_address,
                             0,
                             NULL,
                             0)) {
        return false;
    }

    delay_cycles(50000);
    return true;
}

static bool enumerate_device_on_port(struct uhci_controller* controller,
                                     uint8_t port_index,
                                     uint8_t address,
                                     struct usb_device_descriptor* out_descriptor) {
    uint8_t buffer[USB_DESCRIPTOR_BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    uhci_select_port(controller, port_index);

    if (!usb_get_descriptor(controller, 0, USB_DESCRIPTOR_TYPE_DEVICE, 0, buffer, 8)) {
        serial_write("usb: failed to read initial device descriptor\n");
        return false;
    }

    if (!usb_set_address(controller, address)) {
        serial_write("usb: failed to assign address\n");
        return false;
    }

    if (!usb_get_descriptor(controller,
                            address,
                            USB_DESCRIPTOR_TYPE_DEVICE,
                            0,
                            buffer,
                            sizeof(struct usb_device_descriptor))) {
        serial_write("usb: failed to read full device descriptor\n");
        return false;
    }

    memcpy(out_descriptor, buffer, sizeof(struct usb_device_descriptor));
    return true;
}

static void log_controller_header(size_t index, const struct pci_device* device) {
    serial_write("usb: controller ");
    serial_write_uint((uint32_t)index);
    serial_write(" -> bus=");
    serial_write_uint(device->bus);
    serial_write(" slot=");
    serial_write_uint(device->slot);
    serial_write(" func=");
    serial_write_uint(device->function);
    serial_write(" vendor=0x");
    serial_write_hex16(device->vendor_id);
    serial_write(" device=0x");
    serial_write_hex16(device->device_id);
    serial_write("\n");
}

static void log_port_summary(uint8_t port, const struct usb_port_summary* summary) {
    serial_write("usb:   port ");
    serial_write_uint(port);
    if (!summary->connected) {
        serial_write(": disconnected\n");
        return;
    }

    serial_write(": ");
    serial_write(summary->low_speed ? "low-speed" : "full-speed");
    if (!summary->enumerated) {
        serial_write(" device enumeration failed\n");
        return;
    }

    serial_write(" addr=");
    serial_write_uint(summary->address);
    serial_write(" vendor=0x");
    serial_write_hex16(summary->descriptor.idVendor);
    serial_write(" product=0x");
    serial_write_hex16(summary->descriptor.idProduct);
    serial_write(" class=0x");
    serial_write_hex8(summary->descriptor.bDeviceClass);
    serial_write(" subclass=0x");
    serial_write_hex8(summary->descriptor.bDeviceSubClass);
    serial_write(" protocol=0x");
    serial_write_hex8(summary->descriptor.bDeviceProtocol);
    serial_write("\n");
}

static bool enumerate_controller(size_t index, const struct pci_device* device) {
    if (g_controllers_used >= USB_MAX_CONTROLLERS) {
        serial_write("usb: controller table full, skipping enumeration\n");
        return false;
    }

    struct usb_controller_summary* summary = &g_controllers[g_controllers_used];
    memset(summary, 0, sizeof(*summary));
    summary->pci = *device;

    log_controller_header(index, device);

    if (!uhci_controller_init(device)) {
        serial_write("usb:   failed to initialize UHCI controller\n");
        ++g_controllers_used;
        return false;
    }

    struct uhci_controller* controller = uhci_controller_get();
    if (!controller) {
        serial_write("usb:   controller handle unavailable\n");
        ++g_controllers_used;
        return false;
    }

    summary->initialized = true;
    summary->port_count = uhci_port_count(controller);

    uint8_t next_address = USB_ADDRESS_START;
    for (uint8_t port = 0; port < summary->port_count; ++port) {
        struct usb_port_summary* port_summary = &summary->ports[port];
        port_summary->connected = uhci_port_connected(controller, port);
        port_summary->low_speed = uhci_port_low_speed(controller, port);
        port_summary->enumerated = false;
        port_summary->address = 0;
        memset(&port_summary->descriptor, 0, sizeof(port_summary->descriptor));

        if (!port_summary->connected) {
            log_port_summary(port, port_summary);
            continue;
        }

        if (next_address >= 127) {
            serial_write("usb:   address pool exhausted\n");
            log_port_summary(port, port_summary);
            continue;
        }

        uint8_t assigned_address = next_address++;
        if (enumerate_device_on_port(controller, port, assigned_address, &port_summary->descriptor)) {
            port_summary->enumerated = true;
            port_summary->address = assigned_address;
        }

        log_port_summary(port, port_summary);
    }

    ++g_controllers_used;
    return true;
}

bool usb_init(void) {
    reset_summaries();

    serial_write("usb: init begin\n");

    struct pci_device controllers[USB_MAX_CONTROLLERS];
    serial_write("usb: querying PCI for UHCI controllers\n");
    size_t total = pci_enumerate_by_class(0x0C, 0x03, 0x00, controllers, USB_MAX_CONTROLLERS);
    g_controllers_detected = total;

    serial_write("usb: detected ");
    serial_write_uint((uint32_t)total);
    serial_write(" UHCI controller(s)\n");

    size_t enumerated = total;
    if (enumerated > USB_MAX_CONTROLLERS) {
        enumerated = USB_MAX_CONTROLLERS;
        serial_write("usb: limited enumeration to first ");
        serial_write_uint((uint32_t)enumerated);
        serial_write(" controllers\n");
    }

    bool any_initialized = false;
    for (size_t index = 0; index < enumerated; ++index) {
        if (enumerate_controller(index, &controllers[index])) {
            any_initialized = true;
        }
    }

    g_usb_ready = true;
    if (total == 0) {
        return true;
    }
    return any_initialized;
}

size_t usb_bus_count(void) {
    return g_usb_ready ? g_controllers_detected : 0;
}
