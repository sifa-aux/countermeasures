#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hal.h"
#include "simpleserial.h"

typedef unsigned char UINT8;
typedef unsigned char tSmallUInt;
typedef UINT8 tKeccakLane;

// to allow a simpler performance comparision we deactivate the rng
#define RAND 0x42
// #define RAND rand()

#define ROL8(a, offset) (UINT8)((((UINT8)a) << (offset & 7)) ^ (((UINT8)a) >> (8 - (offset & 7))))

const UINT8 KeccakP200_RotationConstants[25] =
    {
        1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14, 27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44};

const UINT8 KeccakP200_PiLane[25] =
    {
        10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4, 15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1};

const UINT8 KeccakP200_Mod5[10] =
    {
        0, 1, 2, 3, 4, 0, 1, 2, 3, 4};

#define MOD5(argValue) KeccakP200_Mod5[argValue]

const UINT8 KeccakF200_RoundConstants[] =
    {
        0x01, 0x82, 0x8a, 0x00, 0x8b, 0x01, 0x81, 0x09, 0x8a, 0x88, 0x09, 0x0a, 0x8b, 0x8b, 0x89, 0x03, 0x02, 0x80};

#define VAL(a, b, c, d, e, idx) ((((a) & (1 << idx)) >> idx) << 0 | (((b) & (1 << idx)) >> idx) << 1 | (((c) & (1 << idx)) >> idx) << 2 | (((d) & (1 << idx)) >> idx) << 3 | (((e) & (1 << idx)) >> idx) << 4)

uint8_t get_pt(uint8_t *pt)
{
    srand(0);

    // string buf
    unsigned char buf[16];
    memset(buf, 0, 16);

    UINT8 argState[25];
    UINT8 argState2[25];
    UINT8 nr = 18;
    memset(argState, 0, 25);
    memset(argState2, 0, 25);
    argState[0] = 1;

    trigger_high();

    /////////////////////////////////////
    //      KECCAK-F[200] - START      //
    /////////////////////////////////////

    tSmallUInt x, y;
    tKeccakLane temp;
    tKeccakLane BC[5];
    tKeccakLane BCC[5];
    tKeccakLane *state;
    tKeccakLane *state2;
    const tKeccakLane *rc;

    state = (tKeccakLane *)argState;
    state2 = (tKeccakLane *)argState2;

    rc = KeccakF200_RoundConstants + 18 - nr;
    do
    {
        /* Theta */
        for (x = 0; x < 5; ++x)
        {
            BC[x] = state[x] ^ state[5 + x] ^ state[10 + x] ^ state[15 + x] ^ state[20 + x];
        }
        for (x = 0; x < 5; ++x)
        {
            temp = BC[MOD5(x + 4)] ^ ROL8(BC[MOD5(x + 1)], 1);
            for (y = 0; y < 25; y += 5)
            {
                state[y + x] ^= temp;
            }
        }

        /* Theta */
        for (x = 0; x < 5; ++x)
        {
            BCC[x] = state2[x] ^ state2[5 + x] ^ state2[10 + x] ^ state2[15 + x] ^ state2[20 + x];
        }
        for (x = 0; x < 5; ++x)
        {
            temp = BCC[MOD5(x + 4)] ^ ROL8(BCC[MOD5(x + 1)], 1);
            for (y = 0; y < 25; y += 5)
            {
                state2[y + x] ^= temp;
            }
        }

        /* Rho Pi */
        temp = state[1];
        for (x = 0; x < 24; ++x)
        {
            BC[0] = state[KeccakP200_PiLane[x]];
            state[KeccakP200_PiLane[x]] = ROL8(temp, KeccakP200_RotationConstants[x]);
            temp = BC[0];
        }

        /* Rho Pi */
        temp = state2[1];
        for (x = 0; x < 24; ++x)
        {
            BCC[0] = state2[KeccakP200_PiLane[x]];
            state2[KeccakP200_PiLane[x]] = ROL8(temp, KeccakP200_RotationConstants[x]);
            temp = BCC[0];
        }

        ////////////////////////////////
        //      MASKED CHI - DOM      //
        ////////////////////////////////
        for (y = 0; y < 25; y += 5)
        {
            BC[0] = state[y + 0];
            BC[1] = state[y + 1];
            BC[2] = state[y + 2];
            BC[3] = state[y + 3];
            BC[4] = state[y + 4];
            BCC[0] = state2[y + 0];
            BCC[1] = state2[y + 1];
            BCC[2] = state2[y + 2];
            BCC[3] = state2[y + 3];
            BCC[4] = state2[y + 4];
            unsigned char R0 = RAND;
            unsigned char R1 = RAND;
            unsigned char R2 = RAND;
            unsigned char R3 = RAND;
            unsigned char R4 = RAND;
            asm volatile(
                "mov r20, %[a0]"
                "\n\t"
                "com r20"
                "\n\t"
                "mov r21, %[b0]"
                "\n\t"
                "and r21, r20"
                "\n\t"
                "mov r22, %[b1]"
                "\n\t"
                "and r22, r20"
                "\n\t"
                "mov r23, %[a1]"
                "\n\t"
                "and r23, %[b0]"
                "\n\t"
                "mov r24, %[b1]"
                "\n\t"
                "and r24, %[a1]"
                "\n\t"
                "eor r22, %[R0]"
                "\n\t"
                "eor r23, %[R0]"
                "\n\t"
                "eor r21, r22"
                "\n\t"
                "eor r23, r24"
                "\n\t"
                "mov r25, %[e0]"
                "\n\t"
                "mov r26, %[e1]"
                "\n\t"
                "eor r25, r21"
                "\n\t"
                "eor r26, r23"
                "\n\t"

                //---------------------

                "mov r20, %[b0]"
                "\n\t"
                "com r20"
                "\n\t"
                "mov r21, %[c0]"
                "\n\t"
                "and r21, r20"
                "\n\t"
                "mov r22, %[c1]"
                "\n\t"
                "and r22, r20"
                "\n\t"
                "mov r23, %[b1]"
                "\n\t"
                "and r23, %[c0]"
                "\n\t"
                "mov r24, %[c1]"
                "\n\t"
                "and r24, %[b1]"
                "\n\t"
                "eor r22, %[R1]"
                "\n\t"
                "eor r23, %[R1]"
                "\n\t"
                "eor r21, r22"
                "\n\t"
                "eor r23, r24"
                "\n\t"
                "mov r18, %[a0]"
                "\n\t"
                "mov r19, %[a1]"
                "\n\t"
                "eor r18, r21"
                "\n\t"
                "eor r19, r23"
                "\n\t"

                //---------------------

                "mov r20, %[c0]"
                "\n\t"
                "com r20"
                "\n\t"
                "mov r21, %[d0]"
                "\n\t"
                "and r21, r20"
                "\n\t"
                "mov r22, %[d1]"
                "\n\t"
                "and r22, r20"
                "\n\t"
                "mov r23, %[c1]"
                "\n\t"
                "and r23, %[d0]"
                "\n\t"
                "mov r24, %[d1]"
                "\n\t"
                "and r24, %[c1]"
                "\n\t"
                "eor r22, %[R2]"
                "\n\t"
                "eor r23, %[R2]"
                "\n\t"
                "eor r21, r22"
                "\n\t"
                "eor r23, r24"
                "\n\t"
                "eor %[b0], r21"
                "\n\t"
                "eor %[b1], r23"
                "\n\t"

                //---------------------

                "mov r20, %[d0]"
                "\n\t"
                "com r20"
                "\n\t"
                "mov r21, %[e0]"
                "\n\t"
                "and r21, r20"
                "\n\t"
                "mov r22, %[e1]"
                "\n\t"
                "and r22, r20"
                "\n\t"
                "mov r23, %[d1]"
                "\n\t"
                "and r23, %[e0]"
                "\n\t"
                "mov r24, %[e1]"
                "\n\t"
                "and r24, %[d1]"
                "\n\t"
                "eor r22, %[R3]"
                "\n\t"
                "eor r23, %[R3]"
                "\n\t"
                "eor r21, r22"
                "\n\t"
                "eor r23, r24"
                "\n\t"
                "eor %[c0], r21"
                "\n\t"
                "eor %[c1], r23"
                "\n\t"

                //---------------------

                "com %[e0]"
                "\n\t"
                "mov r21, %[a0]"
                "\n\t"
                "and r21, %[e0]"
                "\n\t"
                "mov r22, %[a1]"
                "\n\t"
                "and r22, %[e0]"
                "\n\t"
                "mov r23, %[e1]"
                "\n\t"
                "and r23, %[a0]"
                "\n\t"
                "mov r24, %[a1]"
                "\n\t"
                "and r24, %[e1]"
                "\n\t"
                "eor r22, %[R4]"
                "\n\t"
                "eor r23, %[R4]"
                "\n\t"
                "eor r21, r22"
                "\n\t"
                "eor r23, r24"
                "\n\t"
                "eor %[d0], r21"
                "\n\t"
                "eor %[d1], r23"
                "\n\t"

                //---------------------

                "mov %[a0], r18"
                "\n\t"
                "mov %[a1], r19"
                "\n\t"
                "mov %[e0], r25"
                "\n\t"
                "mov %[e1], r26"
                "\n\t"
                : "=r"(BC[0]), "=r"(BCC[0]),
                  "=r"(BC[1]), "=r"(BCC[1]),
                  "=r"(BC[2]), "=r"(BCC[2]),
                  "=r"(BC[3]), "=r"(BCC[3]),
                  "=r"(BC[4]), "=r"(BCC[4])
                :
                [ a0 ] "0"(BC[0]), [ a1 ] "1"(BCC[0]),
                [ b0 ] "2"(BC[1]), [ b1 ] "3"(BCC[1]),
                [ c0 ] "4"(BC[2]), [ c1 ] "5"(BCC[2]),
                [ d0 ] "6"(BC[3]), [ d1 ] "7"(BCC[3]),
                [ e0 ] "8"(BC[4]), [ e1 ] "9"(BCC[4]),
                [ R0 ] "r"(R0),
                [ R1 ] "r"(R1),
                [ R2 ] "r"(R2),
                [ R3 ] "r"(R3),
                [ R4 ] "r"(R4)
                : "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r18", "r19");
            state[y + 0] = BC[0];
            state[y + 1] = BC[1];
            state[y + 2] = BC[2];
            state[y + 3] = BC[3];
            state[y + 4] = BC[4];
            state2[y + 0] = BCC[0];
            state2[y + 1] = BCC[1];
            state2[y + 2] = BCC[2];
            state2[y + 3] = BCC[3];
            state2[y + 4] = BCC[4];
        }

        ////////////////////////////////////
        //      MASKED CHI - TOFFOLI      //
        ////////////////////////////////////
        // for (y = 0; y < 25; y += 5)
        // {
        //     BC[0] = state[y + 0];
        //     BC[1] = state[y + 1];
        //     BC[2] = state[y + 2];
        //     BC[3] = state[y + 3];
        //     BC[4] = state[y + 4];
        //     BCC[0] = state2[y + 0];
        //     BCC[1] = state2[y + 1];
        //     BCC[2] = state2[y + 2];
        //     BCC[3] = state2[y + 3];
        //     BCC[4] = state2[y + 4];
        //     UINT8 r0 = RAND;
        //     UINT8 r1 = r0;
        //     asm volatile(
        //         "mov r24, %[e0]"
        //         "\n\t"
        //         "com r24"
        //         "\n\t"
        //         "mov r25, %[a1]"
        //         "\n\t"
        //         "and r25,   r24"
        //         "\n\t"
        //         "eor %[r0], r25"
        //         "\n\t"

        //         "mov r24, %[e0]"
        //         "\n\t"
        //         "com r24"
        //         "\n\t"
        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[a0]"
        //         "\n\t"
        //         "and r25,   r24"
        //         "\n\t"
        //         "eor %[r0], r25"
        //         "\n\t"

        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[a1]"
        //         "\n\t"
        //         "and r25, %[e1]"
        //         "\n\t"
        //         "eor %[r1], r25"
        //         "\n\t"

        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[a0]"
        //         "\n\t"
        //         "and r25, %[e1]"
        //         "\n\t"
        //         "eor %[r1], r25"
        //         "\n\t"

        //         //---------------------

        //         "mov r24, %[b0]"
        //         "\n\t"
        //         "com r24"
        //         "\n\t"
        //         "mov r25, %[c1]"
        //         "\n\t"
        //         "and r25,   r24"
        //         "\n\t"
        //         "eor %[a0], r25"
        //         "\n\t"

        //         "mov r24, %[b0]"
        //         "\n\t"
        //         "com r24"
        //         "\n\t"
        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[c0]"
        //         "\n\t"
        //         "and r25,   r24"
        //         "\n\t"
        //         "eor %[a0], r25"
        //         "\n\t"

        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[c1]"
        //         "\n\t"
        //         "and r25, %[b1]"
        //         "\n\t"
        //         "eor %[a1], r25"
        //         "\n\t"

        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[c0]"
        //         "\n\t"
        //         "and r25, %[b1]"
        //         "\n\t"
        //         "eor %[a1], r25"
        //         "\n\t"

        //         //---------------------

        //         "mov r24, %[d0]"
        //         "\n\t"
        //         "com r24"
        //         "\n\t"
        //         "mov r25, %[e1]"
        //         "\n\t"
        //         "and r25,   r24"
        //         "\n\t"
        //         "eor %[c0], r25"
        //         "\n\t"

        //         "mov r24, %[d0]"
        //         "\n\t"
        //         "com r24"
        //         "\n\t"
        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[e0]"
        //         "\n\t"
        //         "and r25,   r24"
        //         "\n\t"
        //         "eor %[c0], r25"
        //         "\n\t"

        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[e1]"
        //         "\n\t"
        //         "and r25, %[d1]"
        //         "\n\t"
        //         "eor %[c1], r25"
        //         "\n\t"

        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[e0]"
        //         "\n\t"
        //         "and r25, %[d1]"
        //         "\n\t"
        //         "eor %[c1], r25"
        //         "\n\t"

        //         //---------------------

        //         "mov r24, %[a0]"
        //         "\n\t"
        //         "com r24"
        //         "\n\t"
        //         "mov r25, %[b1]"
        //         "\n\t"
        //         "and r25,   r24"
        //         "\n\t"
        //         "eor %[e0], r25"
        //         "\n\t"

        //         "mov r24, %[a0]"
        //         "\n\t"
        //         "com r24"
        //         "\n\t"
        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[b0]"
        //         "\n\t"
        //         "and r25,   r24"
        //         "\n\t"
        //         "eor %[e0], r25"
        //         "\n\t"

        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[b1]"
        //         "\n\t"
        //         "and r25, %[a1]"
        //         "\n\t"
        //         "eor %[e1], r25"
        //         "\n\t"

        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[b0]"
        //         "\n\t"
        //         "and r25, %[a1]"
        //         "\n\t"
        //         "eor %[e1], r25"
        //         "\n\t"

        //         //---------------------

        //         "mov r24, %[c0]"
        //         "\n\t"
        //         "com r24"
        //         "\n\t"
        //         "mov r25, %[d1]"
        //         "\n\t"
        //         "and r25,   r24"
        //         "\n\t"
        //         "eor %[b0], r25"
        //         "\n\t"

        //         "mov r24, %[c0]"
        //         "\n\t"
        //         "com r24"
        //         "\n\t"
        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[d0]"
        //         "\n\t"
        //         "and r25,   r24"
        //         "\n\t"
        //         "eor %[b0], r25"
        //         "\n\t"

        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[d1]"
        //         "\n\t"
        //         "and r25, %[c1]"
        //         "\n\t"
        //         "eor %[b1], r25"
        //         "\n\t"

        //         "ldi r25, 0x00"
        //         "\n\t"
        //         "mov r25, %[d0]"
        //         "\n\t"
        //         "and r25, %[c1]"
        //         "\n\t"
        //         "eor %[b1], r25"
        //         "\n\t"

        //         //---------------------

        //         "eor %[d0], %[r0]"
        //         "\n\t"
        //         "eor %[d1], %[r1]"
        //         "\n\t"
        //         : "=r"(BC[0]), "=r"(BCC[0]),
        //           "=r"(BC[1]), "=r"(BCC[1]),
        //           "=r"(BC[2]), "=r"(BCC[2]),
        //           "=r"(BC[3]), "=r"(BCC[3]),
        //           "=r"(BC[4]), "=r"(BCC[4]),
        //           "=r"(r0), "=r"(r1)
        //         :
        //         [ a0 ] "0"(BC[0]), [ a1 ] "1"(BCC[0]),
        //         [ b0 ] "2"(BC[1]), [ b1 ] "3"(BCC[1]),
        //         [ c0 ] "4"(BC[2]), [ c1 ] "5"(BCC[2]),
        //         [ d0 ] "6"(BC[3]), [ d1 ] "7"(BCC[3]),
        //         [ e0 ] "8"(BC[4]), [ e1 ] "9"(BCC[4]),
        //         [ r0 ] "10"(r0), [ r1 ] "11"(r1)
        //         : "r24", "r25" // t0, t1
        //     );
        //     state[y + 0] = BC[0];
        //     state[y + 1] = BC[1];
        //     state[y + 2] = BC[2];
        //     state[y + 3] = BC[3];
        //     state[y + 4] = BC[4];
        //     state2[y + 0] = BCC[0];
        //     state2[y + 1] = BCC[1];
        //     state2[y + 2] = BCC[2];
        //     state2[y + 3] = BCC[3];
        //     state2[y + 4] = BCC[4];
        // }

        /*    Iota */
        temp = *(rc++);
        state[0] ^= temp;
    } while (temp != 0x80);

    trigger_low();

    for (size_t i = 0; i < 16; i++)
    {
        buf[i] = argState[i] ^ argState2[i];
    }

    simpleserial_put('r', 16, buf);

    return 0;
}

// Not needed
uint8_t reset(uint8_t *x)
{
    return 0;
}

int main(void)
{
    platform_init();
    init_uart();
    trigger_setup();
    putch('h');
    putch('e');
    putch('l');
    putch('l');
    putch('o');
    putch('\n');
    simpleserial_init();
    simpleserial_addcmd('p', 16, get_pt);
    simpleserial_addcmd('x', 0, reset);
    while (1)
    {
        simpleserial_get();
    }
}
