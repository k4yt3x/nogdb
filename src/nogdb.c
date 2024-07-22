#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/procfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s {file}\n", argv[0]);
        return 1;
    }

    printf(".: Elf corrupt :.\n");

    int fd;
    if ((fd = open(argv[1], O_RDWR)) < 0) {
        perror("open");
        return 1;
    }

    if (lseek(fd, EI_CLASS, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        close(fd);
        return 1;
    }

    unsigned char class;
    ssize_t bytes_read = read(fd, &class, 1);
    if (bytes_read <= 0) {
        perror("read");
        close(fd);
        return 1;
    }

    // MAP_SHARED is required to actually update the file
    void *map_addr;
    if (class == ELFCLASS32) {
        puts("[*] ELF program is 32-bit.");
        map_addr = mmap(NULL, sizeof(Elf32_Ehdr), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    } else if (class == ELFCLASS64) {
        puts("[*] ELF program is 64-bit.");
        map_addr = mmap(NULL, sizeof(Elf64_Ehdr), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    } else {
        fprintf(stderr, "%s", "Invalid ELF class\n");
        close(fd);
        return 1;
    }

    if (map_addr == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    printf("[*] Current header values:\n");
    if (class == ELFCLASS32) {
        Elf32_Ehdr *header = (Elf32_Ehdr *)map_addr;
        printf(
            "\te_shoff:%d\n\te_shnum:%d\n\te_shstrndx:%d\n",
            header->e_shoff,
            header->e_shnum,
            header->e_shstrndx
        );

        header->e_shoff = 0xffff;
        header->e_shnum = 0xffff;
        header->e_shstrndx = 0xffff;

        printf("[*] Patched header values:\n");
        printf(
            "\te_shoff:%d\n\te_shnum:%d\n\te_shstrndx:%d\n",
            header->e_shoff,
            header->e_shnum,
            header->e_shstrndx
        );
    } else {
        Elf64_Ehdr *header = (Elf64_Ehdr *)map_addr;
        printf(
            "\te_shoff:%lu\n\te_shnum:%d\n\te_shstrndx:%d\n",
            header->e_shoff,
            header->e_shnum,
            header->e_shstrndx
        );

        header->e_shoff = 0xffff;
        header->e_shnum = 0xffff;
        header->e_shstrndx = 0xffff;

        printf("[*] Patched header values:\n");
        printf(
            "\te_shoff:%lu\n\te_shnum:%d\n\te_shstrndx:%d\n",
            header->e_shoff,
            header->e_shnum,
            header->e_shstrndx
        );
    }

    if (msync(NULL, 0, MS_SYNC) == -1) {
        perror("msync");
        close(fd);
        return 1;
    }

    close(fd);
    munmap(map_addr, 0);
    printf("You should not be able to run \"%s\" inside GDB anymore\n", argv[1]);
    return 0;
}
