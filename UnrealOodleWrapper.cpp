#define OODLE_ALLOW_DEPRECATED_COMPRESSORS

#include <iostream>
#include <fcntl.h>
#include <io.h>
#include "include\oodle2.h"

int printUsage() {
    std::cerr << ".exe [option] file_path/stdin=%d output_file_path/stdout\n";
    std::cerr << "Option:\n";
    std::cerr << "-c %d %d:\t\t\tcompress file\n\t\t\t\t1st %d: compression level (from -4 to 9)\n\t\t\t\t2nd %d: compression method (from -1 to 5)\n";
    std::cerr << "-d %d:\t\t\t\tdecompress file\n\t\t\t\t%d: exact decompressed file size\n\t\t\t\t(0 is not accepted, wrong value will return error)\n\n";
    std::cerr << "Other:\n";
    std::cerr << "file_path/stdin=%d:\t\tyou can either provide input file path,\n\t\t\t\tor use stdin by writing \"stdin=%d\",\n\t\t\t\twhere %d is size of stdin binary data\n";
    std::cerr << "output_file_path/stdout:\tyou can either provide input file path,\n\t\t\t\tor use stdout by writing \"stdout\"\n\n";
    std::cerr << "Compression Levels:\n";
    std::cerr << "<0 = compression ratio < speed\n";
    std::cerr << "=0 = No compression\n";
    std::cerr << ">0 = compression ratio > speed\n\n";
    std::cerr << "Compression Methods:\n";
    std::cerr << "-1 = LZB16 (DEPRECATED but still supported)\n";
    std::cerr << "0 = None (memcpy, pass through uncompressed bytes)\n";
    std::cerr << "1 = Kraken (Fast decompression and high compression ratios)\n";
    std::cerr << "2 = Leviathan (Higher compression than Kraken, slightly slower decompression.)\n";
    std::cerr << "3 = Mermaid (between Kraken & Selkie - crazy fast, still decent compression.)\n";
    std::cerr << "4 = Selkie (Selkie is a super-fast relative of Mermaid. For maximum decode speed.)\n";
    std::cerr << "5 = Hydra\n\n";
    return 1;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        return printUsage();
    long compression_level = 9;
    bool compress = false;
    char* p = 0;
    long decompress_size = 0;
    long int compression_method = 3;
    int addarg = 0;

    if (!strncmp(argv[1], "-c", 2)) {
        if (argc != 6) {
            return printUsage();
        }
        errno = 0;
        compress = true;
        compression_level = strtol(argv[2], &p, 10);
        if (errno != 0 || *p != '\0' || compression_level > 9 || compression_level < -4) {
            std::cerr << "Wrong compression level input!\n";
            return printUsage();
        }
        compression_method = strtol(argv[3], &p, 10);
        if (errno != 0 || *p != '\0' || compression_method > 5 || compression_method < -1) {
            std::cerr << "Wrong compression method input!\n";
            return printUsage();
        }
        switch (compression_method) {
            case -1:
                compression_method = 4;
                break;
            case 0:
                compression_method = 3;
                break;
            case 1:
                compression_method = 8;
                break;
            case 2:
                compression_method = 13;
                break;
            case 3:
                compression_method = 9;
                break;
            case 4:
                compression_method = 11;
                break;
            case 5:
                compression_method = 12;
        }
        addarg = 1;
    }
    else if (strncmp(argv[1], "-d", 2)) {
        std::cerr << "Wrong option!\n";
        return printUsage();
    }
    else {
        if (argc != 5) {
            return printUsage();
        }
        errno = 0;
        decompress_size = strtol(argv[2], &p, 10);
        if (errno != 0 || *p != '\0' || decompress_size > INT_MAX || decompress_size < 1) {
            std::cerr << "Wrong decompress size input!\n";
            return printUsage();
        }
    }

    FILE* file = 0;
    size_t filesize = 0;
    void* buffer = 0;

    if (!strncmp(argv[3+addarg], "stdin", 5)) {
        filesize = strtol(argv[3+addarg] + 6, &p, 10);
        buffer = malloc(filesize);
        _setmode(_fileno(stdin), _O_BINARY);
        fread(buffer, filesize, 1, stdin);
    }
    else {
        fopen_s(&file, argv[3+addarg], "rb");
        fseek(file, 0, 2);
        filesize = ftell(file);
        fseek(file, 0, 0);
        buffer = malloc(filesize);
        fread(buffer, filesize, 1, file);
        fclose(file);
    }

    if (!compress) {
        void* new_buffer = malloc(decompress_size);
        size_t dec_size = OodleLZ_Decompress(buffer, filesize, new_buffer, decompress_size, OodleLZ_FuzzSafe_Yes, OodleLZ_CheckCRC_No, OodleLZ_Verbosity_Lots);
        if (!dec_size) {
            std::cerr << "Error while decompressing. Invalid data or provided wrong decompression size!\n";
            return printUsage();
        }
        if (!strncmp(argv[4], "stdout", 6) && new_buffer) {
            fflush(stdout);
            _setmode(_fileno(stdout), _O_BINARY);
            fwrite(new_buffer, decompress_size, 1, stdout);
        }
        else {
            FILE* newfile;
            fopen_s(&newfile, argv[4], "wb");
            if (newfile && new_buffer) {
                fwrite(new_buffer, decompress_size, 1, newfile);
                fclose(newfile);
            }
            free(new_buffer);
            printf("Finished decompression. Compression ratio: %.2f%%\nDecompressed file size: %zd B\n", (float(filesize) / decompress_size) * 100, dec_size);
        }
        return 0;
    }
    else {
        size_t comp_buffer_size = OodleLZ_GetCompressedBufferSizeNeeded(OodleLZ_Compressor(compression_method), filesize);
        void* comp_buffer = malloc(comp_buffer_size);
        size_t com_size = OodleLZ_Compress(OodleLZ_Compressor(compression_method), buffer, filesize, comp_buffer, OodleLZ_CompressionLevel(compression_level));
        if (!com_size) {
            std::cerr << "Error while compressing!\n";
            return 1;
        }
        if (!strncmp(argv[5], "stdout", 6)) {
            fflush(stdout);
            _setmode(_fileno(stdout), _O_BINARY);
            fwrite(comp_buffer, com_size, 1, stdout);
        }
        else {
            FILE* newfile;
            fopen_s(&newfile, argv[5], "wb");
            fwrite(comp_buffer, com_size, 1, newfile);
            fclose(newfile);
            free(comp_buffer);
            printf("Finished compression. Compression ratio: %.2f%%\n", (float(com_size) / filesize) * 100);
        }
        return 0;
    }
}
