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
import os

header = dict(
    version = 0,        # Chip16 specification version
    rom_size = 0,       # size of following header memory in bytes
    pc_start_addr = 0,  # initial pc value
    checksum = 0        # CRC32 value as a string of hexadecimal number
    )

def load_binary(file_path):
    bin_file = open(file_path, 'rb')
    memory_max_size = 64*1024;
    bin_file.seek(0, os.SEEK_END)
    rom = bin_file.tell()
    bin_file.seek(0)

    if file_path.split(".").pop() == 'c16':
        if bin_file.read(4) == 'CH16':
            bin_file.seek(5)
            header['version'] = ord(bin_file.read(1))
            rom = reduce(lambda rst, d:rst*10+d, struct.unpack("<L", bin_file.read(4)))
            header['rom_size'] = rom
            header['pc_start_addr'] = reduce(lambda rst, d:rst*10+d, struct.unpack("<H", bin_file.read(2)))
            header['checksum'] = reduce(lambda rst, d:rst*10+d, struct.unpack("<L", bin_file.read(4)))

            # culculate the checksum and compare it with the
            # given value in header
            crc = binascii.crc32(bin_file.read(rom))
            if (crc & 0xffffffff != header['checksum']):
                bin_file.close()
                raise CliError("File has been damaged. CRC checksum failed")

            # return to the beginning of rom memory
            bin_file.seek(16)
        else:
            # RAW chip16 file
            bin_file.seek(0)
    elif file_path.split(".").pop() == 'bin':
        # RAW chip16 file
        pass
    else:
        bin_file.close()
        raise CliError("Incorrect format. Try .c16, .bin extension files")

    if (rom > memory_max_size):
        bin_file.close()
        raise CliError("Not enough memory")

    # copy data into device memory
    offset = 0
    cpu = current_processor()
    while offset < rom:
#        cli.conf.ram0.image.iface.image.set(0x0 + offset, bin_file.read(4))
        SIM_write_phys_memory(cpu, offset, reduce(lambda rst, d:rst*10+d, struct.unpack("<L", bin_file.read(4))), 4)
        offset = offset + 4

    bin_file.close()

new_command("chip16-load-binary", load_binary,
            args = [arg(str_t, "file-path")],
            alias = "ld",
            type = "loading-commands",
            short = "writes from Chip16 binary file to memory",
            doc = """
<b>chip16-load-binary</b> is a command to load binary data from Chip16
binary files (.c16, .bin) into platform memory.
The only argument <i>file_path</i> is the whole path to the binary file.""")
