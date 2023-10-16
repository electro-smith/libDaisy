#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

// CUSTOM AUDIO CLASS DESC
// Unit numbers are arbitrary selected
#define UAC2_ENTITY_CLOCK 0x04
// Speaker path
#define UAC2_ENTITY_SPK_INPUT_TERMINAL 0x01
#define UAC2_ENTITY_SPK_FEATURE_UNIT 0x02
#define UAC2_ENTITY_SPK_OUTPUT_TERMINAL 0x03
// Microphone path
#define UAC2_ENTITY_MIC_INPUT_TERMINAL 0x11
#define UAC2_ENTITY_MIC_OUTPUT_TERMINAL 0x13

#define TUD_AUDIO_HEADSET_STEREO_DESC_LEN                                  \
    (TUD_AUDIO_DESC_IAD_LEN + TUD_AUDIO_DESC_STD_AC_LEN                    \
     + TUD_AUDIO_DESC_CS_AC_LEN + TUD_AUDIO_DESC_CLK_SRC_LEN               \
     + TUD_AUDIO_DESC_INPUT_TERM_LEN                                       \
     + TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL_LEN                         \
     + TUD_AUDIO_DESC_OUTPUT_TERM_LEN + TUD_AUDIO_DESC_INPUT_TERM_LEN      \
     + TUD_AUDIO_DESC_OUTPUT_TERM_LEN /* Interface 1, Alternate 0 */       \
     + TUD_AUDIO_DESC_STD_AS_INT_LEN  /* Interface 1, Alternate 0 */       \
     + TUD_AUDIO_DESC_STD_AS_INT_LEN + TUD_AUDIO_DESC_CS_AS_INT_LEN        \
     + TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN + TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN \
     + TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN /* Interface 1, Alternate 2 */      \
     + TUD_AUDIO_DESC_STD_AS_INT_LEN + TUD_AUDIO_DESC_CS_AS_INT_LEN        \
     + TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN + TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN \
     + TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN /* Interface 2, Alternate 0 */      \
     + TUD_AUDIO_DESC_STD_AS_INT_LEN   /* Interface 2, Alternate 1 */      \
     + TUD_AUDIO_DESC_STD_AS_INT_LEN + TUD_AUDIO_DESC_CS_AS_INT_LEN        \
     + TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN + TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN \
     + TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN /* Interface 2, Alternate 2 */      \
     + TUD_AUDIO_DESC_STD_AS_INT_LEN + TUD_AUDIO_DESC_CS_AS_INT_LEN        \
     + TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN + TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN \
     + TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN)

#define TUD_AUDIO_HEADSET_STEREO_DESCRIPTOR(_itfnum, _stridx, _epout, _epin)                                                                   \
    /* Standard Interface Association Descriptor (IAD) */                                                                                      \
    TUD_AUDIO_DESC_IAD(                                                                                                                        \
        /*_firstitfs*/ _itfnum,                                                                                                                \
        /*_nitfs*/ 3,                                                                                                                          \
        /*_stridx*/ 0x00), /* Standard AC Interface Descriptor(4.7.1) */                                                                       \
        TUD_AUDIO_DESC_STD_AC(                                                                                                                 \
            /*_itfnum*/ _itfnum,                                                                                                               \
            /*_nEPs*/ 0x00,                                                                                                                    \
            /*_stridx*/                                                                                                                        \
            _stridx), /* Class-Specific AC Interface Header Descriptor(4.7.2) */                                                               \
        TUD_AUDIO_DESC_CS_AC(                                                                                                                  \
            /*_bcdADC*/ 0x0200,                                                                                                                \
            /*_category*/ AUDIO_FUNC_HEADSET,                                                                                                  \
            /*_totallen*/ TUD_AUDIO_DESC_CLK_SRC_LEN                                                                                           \
                + TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL_LEN                                                                                  \
                + TUD_AUDIO_DESC_INPUT_TERM_LEN                                                                                                \
                + TUD_AUDIO_DESC_OUTPUT_TERM_LEN                                                                                               \
                + TUD_AUDIO_DESC_INPUT_TERM_LEN                                                                                                \
                + TUD_AUDIO_DESC_OUTPUT_TERM_LEN,                                                                                              \
            /*_ctrl*/                                                                                                                          \
            AUDIO_CS_AS_INTERFACE_CTRL_LATENCY_POS), /* Clock Source Descriptor(4.7.2.1) */                                                    \
        TUD_AUDIO_DESC_CLK_SRC(                                                                                                                \
            /*_clkid*/ UAC2_ENTITY_CLOCK,                                                                                                      \
            /*_attr*/ 3,                                                                                                                       \
            /*_ctrl*/ 7,                                                                                                                       \
            /*_assocTerm*/ 0x00,                                                                                                               \
            /*_stridx*/ 0x00), /* Input Terminal Descriptor(4.7.2.4) */                                                                        \
        TUD_AUDIO_DESC_INPUT_TERM(                                                                                                             \
            /*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL,                                                                                        \
            /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING,                                                                                       \
            /*_assocTerm*/ 0x00,                                                                                                               \
            /*_clkid*/ UAC2_ENTITY_CLOCK,                                                                                                      \
            /*_nchannelslogical*/ 0x02,                                                                                                        \
            /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,                                                                               \
            /*_idxchannelnames*/ 0x00,                                                                                                         \
            /*_ctrl*/ 0 * (AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS),                                                                  \
            /*_stridx*/ 0x00), /* Feature Unit Descriptor(4.7.2.8) */                                                                          \
        TUD_AUDIO_DESC_FEATURE_UNIT_TWO_CHANNEL(                                                                                               \
            /*_unitid*/ UAC2_ENTITY_SPK_FEATURE_UNIT,                                                                                          \
            /*_srcid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL,                                                                                         \
            /*_ctrlch0master*/                                                                                                                 \
            (AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS                                                                                 \
             | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS),                                                                           \
            /*_ctrlch1*/                                                                                                                       \
            (AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS                                                                                 \
             | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS),                                                                           \
            /*_ctrlch2*/                                                                                                                       \
            (AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS                                                                                 \
             | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS),                                                                           \
            /*_stridx*/ 0x00), /* Output Terminal Descriptor(4.7.2.5) */                                                                       \
        TUD_AUDIO_DESC_OUTPUT_TERM(                                                                                                            \
            /*_termid*/ UAC2_ENTITY_SPK_OUTPUT_TERMINAL,                                                                                       \
            /*_termtype*/ AUDIO_TERM_TYPE_OUT_HEADPHONES,                                                                                      \
            /*_assocTerm*/ 0x00,                                                                                                               \
            /*_srcid*/ UAC2_ENTITY_SPK_FEATURE_UNIT,                                                                                           \
            /*_clkid*/ UAC2_ENTITY_CLOCK,                                                                                                      \
            /*_ctrl*/ 0x0000,                                                                                                                  \
            /*_stridx*/ 0x00), /* Input Terminal Descriptor(4.7.2.4) */                                                                        \
        TUD_AUDIO_DESC_INPUT_TERM(                                                                                                             \
            /*_termid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL,                                                                                        \
            /*_termtype*/ AUDIO_TERM_TYPE_IN_GENERIC_MIC,                                                                                      \
            /*_assocTerm*/ 0x00,                                                                                                               \
            /*_clkid*/ UAC2_ENTITY_CLOCK,                                                                                                      \
            /*_nchannelslogical*/ 0x01,                                                                                                        \
            /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,                                                                               \
            /*_idxchannelnames*/ 0x00,                                                                                                         \
            /*_ctrl*/ 0 * (AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS),                                                                  \
            /*_stridx*/ 0x00), /* Output Terminal Descriptor(4.7.2.5) */                                                                       \
        TUD_AUDIO_DESC_OUTPUT_TERM(                                                                                                            \
            /*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL,                                                                                       \
            /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING,                                                                                       \
            /*_assocTerm*/ 0x00,                                                                                                               \
            /*_srcid*/ UAC2_ENTITY_MIC_INPUT_TERMINAL,                                                                                         \
            /*_clkid*/ UAC2_ENTITY_CLOCK,                                                                                                      \
            /*_ctrl*/ 0x0000,                                                                                                                  \
            /*_stridx*/                                                                                                                        \
            0x00), /* Standard AS Interface Descriptor(4.9.1) */ /* Interface 1, Alternate 0 - default alternate setting with 0 bandwidth */   \
        TUD_AUDIO_DESC_STD_AS_INT(                                                                                                             \
            /*_itfnum*/ (uint8_t)(_itfnum + 1),                                                                                                \
            /*_altset*/ 0x00,                                                                                                                  \
            /*_nEPs*/ 0x00,                                                                                                                    \
            /*_stridx*/                                                                                                                        \
            0x05), /* Standard AS Interface Descriptor(4.9.1) */ /* Interface 1, Alternate 1 - alternate interface for data streaming */       \
        TUD_AUDIO_DESC_STD_AS_INT(                                                                                                             \
            /*_itfnum*/ (uint8_t)(_itfnum + 1),                                                                                                \
            /*_altset*/ 0x01,                                                                                                                  \
            /*_nEPs*/ 0x01,                                                                                                                    \
            /*_stridx*/                                                                                                                        \
            0x05), /* Class-Specific AS Interface Descriptor(4.9.2) */                                                                         \
        TUD_AUDIO_DESC_CS_AS_INT(                                                                                                              \
            /*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL,                                                                                        \
            /*_ctrl*/ AUDIO_CTRL_NONE,                                                                                                         \
            /*_formattype*/ AUDIO_FORMAT_TYPE_I,                                                                                               \
            /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM,                                                                                         \
            /*_nchannelsphysical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX,                                                                         \
            /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,                                                                               \
            /*_stridx*/                                                                                                                        \
            0x00), /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */                                                                \
        TUD_AUDIO_DESC_TYPE_I_FORMAT(                                                                                                          \
            CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX,                                                                               \
            CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX), /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */               \
        TUD_AUDIO_DESC_STD_AS_ISO_EP(                                                                                                          \
            /*_ep*/ _epout,                                                                                                                    \
            /*_attr*/                                                                                                                          \
            (uint8_t)(TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ADAPTIVE                                                                         \
                      | TUSB_ISO_EP_ATT_DATA),                                                                                                 \
            /*_maxEPsize*/                                                                                                                     \
            TUD_AUDIO_EP_SIZE(                                                                                                                 \
                CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                                                                                          \
                CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX,                                                                           \
                CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX),                                                                                           \
            /*_interval*/                                                                                                                      \
            0x01), /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */                                                \
        TUD_AUDIO_DESC_CS_AS_ISO_EP(                                                                                                           \
            /*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK,                                                                          \
            /*_ctrl*/ AUDIO_CTRL_NONE,                                                                                                         \
            /*_lockdelayunit*/                                                                                                                 \
            AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_MILLISEC,                                                                                  \
            /*_lockdelay*/                                                                                                                     \
            0x0001), /* Interface 1, Alternate 2 - alternate interface for data streaming */                                                   \
        TUD_AUDIO_DESC_STD_AS_INT(                                                                                                             \
            /*_itfnum*/ (uint8_t)(_itfnum + 1),                                                                                                \
            /*_altset*/ 0x02,                                                                                                                  \
            /*_nEPs*/ 0x01,                                                                                                                    \
            /*_stridx*/                                                                                                                        \
            0x05), /* Class-Specific AS Interface Descriptor(4.9.2) */                                                                         \
        TUD_AUDIO_DESC_CS_AS_INT(                                                                                                              \
            /*_termid*/ UAC2_ENTITY_SPK_INPUT_TERMINAL,                                                                                        \
            /*_ctrl*/ AUDIO_CTRL_NONE,                                                                                                         \
            /*_formattype*/ AUDIO_FORMAT_TYPE_I,                                                                                               \
            /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM,                                                                                         \
            /*_nchannelsphysical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX,                                                                         \
            /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,                                                                               \
            /*_stridx*/                                                                                                                        \
            0x00), /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */                                                                \
        TUD_AUDIO_DESC_TYPE_I_FORMAT(                                                                                                          \
            CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_RX,                                                                               \
            CFG_TUD_AUDIO_FUNC_1_FORMAT_2_RESOLUTION_RX), /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */               \
        TUD_AUDIO_DESC_STD_AS_ISO_EP(                                                                                                          \
            /*_ep*/ _epout,                                                                                                                    \
            /*_attr*/                                                                                                                          \
            (uint8_t)(TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ADAPTIVE                                                                         \
                      | TUSB_ISO_EP_ATT_DATA),                                                                                                 \
            /*_maxEPsize*/                                                                                                                     \
            TUD_AUDIO_EP_SIZE(                                                                                                                 \
                CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                                                                                          \
                CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_RX,                                                                           \
                CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX),                                                                                           \
            /*_interval*/                                                                                                                      \
            0x01), /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */                                                \
        TUD_AUDIO_DESC_CS_AS_ISO_EP(                                                                                                           \
            /*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK,                                                                          \
            /*_ctrl*/ AUDIO_CTRL_NONE,                                                                                                         \
            /*_lockdelayunit*/                                                                                                                 \
            AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_MILLISEC,                                                                                  \
            /*_lockdelay*/                                                                                                                     \
            0x0001), /* Standard AS Interface Descriptor(4.9.1) */ /* Interface 2, Alternate 0 - default alternate setting with 0 bandwidth */ \
        TUD_AUDIO_DESC_STD_AS_INT(                                                                                                             \
            /*_itfnum*/ (uint8_t)(_itfnum + 2),                                                                                                \
            /*_altset*/ 0x00,                                                                                                                  \
            /*_nEPs*/ 0x00,                                                                                                                    \
            /*_stridx*/                                                                                                                        \
            0x04), /* Standard AS Interface Descriptor(4.9.1) */ /* Interface 2, Alternate 1 - alternate interface for data streaming */       \
        TUD_AUDIO_DESC_STD_AS_INT(                                                                                                             \
            /*_itfnum*/ (uint8_t)(_itfnum + 2),                                                                                                \
            /*_altset*/ 0x01,                                                                                                                  \
            /*_nEPs*/ 0x01,                                                                                                                    \
            /*_stridx*/                                                                                                                        \
            0x04), /* Class-Specific AS Interface Descriptor(4.9.2) */                                                                         \
        TUD_AUDIO_DESC_CS_AS_INT(                                                                                                              \
            /*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL,                                                                                       \
            /*_ctrl*/ AUDIO_CTRL_NONE,                                                                                                         \
            /*_formattype*/ AUDIO_FORMAT_TYPE_I,                                                                                               \
            /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM,                                                                                         \
            /*_nchannelsphysical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX,                                                                         \
            /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,                                                                               \
            /*_stridx*/                                                                                                                        \
            0x00), /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */                                                                \
        TUD_AUDIO_DESC_TYPE_I_FORMAT(                                                                                                          \
            CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX,                                                                               \
            CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_TX), /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */               \
        TUD_AUDIO_DESC_STD_AS_ISO_EP(                                                                                                          \
            /*_ep*/ _epin,                                                                                                                     \
            /*_attr*/                                                                                                                          \
            (uint8_t)(TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ASYNCHRONOUS                                                                     \
                      | TUSB_ISO_EP_ATT_DATA),                                                                                                 \
            /*_maxEPsize*/                                                                                                                     \
            TUD_AUDIO_EP_SIZE(                                                                                                                 \
                CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                                                                                          \
                CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX,                                                                           \
                CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX),                                                                                           \
            /*_interval*/                                                                                                                      \
            0x01), /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */                                                \
        TUD_AUDIO_DESC_CS_AS_ISO_EP(                                                                                                           \
            /*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK,                                                                          \
            /*_ctrl*/ AUDIO_CTRL_NONE,                                                                                                         \
            /*_lockdelayunit*/                                                                                                                 \
            AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED,                                                                                 \
            /*_lockdelay*/                                                                                                                     \
            0x0000), /* Interface 2, Alternate 2 - alternate interface for data streaming */                                                   \
        TUD_AUDIO_DESC_STD_AS_INT(                                                                                                             \
            /*_itfnum*/ (uint8_t)(_itfnum + 2),                                                                                                \
            /*_altset*/ 0x02,                                                                                                                  \
            /*_nEPs*/ 0x01,                                                                                                                    \
            /*_stridx*/                                                                                                                        \
            0x04), /* Class-Specific AS Interface Descriptor(4.9.2) */                                                                         \
        TUD_AUDIO_DESC_CS_AS_INT(                                                                                                              \
            /*_termid*/ UAC2_ENTITY_MIC_OUTPUT_TERMINAL,                                                                                       \
            /*_ctrl*/ AUDIO_CTRL_NONE,                                                                                                         \
            /*_formattype*/ AUDIO_FORMAT_TYPE_I,                                                                                               \
            /*_formats*/ AUDIO_DATA_FORMAT_TYPE_I_PCM,                                                                                         \
            /*_nchannelsphysical*/ CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX,                                                                         \
            /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED,                                                                               \
            /*_stridx*/                                                                                                                        \
            0x00), /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */                                                                \
        TUD_AUDIO_DESC_TYPE_I_FORMAT(                                                                                                          \
            CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_TX,                                                                               \
            CFG_TUD_AUDIO_FUNC_1_FORMAT_2_RESOLUTION_TX), /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */               \
        TUD_AUDIO_DESC_STD_AS_ISO_EP(                                                                                                          \
            /*_ep*/ _epin,                                                                                                                     \
            /*_attr*/                                                                                                                          \
            (uint8_t)(TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ASYNCHRONOUS                                                                     \
                      | TUSB_ISO_EP_ATT_DATA),                                                                                                 \
            /*_maxEPsize*/                                                                                                                     \
            TUD_AUDIO_EP_SIZE(                                                                                                                 \
                CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                                                                                          \
                CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_TX,                                                                           \
                CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX),                                                                                           \
            /*_interval*/                                                                                                                      \
            0x01), /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */                                                \
        TUD_AUDIO_DESC_CS_AS_ISO_EP(                                                                                                           \
            /*_attr*/ AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK,                                                                          \
            /*_ctrl*/ AUDIO_CTRL_NONE,                                                                                                         \
            /*_lockdelayunit*/                                                                                                                 \
            AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED,                                                                                 \
            /*_lockdelay*/ 0x0000)

//--------------------------------------------------------------------+
// Board Specific Configuration
//--------------------------------------------------------------------+

// RHPort number used for device can be defined by board.mk, default to port 0
#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT 0
#endif

// RHPort max operational speed can defined by board.mk
#ifndef BOARD_TUD_MAX_SPEED
#define BOARD_TUD_MAX_SPEED OPT_MODE_DEFAULT_SPEED
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

// defined by compiler flags for flexibility
#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU OPT_MCU_STM32H7
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_NONE
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG 0
#endif

// Enable Device stack
#define CFG_TUD_ENABLED 1

// Default is max speed that hardware controller could support with on-chip PHY
#define CFG_TUD_MAX_SPEED BOARD_TUD_MAX_SPEED

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))
#endif

    //--------------------------------------------------------------------
    // DEVICE CONFIGURATION
    //--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE 64
#endif

//------------- CLASS -------------//
#define CFG_TUD_AUDIO 1
#define CFG_TUD_HID 0
#define CFG_TUD_CDC 1
#define CFG_TUD_MSC 0
#define CFG_TUD_MIDI 1
#define CFG_TUD_VENDOR 0

#define CFG_TUD_CDC_RX_BUFSIZE (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define CFG_TUD_CDC_TX_BUFSIZE (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define CFG_TUD_CDC_EP_BUFSIZE (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define CFG_TUD_MIDI_RX_BUFSIZE (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define CFG_TUD_MIDI_TX_BUFSIZE (TUD_OPT_HIGH_SPEED ? 512 : 64)

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN TUD_AUDIO_HEADSET_STEREO_DESC_LEN

// How many formats are used, need to adjust USB descriptor if changed
#define CFG_TUD_AUDIO_FUNC_1_N_FORMATS 2
#define __RX__ 1

// Audio format type I specifications
#if defined(__RX__)
#define CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE \
    96000 // 16bit/48kHz is the best quality for Renesas RX
    // 48000 // 16bit/48kHz is the best quality for Renesas RX
#else
#define CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE \
    96000 // 24bit/96kHz is the best quality for full-speed, high-speed is needed beyond this
#endif
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX 1
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX 2 // Changed

// 16bit in 16bit slots
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX 2
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_TX 16
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX 2
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_RESOLUTION_RX 16

#if defined(__RX__)
// 8bit in 8bit slots
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_TX 1
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_RESOLUTION_TX 8
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_RX 1
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_RESOLUTION_RX 8
#else
// 24bit in 32bit slots
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_TX 4
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_RESOLUTION_TX 24
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_RX 4
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_RESOLUTION_RX 24
#endif

// EP and buffer size - for isochronous EP´s, the buffer and EP size are equal (different sizes would not make sense)
#define CFG_TUD_AUDIO_ENABLE_EP_IN 1

#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN                             \
    TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                \
                      CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_TX, \
                      CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_2_EP_SZ_IN                             \
    TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                \
                      CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_TX, \
                      CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)

#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ       \
    TU_MAX(CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN, \
           CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX       \
    TU_MAX(                                     \
        CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN, \
        CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN) // Maximum EP IN size for all AS alternate settings used

// EP and buffer size - for isochronous EP´s, the buffer and EP size are equal (different sizes would not make sense)
#define CFG_TUD_AUDIO_ENABLE_EP_OUT 1

#define CFG_TUD_AUDIO_UNC_1_FORMAT_1_EP_SZ_OUT                             \
    TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                \
                      CFG_TUD_AUDIO_FUNC_1_FORMAT_1_N_BYTES_PER_SAMPLE_RX, \
                      CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX)
#define CFG_TUD_AUDIO_UNC_1_FORMAT_2_EP_SZ_OUT                             \
    TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_FUNC_1_MAX_SAMPLE_RATE,                \
                      CFG_TUD_AUDIO_FUNC_1_FORMAT_2_N_BYTES_PER_SAMPLE_RX, \
                      CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX)

#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ      \
    TU_MAX(CFG_TUD_AUDIO_UNC_1_FORMAT_1_EP_SZ_OUT, \
           CFG_TUD_AUDIO_UNC_1_FORMAT_1_EP_SZ_OUT)
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX      \
    TU_MAX(                                     \
        CFG_TUD_AUDIO_UNC_1_FORMAT_1_EP_SZ_OUT, \
        CFG_TUD_AUDIO_UNC_1_FORMAT_1_EP_SZ_OUT) // Maximum EP IN size for all AS alternate settings used

// Number of Standard AS Interface Descriptors (4.9.1) defined per audio function - this is required to be able to remember the current alternate settings of these interfaces - We restrict us here to have a constant number for all audio functions (which means this has to be the maximum number of AS interfaces an audio function has and a second audio function with less AS interfaces just wastes a few bytes)
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT 2

// Size of control request buffer
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ 64

    // CDC FIFO size of TX and RX
    // #define CFG_TUD_CDC_RX_BUFSIZE                    64
    // #define CFG_TUD_CDC_TX_BUFSIZE                    64


#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
