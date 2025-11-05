#include "drivers/usb/uhci.h"

#include <stddef.h>
#include "arch/x86_64/kernel.h"
#include "drivers/pci.h"
#include "drivers/usb/usb.h"
#include "libk/string.h"
#include "libs/serial.h"
#include "libs/assembly.h"

#define UHCI_USBCMD 0x00
#define UHCI_USBSTS 0x02
#define UHCI_USBINTR 0x04
#define UHCI_FRNUM 0x06
#define UHCI_FLBASEADD 0x08
#define UHCI_SOFMOD 0x0C
#define UHCI_PORTSC1 0x10
#define UHCI_PORTSC2 0x12

#define UHCI_USBCMD_RUN    (1 << 0)
#define UHCI_USBCMD_HCRESET (1 << 1)
#define UHCI_USBCMD_GRESET (1 << 2)
#define UHCI_USBCMD_CF     (1 << 4)
#define UHCI_USBCMD_MAXP   (1 << 7)

#define UHCI_USBSTS_USBINT (1 << 0)
#define UHCI_USBSTS_ERROR  (1 << 1)
#define UHCI_USBSTS_RD     (1 << 2)
#define UHCI_USBSTS_HSE    (1 << 3)
#define UHCI_USBSTS_HCPE   (1 << 4)
#define UHCI_USBSTS_HCH    (1 << 5)

#define UHCI_PORTSC_CCS   (1 << 0)
#define UHCI_PORTSC_CSC   (1 << 1)
#define UHCI_PORTSC_PE    (1 << 2)
#define UHCI_PORTSC_POEDC (1 << 3)
#define UHCI_PORTSC_LS    (3 << 4)
#define UHCI_PORTSC_RD    (1 << 6)
#define UHCI_PORTSC_LSDA  (1 << 8)
#define UHCI_PORTSC_PR    (1 << 9)
#define UHCI_PORTSC_SUSP  (1 << 12)
#define UHCI_PORTSC_RWC   (UHCI_PORTSC_RD | UHCI_PORTSC_CSC | UHCI_PORTSC_POEDC)

#define UHCI_LINK_TERMINATE 0x00000001
#define UHCI_LINK_QH        0x00000002

#define UHCI_PID_SETUP 0x2D
#define UHCI_PID_IN    0x69
#define UHCI_PID_OUT   0xE1

#define UHCI_TD_ACTIVE        (1 << 23)
#define UHCI_TD_STALLED       (1 << 22)
#define UHCI_TD_DATABUFERR    (1 << 21)
#define UHCI_TD_BABBLE        (1 << 20)
#define UHCI_TD_NAK           (1 << 19)
#define UHCI_TD_TIMEOUT       (1 << 18)
#define UHCI_TD_BITSTUFF      (1 << 17)

#define UHCI_TD_IOC           (1 << 24)
#define UHCI_TD_ISP_1         (1 << 25)
#define UHCI_TD_LS            (1 << 26)
#define UHCI_TD_SPD           (1 << 27)

#define UHCI_PORT_COUNT UHCI_MAX_PORTS

struct uhci_td {
    uint32_t link_ptr;
    uint32_t control;
    uint32_t token;
    uint32_t buffer;
} __attribute__((packed, aligned(16)));

struct uhci_qh {
    uint32_t head_link_ptr;
    uint32_t element_link_ptr;
} __attribute__((packed, aligned(16)));

struct uhci_controller {
    uint16_t io_base;
    uint8_t active_port;
    bool port_connected[UHCI_PORT_COUNT];
    bool port_low_speed[UHCI_PORT_COUNT];
};

static struct uhci_controller g_controller;
static bool controller_ready = false;
static uint32_t frame_list[1024] __attribute__((aligned(4096)));
static struct uhci_qh control_qh __attribute__((aligned(16)));
static struct uhci_qh interrupt_qh __attribute__((aligned(16)));

static inline uintptr_t virt_to_phys(const void* ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    if (addr >= KERNEL_VIRTUAL_START) {
        uintptr_t phys = addr - KERNEL_VIRTUAL_START + KERNEL_PHYSICAL_START;
        serial_write("uhci: virt_to_phys high addr=");
        serial_write_hex32((uint32_t)(addr & 0xFFFFFFFF));
        serial_write(" -> ");
        serial_write_hex32((uint32_t)(phys & 0xFFFFFFFF));
        serial_write("\n");
        return phys;
    }
    serial_write("uhci: virt_to_phys low addr=");
    serial_write_hex32((uint32_t)(addr & 0xFFFFFFFF));
    serial_write("\n");
    return addr;
}

static inline uint16_t io_read16(uint16_t base, uint16_t reg) {
    return inw(base + reg);
}

static inline void io_write16(uint16_t base, uint16_t reg, uint16_t value) {
    outw(base + reg, value);
}

static inline void io_write32(uint16_t base, uint16_t reg, uint32_t value) {
    outl(base + reg, value);
}

static void wait_io_delay(uint32_t iterations) {
    for (uint32_t i = 0; i < iterations; ++i) {
        io_wait();
    }
}

static void uhci_stop(struct uhci_controller* controller) {
    serial_write("uhci: stop controller\n");
    uint16_t cmd = io_read16(controller->io_base, UHCI_USBCMD);
    cmd &= ~UHCI_USBCMD_RUN;
    io_write16(controller->io_base, UHCI_USBCMD, cmd);

    for (uint32_t i = 0; i < 1000; ++i) {
        uint16_t status = io_read16(controller->io_base, UHCI_USBSTS);
        if (status & UHCI_USBSTS_HCH) {
            break;
        }
        wait_io_delay(10);
    }
    serial_write("uhci: stop complete\n");
}

static void uhci_reset(struct uhci_controller* controller) {
    serial_write("uhci: reset controller\n");
    uhci_stop(controller);

    io_write16(controller->io_base, UHCI_USBCMD, UHCI_USBCMD_HCRESET);
    wait_io_delay(1000);

    for (uint32_t i = 0; i < 30000; ++i) {
        if (!(io_read16(controller->io_base, UHCI_USBCMD) & UHCI_USBCMD_HCRESET)) {
            break;
        }
        wait_io_delay(10);
    }

    io_write16(controller->io_base, UHCI_USBSTS, UHCI_USBSTS_USBINT | UHCI_USBSTS_ERROR |
                                                 UHCI_USBSTS_RD | UHCI_USBSTS_HSE |
                                                 UHCI_USBSTS_HCPE | UHCI_USBSTS_HCH);
    serial_write("uhci: reset complete\n");
}

static void uhci_configure_frame_list(void) {
    serial_write("uhci: configure frame list\n");
    memset(frame_list, 0, sizeof(frame_list));
    memset(&control_qh, 0, sizeof(control_qh));
    memset(&interrupt_qh, 0, sizeof(interrupt_qh));

    control_qh.head_link_ptr = virt_to_phys(&interrupt_qh) | UHCI_LINK_QH;
    control_qh.element_link_ptr = UHCI_LINK_TERMINATE;

    interrupt_qh.head_link_ptr = UHCI_LINK_TERMINATE;
    interrupt_qh.element_link_ptr = UHCI_LINK_TERMINATE;

    uintptr_t control_qh_phys = virt_to_phys(&control_qh);
    for (size_t i = 0; i < 1024; ++i) {
        frame_list[i] = (uint32_t)(control_qh_phys | UHCI_LINK_QH);
    }
    serial_write("uhci: frame list configured\n");
}

static bool uhci_reset_port(struct uhci_controller* controller, uint8_t port_index) {
    serial_write("uhci: reset_port index=");
    serial_write_uint(port_index);
    serial_write("\n");
    uint16_t port_reg = port_index == 0 ? UHCI_PORTSC1 : UHCI_PORTSC2;

    uint16_t status = io_read16(controller->io_base, port_reg);
    io_write16(controller->io_base, port_reg, status | UHCI_PORTSC_RWC);
    serial_write("uhci: port status after RWC=");
    serial_write_hex16(io_read16(controller->io_base, port_reg));
    serial_write("\n");

    io_write16(controller->io_base, port_reg, status | UHCI_PORTSC_PR);
    wait_io_delay(2000);
    serial_write("uhci: port reset signal asserted\n");

    status = io_read16(controller->io_base, port_reg);
    io_write16(controller->io_base, port_reg, status & ~UHCI_PORTSC_PR);
    wait_io_delay(2000);
    serial_write("uhci: port reset signal cleared\n");

    status = io_read16(controller->io_base, port_reg);
    io_write16(controller->io_base, port_reg, status | UHCI_PORTSC_PE);
    serial_write("uhci: port enabled\n");

    for (uint32_t i = 0; i < 100000; ++i) {
        status = io_read16(controller->io_base, port_reg);
        if (status & UHCI_PORTSC_PE) {
            bool connected = (status & UHCI_PORTSC_CCS) != 0;
            uint16_t ls = (status & UHCI_PORTSC_LS) >> 4;
            controller->port_connected[port_index] = connected;
            controller->port_low_speed[port_index] = (ls == 0x1);
            serial_write("uhci: port status connected=");
            serial_write(connected ? "yes" : "no");
            serial_write(" low_speed=");
            serial_write(controller->port_low_speed[port_index] ? "yes\n" : "no\n");
            return connected;
        }
        wait_io_delay(10);
    }

    controller->port_connected[port_index] = false;
    controller->port_low_speed[port_index] = false;
    serial_write("uhci: port reset failed (timeout)\n");
    return false;
}

static uint32_t build_token(uint8_t pid, uint8_t device_address, uint8_t endpoint, uint8_t data_toggle, uint16_t length) {
    uint32_t token = pid;
    token |= ((uint32_t)(device_address & 0x7F)) << 8;
    token |= ((uint32_t)(endpoint & 0x0F)) << 15;
    token |= ((uint32_t)(data_toggle & 0x01)) << 19;

    uint16_t max_len = length ? (uint16_t)(length - 1) : 0x7FF;
    token |= ((uint32_t)max_len & 0x7FF) << 21;
    return token;
}

static uint32_t td_base_control(bool low_speed) {
    uint32_t control = UHCI_TD_ACTIVE | UHCI_TD_ISP_1;
    if (low_speed) {
        control |= UHCI_TD_LS;
    }
    return control;
}

static bool td_status_ok(uint32_t control) {
    if (control & UHCI_TD_ACTIVE) {
        return false;
    }
    if (control & (UHCI_TD_STALLED | UHCI_TD_DATABUFERR | UHCI_TD_BABBLE |
                   UHCI_TD_TIMEOUT | UHCI_TD_BITSTUFF)) {
        return false;
    }
    return true;
}

static bool uhci_wait_for_completion(struct uhci_td* td, uint32_t timeout_iterations) {
    serial_write("uhci: wait_for_completion td=");
    serial_write_hex32((uint32_t)(uintptr_t)td);
    serial_write(" timeout=");
    serial_write_uint(timeout_iterations);
    serial_write("\n");
    for (uint32_t i = 0; i < timeout_iterations; ++i) {
        if (!(td->control & UHCI_TD_ACTIVE)) {
            bool ok = td_status_ok(td->control);
            serial_write(ok ? "uhci: td completed successfully\n"
                            : "uhci: td completed with error\n");
            return ok;
        }
        wait_io_delay(10);
    }
    serial_write("uhci: wait_for_completion timeout\n");
    return false;
}

static void qh_link_td(struct uhci_qh* qh, struct uhci_td* td) {
    qh->element_link_ptr = td ? (uint32_t)virt_to_phys(td) : UHCI_LINK_TERMINATE;
}

struct uhci_controller* uhci_controller_get(void) {
    return controller_ready ? &g_controller : NULL;
}

bool uhci_controller_init(const struct pci_device* device) {
    serial_write("uhci_controller_init: start\n");
    if (!device) {
        serial_write("uhci_controller_init: invalid device pointer\n");
        return false;
    }

    memset(&g_controller, 0, sizeof(g_controller));
    controller_ready = false;

    uint16_t command = pci_config_read16(device->bus, device->slot, device->function, 0x04);
    command |= (1 << 0); // I/O space
    command |= (1 << 2); // Bus master
    pci_config_write16(device->bus, device->slot, device->function, 0x04, command);

    uint16_t io_base = pci_config_read16(device->bus, device->slot, device->function, 0x20);
    io_base &= ~0x1;
    if (io_base == 0) {
        serial_write("uhci_controller_init: pci config read failed (no resources)\n");
        return false;
    }

    g_controller.io_base = io_base;
    g_controller.active_port = 0;

    uhci_reset(&g_controller);
    serial_write("uhci_controller_init: controller reset\n");
    uhci_configure_frame_list();
    serial_write("uhci_controller_init: frame list configured\n");

    io_write16(g_controller.io_base, UHCI_FRNUM, 0);
    io_write16(g_controller.io_base, UHCI_USBINTR, 0);
    io_write32(g_controller.io_base, UHCI_FLBASEADD, (uint32_t)virt_to_phys(frame_list));
    io_write16(g_controller.io_base, UHCI_SOFMOD, 0x40);

    uint16_t cmd = UHCI_USBCMD_MAXP | UHCI_USBCMD_RUN;
    io_write16(g_controller.io_base, UHCI_USBCMD, cmd);
    io_write16(g_controller.io_base, UHCI_USBCMD, cmd | UHCI_USBCMD_CF);
    serial_write("uhci_controller_init: host controller run\n");

    uint8_t first_connected = 0xFF;
    for (uint8_t port = 0; port < UHCI_PORT_COUNT; ++port) {
        bool connected = uhci_reset_port(&g_controller, port);
        if (connected && first_connected == 0xFF) {
            first_connected = port;
            serial_write("uhci_controller_init: first connected port found\n");
        }
    }

    if (first_connected != 0xFF) {
        g_controller.active_port = first_connected;
        serial_write("uhci_controller_init: controller has an active device\n");
    } else {
        serial_write("uhci_controller_init: no active devices, controller idle\n");
    }

    controller_ready = true;
    return true;
}

void uhci_set_address(struct uhci_controller* controller, uint8_t address) {
    (void)controller;
    (void)address;
    // For UHCI the device address is carried in TD tokens. No global state update needed here.
}

bool uhci_control_transfer(struct uhci_controller* controller, uint8_t address, const struct uhci_setup_data* transfer) {
    serial_write("uhci_control_transfer: address=");
    serial_write_uint(address);
    serial_write(" length=");
    serial_write_uint(transfer ? transfer->length : 0);
    serial_write(" dir=");
    serial_write(transfer && transfer->direction_in ? "IN\n" : "OUT\n");
    if (!controller || !transfer || !transfer->setup) {
        serial_write("uhci_control_transfer: invalid parameters\n");
        return false;
    }

    bool low_speed = false;
    if (controller->active_port < UHCI_PORT_COUNT) {
        low_speed = controller->port_low_speed[controller->active_port];
    }

    struct uhci_td setup_td __attribute__((aligned(16)));
    struct uhci_td data_td __attribute__((aligned(16)));
    struct uhci_td status_td __attribute__((aligned(16)));

    memset(&setup_td, 0, sizeof(setup_td));
    memset(&data_td, 0, sizeof(data_td));
    memset(&status_td, 0, sizeof(status_td));

    setup_td.link_ptr = UHCI_LINK_TERMINATE;
    setup_td.control = td_base_control(low_speed);
    setup_td.token = build_token(UHCI_PID_SETUP, address, 0, 0, sizeof(struct usb_setup_packet));
    setup_td.buffer = (uint32_t)virt_to_phys(transfer->setup);
    setup_td.control |= UHCI_TD_IOC;

    struct uhci_td* last_td = &setup_td;
    struct uhci_td* data_stage = NULL;

    if (transfer->length && transfer->data) {
        data_td.link_ptr = UHCI_LINK_TERMINATE;
        data_td.control = td_base_control(low_speed);
        data_td.token = build_token(transfer->direction_in ? UHCI_PID_IN : UHCI_PID_OUT,
                                    address, 0, 1, transfer->length);
        data_td.buffer = (uint32_t)virt_to_phys(transfer->data);
        if (transfer->direction_in) {
            data_td.control |= UHCI_TD_SPD;
        }
        last_td->link_ptr = (uint32_t)virt_to_phys(&data_td);
        last_td = &data_td;
        data_stage = &data_td;
    }

    status_td.link_ptr = UHCI_LINK_TERMINATE;
    status_td.control = td_base_control(low_speed) | UHCI_TD_SPD;
    status_td.token = build_token(transfer->direction_in ? UHCI_PID_OUT : UHCI_PID_IN,
                                  address, 0, data_stage ? 1 : 1, 0);
    status_td.buffer = 0;
    status_td.control |= UHCI_TD_IOC;
    last_td->link_ptr = (uint32_t)virt_to_phys(&status_td);

    qh_link_td(&control_qh, &setup_td);

    bool ok = uhci_wait_for_completion(&status_td, 200000);
    serial_write(ok ? "uhci_control_transfer: completed successfully\n"
                    : "uhci_control_transfer: completion failed\n");

    control_qh.element_link_ptr = UHCI_LINK_TERMINATE;
    return ok;
}

void uhci_select_port(struct uhci_controller* controller, uint8_t port_index) {
    if (!controller || port_index >= UHCI_PORT_COUNT) {
        serial_write("uhci_select_port: invalid controller or port\n");
        return;
    }

    controller->active_port = port_index;
    serial_write("uhci_select_port: active port set to ");
    serial_write_uint(port_index);
    serial_write("\n");
}

uint8_t uhci_port_count(const struct uhci_controller* controller) {
    (void)controller;
    return UHCI_PORT_COUNT;
}

bool uhci_port_connected(const struct uhci_controller* controller, uint8_t port_index) {
    if (!controller || port_index >= UHCI_PORT_COUNT) {
        return false;
    }
    return controller->port_connected[port_index];
}

bool uhci_port_low_speed(const struct uhci_controller* controller, uint8_t port_index) {
    if (!controller || port_index >= UHCI_PORT_COUNT) {
        return false;
    }
    return controller->port_low_speed[port_index];
}

bool uhci_interrupt_poll(struct uhci_controller* controller, uint8_t address, struct uhci_interrupt_transfer* transfer) {
    serial_write("uhci_interrupt_poll: address=");
    serial_write_uint(address);
    serial_write(" endpoint=");
    serial_write_hex8(transfer ? transfer->endpoint : 0);
    serial_write(" length=");
    serial_write_uint(transfer ? transfer->length : 0);
    serial_write("\n");
    if (!controller || !transfer || !transfer->buffer || !transfer->length) {
        serial_write("uhci_interrupt_poll: invalid parameters\n");
        return false;
    }

    bool low_speed = false;
    if (controller->active_port < UHCI_PORT_COUNT) {
        low_speed = controller->port_low_speed[controller->active_port];
    }

    struct uhci_td interrupt_td __attribute__((aligned(16)));
    memset(&interrupt_td, 0, sizeof(interrupt_td));

    interrupt_td.link_ptr = UHCI_LINK_TERMINATE;
    interrupt_td.control = td_base_control(low_speed);
    interrupt_td.token = build_token(UHCI_PID_IN, address, transfer->endpoint & 0x0F, 1, transfer->length);
    interrupt_td.control |= UHCI_TD_IOC;
    interrupt_td.control |= UHCI_TD_SPD;
    interrupt_td.buffer = (uint32_t)virt_to_phys(transfer->buffer);

    qh_link_td(&interrupt_qh, &interrupt_td);

    bool ok = uhci_wait_for_completion(&interrupt_td, 200000);

    interrupt_qh.element_link_ptr = UHCI_LINK_TERMINATE;
    serial_write(ok ? "uhci_interrupt_poll: success\n"
                    : "uhci_interrupt_poll: failure\n");
    return ok;
}
