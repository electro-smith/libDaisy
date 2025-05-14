/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 rppicomidi (modified without Boost dependency)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#pragma once
#include <stdint.h>

// USB MIDI allows up to 16 virtual cable "streams" per USB endpoint
// The following macros for for a multi-stream interface still assume one
// IN endpoint and one OUT endpoint but permit up to 16 streams per endpoint

// Assign jack ID numbers for all External IN and Embedded OUT Jacks
// - _cablenum is the virtual cable number associated with the Jack pair
#define TUD_MIDI_MULTI_JACKID_IN_EXT(_cablenum) (uint8_t)((_cablenum)*2 + 1)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB(_cablenum) (uint8_t)((_cablenum)*2 + 2)

// Assign jack ID numbers for all External OUT and Embedded IN Jacks
// - _cablenum is the nth virtual cable number associated with the Jack pair, where n starts from 1
// - _numcables_in is the number of External IN Jacks.
#define TUD_MIDI_MULTI_JACKID_IN_EMB(_cablenum, _numcables_in) \
    (uint8_t)((_cablenum)*2 + 1 + ((_numcables_in)*2))
#define TUD_MIDI_MULTI_JACKID_OUT_EXT(_cablenum, _numcables_in) \
    (uint8_t)((_cablenum)*2 + 2 + ((_numcables_in)*2))

#ifndef CFG_TUD_MIDI_FIRST_PORT_STRIDX
#define CFG_TUD_MIDI_FIRST_PORT_STRIDX 0
#endif

#if CFG_TUD_MIDI_FIRST_PORT_STRIDX
#define EMB_IN_JACK_STRIDX(n) (n + (CFG_TUD_MIDI_FIRST_PORT_STRIDX))
#define EMB_OUT_JACK_STRIDX(_numcables_in, n) \
    (n + _numcables_in + (CFG_TUD_MIDI_FIRST_PORT_STRIDX))
#else
#define EMB_IN_JACK_STRIDX(n) 0
#define EMB_OUT_JACK_STRIDX(_numcables_in, n) 0
#endif

#define TUD_MIDI_MULTI_JACK_IN_DESC(_cablenum, _stridx)                       \
    /* MS In Jack (External) */                                               \
    6, TUSB_DESC_CS_INTERFACE, MIDI_CS_INTERFACE_IN_JACK, MIDI_JACK_EXTERNAL, \
        TUD_MIDI_MULTI_JACKID_IN_EXT(_cablenum),                              \
        0, /* MS Out Jack (Embedded), connected to In Jack External */        \
        9, TUSB_DESC_CS_INTERFACE, MIDI_CS_INTERFACE_OUT_JACK,                \
        MIDI_JACK_EMBEDDED, TUD_MIDI_MULTI_JACKID_OUT_EMB(_cablenum), 1,      \
        TUD_MIDI_MULTI_JACKID_IN_EXT(_cablenum), 1, _stridx

#define TUD_MIDI_MULTI_DESC_JACK_LEN(_numcables) ((6 + 9) * (_numcables))

#define TUD_MIDI_MULTI_JACK_OUT_DESC(_cablenum, _numcables_in, _stridx)       \
    /* MS In Jack (Embedded) */                                               \
    6, TUSB_DESC_CS_INTERFACE, MIDI_CS_INTERFACE_IN_JACK, MIDI_JACK_EMBEDDED, \
        TUD_MIDI_MULTI_JACKID_IN_EMB(_cablenum, _numcables_in),               \
        _stridx, /* MS Out Jack (External), connected to In Jack Embedded */  \
        9, TUSB_DESC_CS_INTERFACE, MIDI_CS_INTERFACE_OUT_JACK,                \
        MIDI_JACK_EXTERNAL,                                                   \
        TUD_MIDI_MULTI_JACKID_OUT_EXT(_cablenum, _numcables_in), 1,           \
        TUD_MIDI_MULTI_JACKID_IN_EMB(_cablenum, _numcables_in), 1, 0

#define TUD_MIDI_MULTI_DESC_LEN(_numcables_in, _numcables_out)            \
    (TUD_MIDI_DESC_HEAD_LEN + TUD_MIDI_MULTI_DESC_JACK_LEN(_numcables_in) \
     + TUD_MIDI_MULTI_DESC_JACK_LEN(_numcables_out)                       \
     + TUD_MIDI_DESC_EP_LEN(_numcables_in)                                \
     + TUD_MIDI_DESC_EP_LEN(_numcables_out))

/* Replace BOOST_PP_ENUM macros with manual expansion macros */
/* Support up to 16 cables (which is the USB MIDI limit) */

/* Macros for IN jacks */
#define TUD_MIDI_MULTI_DESC_JACK_IN_0(...)
#define TUD_MIDI_MULTI_DESC_JACK_IN_1(...) \
    TUD_MIDI_MULTI_JACK_IN_DESC(0, EMB_IN_JACK_STRIDX(0))
#define TUD_MIDI_MULTI_DESC_JACK_IN_2(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_1(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(1, EMB_IN_JACK_STRIDX(1))
#define TUD_MIDI_MULTI_DESC_JACK_IN_3(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_2(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(2, EMB_IN_JACK_STRIDX(2))
#define TUD_MIDI_MULTI_DESC_JACK_IN_4(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_3(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(3, EMB_IN_JACK_STRIDX(3))
#define TUD_MIDI_MULTI_DESC_JACK_IN_5(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_4(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(4, EMB_IN_JACK_STRIDX(4))
#define TUD_MIDI_MULTI_DESC_JACK_IN_6(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_5(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(5, EMB_IN_JACK_STRIDX(5))
#define TUD_MIDI_MULTI_DESC_JACK_IN_7(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_6(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(6, EMB_IN_JACK_STRIDX(6))
#define TUD_MIDI_MULTI_DESC_JACK_IN_8(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_7(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(7, EMB_IN_JACK_STRIDX(7))
#define TUD_MIDI_MULTI_DESC_JACK_IN_9(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_8(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(8, EMB_IN_JACK_STRIDX(8))
#define TUD_MIDI_MULTI_DESC_JACK_IN_10(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_9(),        \
        TUD_MIDI_MULTI_JACK_IN_DESC(9, EMB_IN_JACK_STRIDX(9))
#define TUD_MIDI_MULTI_DESC_JACK_IN_11(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_10(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(10, EMB_IN_JACK_STRIDX(10))
#define TUD_MIDI_MULTI_DESC_JACK_IN_12(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_11(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(11, EMB_IN_JACK_STRIDX(11))
#define TUD_MIDI_MULTI_DESC_JACK_IN_13(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_12(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(12, EMB_IN_JACK_STRIDX(12))
#define TUD_MIDI_MULTI_DESC_JACK_IN_14(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_13(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(13, EMB_IN_JACK_STRIDX(13))
#define TUD_MIDI_MULTI_DESC_JACK_IN_15(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_14(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(14, EMB_IN_JACK_STRIDX(14))
#define TUD_MIDI_MULTI_DESC_JACK_IN_16(...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_15(),       \
        TUD_MIDI_MULTI_JACK_IN_DESC(15, EMB_IN_JACK_STRIDX(15))

/* Macros for OUT jacks */
#define TUD_MIDI_MULTI_DESC_JACK_OUT_0(...)
#define TUD_MIDI_MULTI_DESC_JACK_OUT_1(_numcables_in) \
    TUD_MIDI_MULTI_JACK_OUT_DESC(                     \
        0, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 0))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_2(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_1(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                 \
            1, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 1))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_3(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_2(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                 \
            2, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 2))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_4(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_3(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                 \
            3, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 3))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_5(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_4(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                 \
            4, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 4))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_6(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_5(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                 \
            5, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 5))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_7(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_6(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                 \
            6, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 6))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_8(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_7(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                 \
            7, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 7))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_9(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_8(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                 \
            8, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 8))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_10(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_9(_numcables_in),     \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                  \
            9, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 9))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_11(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_10(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                  \
            10, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 10))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_12(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_11(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                  \
            11, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 11))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_13(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_12(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                  \
            12, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 12))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_14(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_13(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                  \
            13, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 13))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_15(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_14(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                  \
            14, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 14))
#define TUD_MIDI_MULTI_DESC_JACK_OUT_16(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_15(_numcables_in),    \
        TUD_MIDI_MULTI_JACK_OUT_DESC(                  \
            15, _numcables_in, EMB_OUT_JACK_STRIDX(_numcables_in, 15))

/* Select proper macro based on number of cables */
#define TUD_MIDI_MULTI_DESC_JACK_IN_X(_numcables_in, ...) \
    TUD_MIDI_MULTI_DESC_JACK_IN_##_numcables_in(__VA_ARGS__)
#define TUD_MIDI_MULTI_DESC_JACK_OUT_X(_numcables_out, _numcables_in) \
    TUD_MIDI_MULTI_DESC_JACK_OUT_##_numcables_out(_numcables_in)

/* Combined descriptor for both IN and OUT jacks */
#define TUD_MIDI_MULTI_DESC_JACK_DESC(_numcables_in, _numcables_out) \
    TUD_MIDI_MULTI_DESC_JACK_IN_X(_numcables_in),                    \
        TUD_MIDI_MULTI_DESC_JACK_OUT_X(_numcables_out, _numcables_in)

/* Similar approach for jackIDs */
#define TUD_MIDI_MULTI_JACKID_IN_EMB_0(...)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_1(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB(0, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_2(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_1(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(1, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_3(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_2(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(2, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_4(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_3(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(3, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_5(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_4(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(4, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_6(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_5(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(5, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_7(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_6(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(6, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_8(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_7(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(7, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_9(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_8(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(8, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_10(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_9(_numcables_in),     \
        TUD_MIDI_MULTI_JACKID_IN_EMB(9, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_11(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_10(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(10, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_12(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_11(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(11, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_13(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_12(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(12, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_14(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_13(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(13, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_15(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_14(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(14, _numcables_in)
#define TUD_MIDI_MULTI_JACKID_IN_EMB_16(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_15(_numcables_in),    \
        TUD_MIDI_MULTI_JACKID_IN_EMB(15, _numcables_in)

#define TUD_MIDI_MULTI_JACKID_OUT_EMB_0(...)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_1(...) TUD_MIDI_MULTI_JACKID_OUT_EMB(0)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_2(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_1(), TUD_MIDI_MULTI_JACKID_OUT_EMB(1)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_3(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_2(), TUD_MIDI_MULTI_JACKID_OUT_EMB(2)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_4(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_3(), TUD_MIDI_MULTI_JACKID_OUT_EMB(3)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_5(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_4(), TUD_MIDI_MULTI_JACKID_OUT_EMB(4)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_6(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_5(), TUD_MIDI_MULTI_JACKID_OUT_EMB(5)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_7(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_6(), TUD_MIDI_MULTI_JACKID_OUT_EMB(6)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_8(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_7(), TUD_MIDI_MULTI_JACKID_OUT_EMB(7)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_9(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_8(), TUD_MIDI_MULTI_JACKID_OUT_EMB(8)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_10(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_9(), TUD_MIDI_MULTI_JACKID_OUT_EMB(9)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_11(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_10(), TUD_MIDI_MULTI_JACKID_OUT_EMB(10)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_12(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_11(), TUD_MIDI_MULTI_JACKID_OUT_EMB(11)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_13(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_12(), TUD_MIDI_MULTI_JACKID_OUT_EMB(12)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_14(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_13(), TUD_MIDI_MULTI_JACKID_OUT_EMB(13)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_15(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_14(), TUD_MIDI_MULTI_JACKID_OUT_EMB(14)
#define TUD_MIDI_MULTI_JACKID_OUT_EMB_16(...) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_15(), TUD_MIDI_MULTI_JACKID_OUT_EMB(15)

#define TUD_MIDI_MULTI_DESC_JACKID_IN_EMB_X(_numcables_out, _numcables_in) \
    TUD_MIDI_MULTI_JACKID_IN_EMB_##_numcables_out(_numcables_in)
#define TUD_MIDI_MULTI_DESC_JACKID_OUT_EMB_X(_numcables_in) \
    TUD_MIDI_MULTI_JACKID_OUT_EMB_##_numcables_in()

#define TUD_MIDI_MULTI_DESC_JACKID_IN_EMB(_numcables_out, _numcables_in) \
    TUD_MIDI_MULTI_DESC_JACKID_IN_EMB_X(_numcables_out, _numcables_in)
#define TUD_MIDI_MULTI_DESC_JACKID_OUT_EMB(_numcables_in) \
    TUD_MIDI_MULTI_DESC_JACKID_OUT_EMB_X(_numcables_in)

#define TUD_MIDI_MULTI_DESC_HEAD(                                     \
    _itfnum, _stridx, _numcables_in, _numcables_out)                  \
    /* Audio Control (AC) Interface */                                \
    9, TUSB_DESC_INTERFACE, _itfnum, 0, 0, TUSB_CLASS_AUDIO,          \
        AUDIO_SUBCLASS_CONTROL, AUDIO_FUNC_PROTOCOL_CODE_UNDEF,       \
        _stridx, /* AC Header */                                      \
        9, TUSB_DESC_CS_INTERFACE, AUDIO_CS_AC_INTERFACE_HEADER,      \
        U16_TO_U8S_LE(0x0100), U16_TO_U8S_LE(0x0009), 1,              \
        (uint8_t)((_itfnum) + 1), /* MIDI Streaming (MS) Interface */ \
        9, TUSB_DESC_INTERFACE, (uint8_t)((_itfnum) + 1), 0, 2,       \
        TUSB_CLASS_AUDIO, AUDIO_SUBCLASS_MIDI_STREAMING,              \
        AUDIO_FUNC_PROTOCOL_CODE_UNDEF, 0, /* MS Header */            \
        7, TUSB_DESC_CS_INTERFACE, MIDI_CS_INTERFACE_HEADER,          \
        U16_TO_U8S_LE(0x0100),                                        \
        U16_TO_U8S_LE(7 + TUD_MIDI_MULTI_DESC_JACK_LEN(_numcables_in) \
                      + TUD_MIDI_MULTI_DESC_JACK_LEN(_numcables_out)  \
                      + TUD_MIDI_DESC_EP_LEN(_numcables_in)           \
                      + TUD_MIDI_DESC_EP_LEN(_numcables_out))

// MIDI multi-stream descriptor.
// - _itfnum is the interface number for the configuration descriptor
// - _stridx is the index of the string that describes the interface
// - _epout is the address of the Bulk OUT endpoint
// - _epin is the address of the Bulk IN endpoint
// - _epsize is the maximum transfer size for the Bulk endpoints
// - _numcables_in Number of Embedded IN Jacks connected to corresponding External Jack Out (routes to the host OUT endpoint)
// - _numcables_out Number of Embedded OUT Jacks connected to corresponding External Jack In (routes to the Host IN endpoint)
#define TUD_MIDI_MULTI_DESCRIPTOR(                                             \
    _itfnum, _stridx, _epout, _epin, _epsize, _numcables_in, _numcables_out)   \
    TUD_MIDI_MULTI_DESC_HEAD(_itfnum, _stridx, _numcables_in, _numcables_out), \
        TUD_MIDI_MULTI_DESC_JACK_DESC(_numcables_in, _numcables_out),          \
        TUD_MIDI_DESC_EP(_epout, _epsize, _numcables_out),                     \
        TUD_MIDI_MULTI_DESC_JACKID_IN_EMB(_numcables_out, _numcables_in),      \
        TUD_MIDI_DESC_EP(_epin, _epsize, _numcables_in),                       \
        TUD_MIDI_MULTI_DESC_JACKID_OUT_EMB(_numcables_in)

// Return the number of bytes read in the stream and set *cable_num to the cable number in the stream.
// Return 0 when when there are no more streams or stream fragments in the receive FIFO
// If cable_num is NULL, then this function behaves like to tud_midi_stream_read()
uint32_t
tud_midi_demux_stream_read(uint8_t* cable_num, void* buffer, uint32_t bufsize);
