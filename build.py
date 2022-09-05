import os
import subprocess
import time

files = [   "src/gd32f10x_it.c",
            "src/system_gd32f10x.c",
            "src/gd32f10x_eval.c",
            "src/gd32f10x_usbd_hw.c",
            "src/startup_gd32f10x_md.s",
            "src/main.c",
            "src/systick.c",
            "Firmware/Peripherals/Source/gd32f10x_rcu.c",
            "Firmware/Peripherals/Source/gd32f10x_gpio.c",
            "Firmware/Peripherals/Source/gd32f10x_misc.c",
            "Firmware/Peripherals/USB/usbd_lld_int.c",
            "Firmware/Peripherals/USB/usbd_lld_core.c",
            "Firmware/Peripherals/USB/usbd_core.c",
            "Firmware/Peripherals/USB/cdc_acm_core.c",
            "Firmware/Peripherals/USB/usbd_transc.c",
            "Firmware/Peripherals/USB/usbd_enum.c",
        ]

# common_path = "C:/Soft/gcc-arm-none-eabi/bin/"
# common_path = "/media/master/0E5513DF0E5513DF/Work/GD32/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux/gcc-arm-none-eabi-10.3-2021.10/bin/"
common_path = "/usr/bin/"

stlink_path = "/media/master/0E5513DF0E5513DF/Work/Python/PySTLINK/pystlink"

asm_path = f"{common_path}arm-none-eabi-as"
asm_flags = "-x assembler-with-cpp"

compiler_path = f"{common_path}arm-none-eabi-gcc"
compiler_flags =   ["-c",
                    "-mcpu=cortex-m3",
                    "-mthumb",
                    "-DUSE_STDPERIPH_DRIVER -DGD32F10X_MD",
                    "-I .",
                    "-I Firmware/Include",
                    "-I Firmware/CMSIS/CoreSupport",
                    "-I Firmware/Peripherals/Include",
                    "-I Firmware/Peripherals/USB",
                    "-I src",
                    "-Os",
                    "-Wall -Wpedantic -Wextra",
                    "-fno-common -fdata-sections -ffunction-sections",
                    "-std=c99"
                    ]

linker_path = f"{common_path}arm-none-eabi-ld"
linker_flags = ["-mcpu=cortex-m3 -mthumb",
                "-specs=nano.specs",
                "-Tsrc/GD32F103C8Tx_FLASH.ld",
                "-lc -lm -lnosys",
                "-Wl,--gc-sections"
                ]

objcopy_path = f"{common_path}arm-none-eabi-objcopy"

def compile_asm(input_file:str, output_file:str) -> int:
    ''' returns 1 if error, 0 if ok '''
    print(". ", end='', flush=True)
    return subprocess.call(f'{compiler_path} {asm_flags} {" ".join(compiler_flags)} {input_file} -o {output_file}',shell=True)


def compile(input_file:str, output_file:str) -> int:
    ''' returns 1 if error, 0 if ok '''
    print(". ", end='', flush=True)
    return subprocess.call(f'{compiler_path} {" ".join(compiler_flags)} {input_file} -o {output_file}',shell=True)


def compile_all():
    for item in files:
        f_name = item.split("/")[-1]
        f_type = item.split(".")[-1]
        if f_type == "c":
            if compile(item, f'temp_files/{f_name.split(".")[0]}.o') == 1:
                return 1
        elif f_type == "s":
            if compile_asm(item, f'temp_files/{f_name.split(".")[0]}.o') == 1:
                return 1
    return 0

def link_all():
    items = ''
    for item in files:
        f_name = item.split("/")[-1]
        items += f'temp_files/{f_name.split(".")[0]}.o '
    return subprocess.call(f'{compiler_path} {" ".join(linker_flags)} {items} -o temp_files/main.elf', shell=True)


def convert_elf():
    subprocess.call(f'{objcopy_path} -O ihex temp_files/main.elf temp_files/main.hex', shell=True)
    subprocess.call(f'{objcopy_path} -O binary temp_files/main.elf firmware.bin', shell=True)


def main():
    print("Compilation: ", end='', flush=True)
    if not os.path.isdir('temp_files'):
        subprocess.call('mkdir temp_files', shell=True)
    if 0 == compile_all():
        print("complete!")
        if 0 == link_all():
            convert_elf()
            subprocess.call(f'{common_path}arm-none-eabi-objdump --disassemble temp_files/main.elf > temp_files/main.list', shell=True)
            subprocess.call('rm -r temp_files/*', shell=True)
            if os.path.isdir(stlink_path):
                subprocess.call(f'{stlink_path}/./pystlink flash:erase:verify:firmware.bin', shell=True)


if __name__ == '__main__':
    main()
