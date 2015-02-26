# This Software is part of Wind River Simics. The rights to copy, distribute,
# modify, or otherwise make use of this Software may be licensed only
# pursuant to the terms of an applicable Wind River license agreement.
# 
# Copyright 2010-2014 Intel Corporation

from cli import *
from simics import *
import cli_impl
import sim_commands
import struct
import binascii

header = dict(
    version = 0,        # Chip16 specification version
    rom_size = 0,       # size of following header memory in bytes
    pc_start_addr = 0,  # initial pc value
    checksum = 0        # CRC32 value as a string of hexadecimal number
    )

def load_binary(file_path):

    bin_file = open(file_path, 'rb')
    memory_max_size = 64*1024;

    if '.c16' in file_path:
        header_flag = 1
    elif '.bin' in file_path:
        header_flag = 0
    else:
        bin_file.close()
        raise CliError("Incorrect format. Try .c16, .bin extension files")

    if (header_flag):

        temp = bin_file.read(5)
        header['version'] = ord(bin_file.read(1))
        rom = reduce(lambda rst, d:rst*10+d, struct.unpack("<L", bin_file.read(4)))
        header['rom_size'] = rom

        if (rom > memory_max_size):
            bin_file.close()
            raise CliError("Not enough memory")

        header['pc_start_addr'] = reduce(lambda rst, d:rst*10+d, struct.unpack("<H", bin_file.read(2)))
        header['checksum'] = reduce(lambda rst, d:rst*10+d, struct.unpack("<L", bin_file.read(4)))

        # culculate the checksum and compare it with the
        # given value in header
        crc = binascii.crc32(bin_file.read(rom))
        if (crc & 0xffffffff != header['checksum']):
            bin_file.close()
            raise CliError("File has been damaged. CRC checksum failed")
        # return to the beginning of rom memory
        bin_file.seek(16, 0)

    # copy data into device memory
    offset = 0
    while (offset < rom - 4):
        cli.conf.ram0.image.iface.image.set(0x0 + offset, bin_file.read(4))
        offset = offset + 4

    bin_file.close()

new_command("load_binary", load_binary,
            args = [arg(str_t, "file-path")],
            alias = "ld",
            type = "loading-commands",
            short = "writes from Chip16 binary file to memory",
            doc = """
<b>load-binary</b> is a command to load binary data from Chip16 
binary files (.c16, .bin) into platform memory. 
The only argument <i>file_path</i> is the whole path to the binary file""")
