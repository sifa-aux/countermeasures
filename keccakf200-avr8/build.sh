mkdir -p .dep
mkdir -p objdir

rm -f -- main-CW303.hex main-CW303.eep main-CW303.cof main-CW303.elf main-CW303.map main-CW303.sym main-CW303.lss
rm -f -- objdir/*.o
rm -f -- objdir/*.lst
rm -f -- main.s simpleserial.s XMEGA_AES_driver.s uart.s usart_driver.s xmega_hal.s rtc_driver.s
rm -f -- main.d simpleserial.d XMEGA_AES_driver.d uart.d usart_driver.d xmega_hal.d rtc_driver.d
rm -f -- main.i simpleserial.i XMEGA_AES_driver.i uart.i usart_driver.i xmega_hal.i rtc_driver.i

# Compiling: main.c
avr-gcc -c -mmcu=atxmega128d3 -I. -fpack-struct -g0 -DSS_VER=SS_VER_1_1 -DHAL_TYPE=HAL_xmega -DPLATFORM=CW303 -DF_CPU=7372800UL -Os -funsigned-char -funsigned-bitfields -fshort-enums -Wall -Wstrict-prototypes -Wa,-adhlns=objdir/main.lst -Ifirmware/simpleserial/ -Ifirmware/hal -Ifirmware/hal/xmega -Ifirmware/crypto/ -std=gnu99 -MMD -MP -MF .dep/main.o.d main.c -o objdir/main.o

# Compiling: firmware/simpleserial/simpleserial.c
avr-gcc -c -mmcu=atxmega128d3 -I. -fpack-struct -g0 -DSS_VER=SS_VER_1_1 -DHAL_TYPE=HAL_xmega -DPLATFORM=CW303 -DF_CPU=7372800UL -Os -funsigned-char -funsigned-bitfields -fshort-enums -Wall -Wstrict-prototypes -Wa,-adhlns=objdir/simpleserial.lst -Ifirmware/simpleserial/ -Ifirmware/hal -Ifirmware/hal/xmega -Ifirmware/crypto/ -std=gnu99 -MMD -MP -MF .dep/simpleserial.o.d firmware/simpleserial/simpleserial.c -o objdir/simpleserial.o

# Compiling: firmware/hal/xmega/uart.c
avr-gcc -c -mmcu=atxmega128d3 -I. -fpack-struct -g0 -DSS_VER=SS_VER_1_1 -DHAL_TYPE=HAL_xmega -DPLATFORM=CW303 -DF_CPU=7372800UL -Os -funsigned-char -funsigned-bitfields -fshort-enums -Wall -Wstrict-prototypes -Wa,-adhlns=objdir/uart.lst -Ifirmware/simpleserial/ -Ifirmware/hal -Ifirmware/hal/xmega -Ifirmware/crypto/ -std=gnu99 -MMD -MP -MF .dep/uart.o.d firmware/hal/xmega/uart.c -o objdir/uart.o

# Compiling: firmware/hal/xmega/usart_driver.c
avr-gcc -c -mmcu=atxmega128d3 -I. -fpack-struct -g0 -DSS_VER=SS_VER_1_1 -DHAL_TYPE=HAL_xmega -DPLATFORM=CW303 -DF_CPU=7372800UL -Os -funsigned-char -funsigned-bitfields -fshort-enums -Wall -Wstrict-prototypes -Wa,-adhlns=objdir/usart_driver.lst -Ifirmware/simpleserial/ -Ifirmware/hal -Ifirmware/hal/xmega -Ifirmware/crypto/ -std=gnu99 -MMD -MP -MF .dep/usart_driver.o.d firmware/hal/xmega/usart_driver.c -o objdir/usart_driver.o

# Compiling: firmware/hal/xmega/xmega_hal.c
avr-gcc -c -mmcu=atxmega128d3 -I. -fpack-struct -g0 -DSS_VER=SS_VER_1_1 -DHAL_TYPE=HAL_xmega -DPLATFORM=CW303 -DF_CPU=7372800UL -Os -funsigned-char -funsigned-bitfields -fshort-enums -Wall -Wstrict-prototypes -Wa,-adhlns=objdir/xmega_hal.lst -Ifirmware/simpleserial/ -Ifirmware/hal -Ifirmware/hal/xmega -Ifirmware/crypto/ -std=gnu99 -MMD -MP -MF .dep/xmega_hal.o.d firmware/hal/xmega/xmega_hal.c -o objdir/xmega_hal.o

# Compiling: firmware/hal/xmega/rtc_driver.c
avr-gcc -c -mmcu=atxmega128d3 -I. -fpack-struct -g0 -DSS_VER=SS_VER_1_1 -DHAL_TYPE=HAL_xmega -DPLATFORM=CW303 -DF_CPU=7372800UL -Os -funsigned-char -funsigned-bitfields -fshort-enums -Wall -Wstrict-prototypes -Wa,-adhlns=objdir/rtc_driver.lst -Ifirmware/simpleserial/ -Ifirmware/hal -Ifirmware/hal/xmega -Ifirmware/crypto/ -std=gnu99 -MMD -MP -MF .dep/rtc_driver.o.d firmware/hal/xmega/rtc_driver.c -o objdir/rtc_driver.o

# Linking: main-CW303.elf
avr-gcc -mmcu=atxmega128d3 -I. -fpack-struct -g0 -DSS_VER=SS_VER_1_1 -DHAL_TYPE=HAL_xmega -DPLATFORM=CW303 -DF_CPU=7372800UL -Os -funsigned-char -funsigned-bitfields -fshort-enums -Wall -Wstrict-prototypes -Wa,-adhlns=objdir/main.o -Ifirmware/simpleserial/ -Ifirmware/hal -Ifirmware/hal/xmega -Ifirmware/crypto/ -std=gnu99 -MMD -MP -MF .dep/main-CW303.elf.d objdir/main.o objdir/simpleserial.o objdir/uart.o objdir/usart_driver.o objdir/xmega_hal.o objdir/rtc_driver.o --output main-CW303.elf -Wl,-Map=main-CW303.map,--cref -lm

avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature main-CW303.elf main-CW303.hex
