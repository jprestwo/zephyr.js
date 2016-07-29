#include <zephyr.h>
#include <flash.h>
#include <device.h>

#include "zjs_util.h"
#include "zjs_flash.h"

#define FLASH_START_OFFSET      0x00
#define FLASH_SECTOR_SIZE       4096
// Magic number signifying the start of the flash file table
#define FLASH_START_MAGIC       0x11223344
#define FLASH_MAX_OFFSET        16

#define BIT_IS_SET(i, b)  ((i) & (1 << (b)))
#define BIT_SET(i, b)     ((i) |= (1 << (b)))
#define BIT_CLR(i, b)     ((i) &= ~(1 << (b)))

static int find_id(uint64_t* use)
{
    int id = 1;
    while (BIT_IS_SET(*use, id)) {
        ++id;
    }
    BIT_SET(*use, id);
    return id;
}


static uint64_t sector_use;
static uint32_t swap_sec = 0;
static struct device *flash_dev;

struct file_table_entry {
    uint32_t offset;
    char name[16];
    struct file_table_entry* next;
};

struct file_table_entry* file_list = NULL;

static uint8_t can_write(uint32_t offset, uint32_t size)
{
    uint32_t count = 0;
    uint32_t chunk;

    while (count < size) {
        if (flash_read(flash_dev, offset + count, &chunk, 4) != 0) {
            PRINT("[flash] can_write(): Flash read failed!\n");
            return 0;
        }
        if (chunk != 0xffffffff) {
            PRINT("Chunk wrong: %x\n", chunk);
            return 0;
        }
        PRINT("chunk: %x\n", chunk);
        count += 4;
    }
    return 1;
}

static void copy_to_sector(uint32_t from, uint32_t to, uint32_t start, uint32_t end)
{
    uint8_t buf[128];
    uint32_t count = 0;
    while (count < FLASH_SECTOR_SIZE) {
        if (flash_read(flash_dev, from + count, buf, 128) != 0) {
            PRINT("[flash] flash_init(): Flash read failed!\n");
            return;
        }
        flash_write_protection_set(flash_dev, false);
        if (flash_write(flash_dev, to + count, buf, 128) != 0) {
            PRINT("[flash] flash_write(): Flash write failed!\n");
            return;
        }
        count += 128;
    }
    flash_write_protection_set(flash_dev, false);
    if (flash_erase(flash_dev, from, FLASH_SECTOR_SIZE) != 0) {
        PRINT("[flash] flash_init(): Flash erase failed!\n");
        return;
    }
    count = 0;
    while (count < FLASH_SECTOR_SIZE) {
        if (count < start || count > end) {
            uint32_t chunk;
            if (flash_read(flash_dev, to + count, &chunk, 4) != 0) {
                PRINT("[flash] flash_init(): Flash read failed!\n");
                return;
            }
            flash_write_protection_set(flash_dev, false);
            if (flash_write(flash_dev, from + count, &chunk, 4) != 0) {
                PRINT("[flash] flash_write(): Flash write failed!\n");
                return;
            }
        }
        count += 4;
    }
}

// Load the flash file table into RAM, if one does not exist, create it.
static void flash_init(void)
{
    uint32_t magic;
    if (flash_read(flash_dev, FLASH_START_OFFSET, &magic, 4) != 0) {
        PRINT("[flash] flash_init(): Flash read failed!\n");
        return;
    }
    if (magic != FLASH_START_MAGIC) {
        PRINT("File table not initialized\n");

        flash_write_protection_set(flash_dev, false);
        if (flash_erase(flash_dev, FLASH_START_OFFSET, FLASH_SECTOR_SIZE) != 0) {
            PRINT("[flash] flash_init(): Flash erase failed!\n");
            return;
        }
        flash_write_protection_set(flash_dev, false);

        magic = FLASH_START_MAGIC;
        // Write magic start of flash number
        if (flash_write(flash_dev, FLASH_START_OFFSET, &magic, 4) != 0) {
            PRINT("[flash] flash_init(): Flash write failed!\n");
            return;
        }
    } else {
        PRINT("File table intialized\n");
        // Start populating file list
        uint32_t count = 1;
        while (1) {
            uint32_t test;
            if (flash_read(flash_dev, FLASH_START_OFFSET + (count * FLASH_SECTOR_SIZE), &test, 4) != 0) {
                PRINT("[flash] flash_init(): Flash read failed!\n");
                return;
            }
            if (test == 0xffffffff) {
                PRINT("End of files\n");
                break;
            } else {
                struct file_table_entry* new = task_malloc(sizeof(struct file_table_entry));
                new->offset = (count * FLASH_SECTOR_SIZE);
                if (flash_read(flash_dev, FLASH_START_OFFSET + (count * FLASH_SECTOR_SIZE), new->name, 16) != 0) {
                    PRINT("[flash] flash_init(): Flash read failed!\n");
                    return;
                }
                if (file_list) {
                    new->next = file_list;
                    file_list = new;
                } else {
                    new->next = NULL;
                    file_list = new;
                }
                PRINT("File found, added to list\n");
                BIT_SET(sector_use, count);
            }
            count++;
        }
    }
}

static jerry_value_t fs_write(const jerry_value_t function_obj_val,
                              const jerry_value_t this_val,
                              const jerry_value_t args_p[],
                              const jerry_length_t args_cnt)
{
    uint32_t fd = jerry_get_number_value(args_p[0]);
    uint32_t offset = jerry_get_number_value(args_p[2]);
    uint32_t buf_sz = jerry_get_number_value(args_p[3]);
    char buf[buf_sz];
    jerry_string_to_char_buffer(args_p[1], buf, buf_sz);

    if (!can_write(fd + 16 + offset, buf_sz)) {
        PRINT("Must erase before writing\n");
        uint32_t tmp_sec = find_id(&sector_use);
        copy_to_sector(fd, tmp_sec, offset, offset + buf_sz);
    }
    flash_write_protection_set(flash_dev, false);
    if (flash_write(flash_dev, fd + 16 + offset, buf, buf_sz) != 0) {
        PRINT("[flash] flash_write(): Flash write failed!\n");
        return ZJS_UNDEFINED;
    }
}

static jerry_value_t fs_read(const jerry_value_t function_obj_val,
                             const jerry_value_t this_val,
                             const jerry_value_t args_p[],
                             const jerry_length_t args_cnt)
{
    uint32_t fd = jerry_get_number_value(args_p[0]);
    uint32_t offset = jerry_get_number_value(args_p[2]);
    uint32_t buf_sz = jerry_get_number_value(args_p[3]);
    char buf[buf_sz];
    jerry_string_to_char_buffer(args_p[1], buf, buf_sz);

    if (flash_read(flash_dev, fd + 16 + offset, buf, buf_sz) != 0) {
        PRINT("[flash] flash_read(): Flash read failed!\n");
        return ZJS_UNDEFINED;
    }
    return jerry_create_string(buf);
}

static jerry_value_t fs_open(const jerry_value_t function_obj_val,
                             const jerry_value_t this_val,
                             const jerry_value_t args_p[],
                             const jerry_length_t args_cnt)
{
    uint32_t offset = 0;
    char name[16] = {0};
    int wlen = jerry_string_to_char_buffer(args_p[0], name, 16);
    PRINT("Opening file %s\n", name);
    struct file_table_entry* cur = file_list;
    while (cur) {
        if (strcmp(name, cur->name) == 0) {
            PRINT("FOUND\n");
            return jerry_create_number((double)cur->offset);
        }
        cur = cur->next;
    }
    uint32_t new_sec = find_id(&sector_use);
    PRINT("New sector file %u\n", new_sec);
    flash_write_protection_set(flash_dev, false);
    if (flash_erase(flash_dev, FLASH_START_OFFSET + (new_sec * FLASH_SECTOR_SIZE), FLASH_SECTOR_SIZE) != 0) {
        PRINT("[flash] flash_open(): Flash erase failed!\n");
        return ZJS_UNDEFINED;
    }
    flash_write_protection_set(flash_dev, false);
    if (flash_write(flash_dev, FLASH_START_OFFSET + (new_sec * FLASH_SECTOR_SIZE), name, 16) != 0) {
        PRINT("[flash] flash_open(): Flash write file name failed\n");
        return ZJS_UNDEFINED;
    }
    struct file_table_entry* new = task_malloc(sizeof(struct file_table_entry));
    new->offset = (new_sec * FLASH_SECTOR_SIZE);
    if (file_list) {
        new->next = file_list;
        file_list = new;
    } else {
        new->next = NULL;
        file_list = new;
    }

    return jerry_create_number((double)new->offset);;
}

jerry_value_t zjs_flash_init(void)
{
    flash_dev = device_get_binding("W25QXXDV");
    if (!flash_dev) {
        PRINT("zjs_flash_init: cannot find W25QXXDV device\n");
        return zjs_error("zjs_flash_init: cannot find W25QXXDV device");
    }

    flash_init();

    // create flash object
    jerry_value_t flash_obj = jerry_create_object();
    zjs_obj_add_function(flash_obj, fs_open, "open");
    zjs_obj_add_function(flash_obj, fs_read, "read");
    zjs_obj_add_function(flash_obj, fs_write, "write");
    return flash_obj;
}

