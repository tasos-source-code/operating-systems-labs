// fs_testbench.c
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BLOCK_SIZE 4096

static void die(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

static void make_path(char *out, size_t out_size, const char *dir, const char *file)
{
    int n = snprintf(out, out_size, "%s/%s", dir, file);
    if (n < 0 || (size_t)n >= out_size) {
        fprintf(stderr, "Path too long\n");
        exit(EXIT_FAILURE);
    }
}

static void fill_pattern(uint8_t *buf, size_t size, uint8_t seed)
{
    for (size_t i = 0; i < size; i++) {
        buf[i] = (uint8_t)(seed + i);
    }
}

static int verify_pattern(uint8_t *buf, size_t size, uint8_t seed)
{
    for (size_t i = 0; i < size; i++) {
        uint8_t expected = (uint8_t)(seed + i);
        if (buf[i] != expected) {
            fprintf(stderr,
                    "Mismatch at byte %zu: got 0x%02x expected 0x%02x\n",
                    i, buf[i], expected);
            return -1;
        }
    }
    return 0;
}

static void build_mixed_buffer(uint8_t *buf, int blocks, uint8_t old_seed, uint8_t new_seed)
{
    for (int b = 0; b < blocks; b++) {
        uint8_t *block = buf + b * BLOCK_SIZE;

        /*
         * Blocks 0, 3, 6, ... stay the same between writes.
         * The rest change.
         */
        if (b % 3 == 0) {
            fill_pattern(block, BLOCK_SIZE, old_seed + b);
        } else {
            fill_pattern(block, BLOCK_SIZE, new_seed + b);
        }
    }
}

static void write_full_file(const char *path, int blocks, uint8_t old_seed, uint8_t new_seed)
{
    size_t size = (size_t)BLOCK_SIZE * blocks;

    uint8_t *buf = malloc(size);
    if (!buf) die("malloc");

    build_mixed_buffer(buf, blocks, old_seed, new_seed);

    int fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd < 0) die("open write");

    size_t total = 0;
    while (total < size) {
        ssize_t written = write(fd, buf + total, size - total);
        if (written < 0) die("write");

        if (written == 0) {
            fprintf(stderr, "write returned 0\n");
            exit(EXIT_FAILURE);
        }

        total += written;
    }

    if (fsync(fd) < 0) die("fsync");

    close(fd);
    free(buf);

    printf("WROTE  %s  size=%zu bytes (%d blocks)\n", path, size, blocks);
}

static void read_and_verify_file(const char *path, int blocks,
                                 uint8_t old_seed, uint8_t new_seed)
{
    size_t size = (size_t)BLOCK_SIZE * blocks;

    uint8_t *buf = malloc(size);
    uint8_t *expected = malloc(size);

    if (!buf || !expected) die("malloc");

    build_mixed_buffer(expected, blocks, old_seed, new_seed);

    int fd = open(path, O_RDONLY);
    if (fd < 0) die("open read");

    size_t total = 0;
    while (total < size) {
        ssize_t r = read(fd, buf + total, size - total);
        if (r < 0) die("read");

        if (r == 0) {
            fprintf(stderr, "Unexpected EOF: read %zu / %zu\n", total, size);
            exit(EXIT_FAILURE);
        }

        total += r;
    }

    if (memcmp(buf, expected, size) == 0) {
        printf("VERIFY OK  %s\n", path);
    } else {
        fprintf(stderr, "VERIFY FAILED  %s\n", path);
        printf("expected\t %s\n",expected);
        printf("buf \t%s\n", buf);

        exit(EXIT_FAILURE);
    }

    close(fd);
    free(buf);
    free(expected);
}

static void overwrite_one_block(const char *path, int block_index, uint8_t seed)
{
    uint8_t buf[BLOCK_SIZE];
    fill_pattern(buf, BLOCK_SIZE, seed);

    int fd = open(path, O_WRONLY);
    if (fd < 0) die("open overwrite");

    off_t offset = (off_t)block_index * BLOCK_SIZE;

    if (lseek(fd, offset, SEEK_SET) < 0) die("lseek");

    size_t total = 0;
    while (total < BLOCK_SIZE) {
        ssize_t written = write(fd, buf + total, BLOCK_SIZE - total);
        if (written < 0) die("write overwrite");

        if (written == 0) {
            fprintf(stderr, "write returned 0\n");
            exit(EXIT_FAILURE);
        }

        total += written;
    }

    if (fsync(fd) < 0) die("fsync overwrite");

    close(fd);

    printf("OVERWROTE block %d of %s\n", block_index, path);
}

static void verify_one_block(const char *path, int block_index, uint8_t seed)
{
    uint8_t buf[BLOCK_SIZE];

    int fd = open(path, O_RDONLY);
    if (fd < 0) die("open verify block");

    off_t offset = (off_t)block_index * BLOCK_SIZE;

    if (lseek(fd, offset, SEEK_SET) < 0) die("lseek verify block");
    size_t total = 0;
    while (total < BLOCK_SIZE) {
        ssize_t r = read(fd, buf + total, BLOCK_SIZE - total);
        if (r < 0) die("read verify block");

        if (r == 0) {
            fprintf(stderr, "Unexpected EOF while reading block\n");
            exit(EXIT_FAILURE);
        }

        total += r;
    }

    if (verify_pattern(buf, BLOCK_SIZE, seed) == 0) {
        printf("VERIFY OK overwritten block %d of %s\n", block_index, path);
    } else {
        fprintf(stderr, "VERIFY FAILED overwritten block\n");
        exit(EXIT_FAILURE);
    }

    close(fd);
}

static void truncate_file_test(const char *path, off_t new_size)
{
    if (truncate(path, new_size) < 0) {
        die("truncate");
    }

    struct stat st;
    if (stat(path, &st) < 0) {
        die("stat after truncate");
    }

    if (st.st_size != new_size) {
        fprintf(stderr, "TRUNCATE FAILED %s: size is %lld, expected %lld\n", path, (long long)st.st_size, (long long)new_size);
        exit(EXIT_FAILURE);
    }
    printf("TRUNCATE OK  %s  new_size=%lld bytes\n", path, (long long)new_size);
}

static void verify_zero_region(const char *path, off_t offset, size_t size)
{
    uint8_t *buf = malloc(size);
    if (!buf) die("malloc zero verify");

    int fd = open(path, O_RDONLY);
    if (fd < 0) die("open verify zero region");

    if (lseek(fd, offset, SEEK_SET) < 0) {
        die("lseek verify zero region");
    }

    size_t total = 0;
    while (total < size) {
        ssize_t r = read(fd, buf + total, size - total);
        if (r < 0) die("read verify zero region");

        if (r == 0) {
            fprintf(stderr, "Unexpected EOF while verifying zero region\n");
            exit(EXIT_FAILURE);
        }

        total += r;
    }

    for (size_t i = 0; i < size; i++) {
        if (buf[i] != 0) {
            fprintf(stderr, "ZERO VERIFY FAILED at byte %zu: got 0x%02x expected 0x00\n", i, buf[i]);
            exit(EXIT_FAILURE);
        }
    }

    close(fd);
    free(buf);

    printf("VERIFY OK zero region  %s  offset=%lld size=%zu\n", path, (long long)offset, size);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *dir = argv[1];

    char file1[1024];
    char file2[1024];
    char file3[1024];
    char testdir[1024];
    int ret;

    make_path(testdir, sizeof(testdir), dir, "testdir");

    if (mkdir(testdir, 0755) < 0 && errno != EEXIST) {
        die("mkdir testdir");
    }
    printf("CREATED DIRECTORY: %s\n", testdir);

    make_path(file1, sizeof(file1), dir, "testdir/test_file_1.bin");
    make_path(file2, sizeof(file2), dir, "test_file_2.bin");
    make_path(file3, sizeof(file3), dir, "test_file_3.bin");

    printf("Filesystem testbench path: %s\n\n", dir);

    /*
     * First write.
     */
    write_full_file(file1, 1, 0x10, 0x20);
    write_full_file(file2, 4, 0x30, 0x40);
    write_full_file(file3, 8, 0x50, 0x60);

    printf("\n");

    read_and_verify_file(file1, 1, 0x10, 0x20);
    read_and_verify_file(file2, 4, 0x30, 0x40);
    read_and_verify_file(file3, 8, 0x50, 0x60);
    printf("\n");
    /*
     * Second write.
     * Blocks 0, 3, 6, ... remain same because old_seed is same.
     * Other blocks change because new_seed changes.
     */
    write_full_file(file2, 2, 0x30, 0x99);
    read_and_verify_file(file2, 2, 0x30, 0x99);
    printf("\n");

    /*
     * Overwrite one block inside file2.
     */
    overwrite_one_block(file2, 2, 0xAA);
    verify_one_block(file2, 2, 0xAA);

     /*
     * Truncate tests.
     */
    printf("\n");
    //Shrink file3 from 8 blocks to 3 blocks.
    truncate_file_test(file3, 3 * BLOCK_SIZE);

    /*
     * After shrinking, the first 3 blocks should still contain
     * the original expected data.
     */
    read_and_verify_file(file3, 3, 0x50, 0x60);
    printf("\n");

    /*
     * Extend file3 from 3 blocks to 6 blocks.
     * The newly created area must read as zero bytes.
     */
    truncate_file_test(file3, 6 * BLOCK_SIZE);
    /*
     * Verify the old part still exists.
     */
    read_and_verify_file(file3, 3, 0x50, 0x60);
    /*
     * Verify the extended part is zero-filled.
     */
    verify_zero_region(file3, 3 * BLOCK_SIZE, 3 * BLOCK_SIZE);
    printf("\n");

    if (unlink(file1) < 0) die("unlink file1");
    printf("DELETED %s\n", file1);
    if (unlink(file2) < 0) die("unlink file2");
    printf("DELETED %s\n", file2);
    if (unlink(file3) < 0) die("unlink file3");
    printf("DELETED %s\n", file3);

    ret = system("rm -rf fs/mountdir/testdir");
    if (ret < 0) {
        perror("system");
        return 1;
    }
    printf("DELETED FOLDER: mountdir/testdir\n");

    printf("\nFilesystem testbench completed successfully.\n");

    return EXIT_SUCCESS;
}