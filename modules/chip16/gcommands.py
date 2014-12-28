# This Software is part of Wind River Simics. The rights to copy, distribute,
# modify, or otherwise make use of this Software may be licensed only
# pursuant to the terms of an applicable Wind River license agreement.
# 
# Copyright 2010-2014 Intel Corporation

from cli import *
from simics import *
import cli_impl
import sim_commands

header = dict(
    version = '',       # Chip16 spec. version
    rom_size = 0,       # size of following header memory in bytes
    pc_start_addr = 0,  # initial pc value
    checksum = ''       # CRC32 value as a string of hexadecimal number
    )

def read_header(byte_list):
    """
    Reads header info. 
    Fills the header dictionary with corresponding data 

    Arguments:
    byte_list -- list structure of bytes
                (see loading_binary function for details)
    """

    for i in range(5):
        byte = byte_list.pop()

    byte = byte_list.pop()
    header['version'] = '{}.{}'.format(byte[0], byte[1])

    sum_ = 0
    for i in range(0, 8, 2):
        byte = byte_list.pop()
        sum_ = sum_ + int(byte[0], 16) * pow(16, i + 1) \
                    + int(byte[1], 16) * pow(16, i)
    if (sum_ > 64 * 1024):
        raise CliError("Not enough memory")

    header['rom_size'] = sum_

    sum_ = 0
    for k in range(0, 2):
        byte = byte_list.pop()
        sum_ = sum_ + int(byte[0], 16) * pow(16, i + 1) \
                    + int(byte[1], 16) * pow(16, i)
    header['pc_start_addr'] = sum_

    for k in range(0, 4):
        byte = byte_list.pop()
        header['checksum'] = header['checksum'] + byte

def load_binary(file_path):
    
    if '.c16' in file_path:
        header_flag = 1
    elif '.bin' in file_path:
        header_flag = 0
    else: 
        raise CliError("Incorrect file. Try .c16, .bin extension files")

    bin_file = open(file_path, 'rb')

    byte_list = []
    line_list = [line for line in bin_file]

    # creating list: byte_list - strings of the form 'XX'
    # X - hexadecimal digit
    # thus, each element is one byte in hexadecimal representation

    for j in line_list:
        for k in j:
            byte = hex(ord(k))
            if len(byte) == 3:
                byte_line = '0' + byte[2]
            else:
                byte_line = byte[2] + byte[3]
            byte_list.append(byte_line)

    # reverse list in order to use pop() method
    byte_list.reverse()

    if header_flag:
        # after this we have only rom data
        read_header(byte_list)

    # create data_list with instructions in integer form
    # for future write to memory
    data_list = []
    data = ''
    for i in byte_list:
        for k in range(4):
            i = byte_list.pop()
            data = data + i
        data = int(data, 16)
        data_list.append(data)
        data = ''

    bin_file.close()

new_command("load-binary", load_binary,
            args = [arg(str_t, "file-path")],
            alias = "ld",
            type = "loading-commands",
            short = "writes from Chip16 binary file to memory",
            doc = """
<b>load-binary</b> is a command to load binary data from Chip16 
binary files (.c16, .bin) into platform memory. 
Only argument <i>file_path</i> is the whole path to the binary file""")
