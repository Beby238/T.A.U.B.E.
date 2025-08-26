## Error Message vom ESP32-S3 wroom mit CAm modul drinne:
ELF file SHA256: 0713cb713
Rebooting...
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0xc (RTC_SW_CPU_RST),boot:0x8 (SPI_FAST_FLASH_BOOT)
Saved PC:0x4037843e
SPIWP:0xee
mode:DIO, clock div:1
load:0x3fce2820,len:0x1180
load:0x403c8700,len:0xc2c
load:0x403cb700,len:0x311c
entry 0x403c88b8

Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.
Core  1 register dump:
PC      : 0x4201a55d  PS      : 0x00060430  A0      : 0x8201263a  A1      : 0x3fcebe00  
A2      : 0x3fcecd98  A3      : 0x3fcebef8  A4      : 0xffff8fff  A5      : 0x00060223  
A6      : 0x3c04b854  A7      : 0x00000001  A8      : 0x3c04b8ac  A9      : 0x00000000  
A10     : 0x00000016  A11     : 0x00000060  A12     : 0x00000060  A13     : 0x00060223  
A14     : 0x00000001  A15     : 0x3fcec484  SAR     : 0x00000020  EXCCAUSE: 0x0000001c  
EXCVADDR: 0x00000000  LBEG    : 0x400570e8  LEND    : 0x400570f3  LCOUNT  : 0x00000000  

Backtrace: 0x4201a55a:0x3fcebe00 0x42012637:0x3fcebe50 0x42012179:0x3fcebe80 0x42002562:0x3fcebee0 0x4200416a:0x3fcebf80 0x4037b3e9:0x3fcebfa0




