# Performance comparison of SIFA protected CHI Sbox

This code can be used to perform a direct performance comparison between masked implementations of `Keccak-f[200]`, with and without SIFA protection. You can find the corresponding paper [here](https://eprint.iacr.org/2019/536.pdf).

To allow a simpler comparison the cost of the RNG is neglected.

## Usage Instructions - Chipwhisperer CWLITE XMEGA

Get the AVR Toolchain: `apt-get install avr-libc gcc-avr`
Execute `bash build.sh` then flash the binary using the Chipwhisperer framework.

