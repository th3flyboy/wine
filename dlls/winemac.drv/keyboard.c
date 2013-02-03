/*
 * MACDRV keyboard driver
 *
 * Copyright 1993 Bob Amstadt
 * Copyright 1996 Albrecht Kleine
 * Copyright 1997 David Faure
 * Copyright 1998 Morten Welinder
 * Copyright 1998 Ulrich Weigand
 * Copyright 1999 Ove Kåven
 * Copyright 2011, 2012, 2013 Ken Thomases for CodeWeavers Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"

#include "macdrv.h"
#include "winuser.h"
#include "wine/unicode.h"

WINE_DEFAULT_DEBUG_CHANNEL(keyboard);
WINE_DECLARE_DEBUG_CHANNEL(key);


/* Carbon-style modifier mask definitions from <Carbon/HIToolbox/Events.h>. */
enum {
    cmdKeyBit       = 8,
    shiftKeyBit     = 9,
    alphaLockBit    = 10,
    optionKeyBit    = 11,
    controlKeyBit   = 12,
};

enum {
    cmdKey      = 1 << cmdKeyBit,
    shiftKey    = 1 << shiftKeyBit,
    alphaLock   = 1 << alphaLockBit,
    optionKey   = 1 << optionKeyBit,
    controlKey  = 1 << controlKeyBit,
};


/* Mac virtual key code definitions from <Carbon/HIToolbox/Events.h>. */
enum {
    kVK_ANSI_A              = 0x00,
    kVK_ANSI_S              = 0x01,
    kVK_ANSI_D              = 0x02,
    kVK_ANSI_F              = 0x03,
    kVK_ANSI_H              = 0x04,
    kVK_ANSI_G              = 0x05,
    kVK_ANSI_Z              = 0x06,
    kVK_ANSI_X              = 0x07,
    kVK_ANSI_C              = 0x08,
    kVK_ANSI_V              = 0x09,
    kVK_ISO_Section         = 0x0A,
    kVK_ANSI_B              = 0x0B,
    kVK_ANSI_Q              = 0x0C,
    kVK_ANSI_W              = 0x0D,
    kVK_ANSI_E              = 0x0E,
    kVK_ANSI_R              = 0x0F,
    kVK_ANSI_Y              = 0x10,
    kVK_ANSI_T              = 0x11,
    kVK_ANSI_1              = 0x12,
    kVK_ANSI_2              = 0x13,
    kVK_ANSI_3              = 0x14,
    kVK_ANSI_4              = 0x15,
    kVK_ANSI_6              = 0x16,
    kVK_ANSI_5              = 0x17,
    kVK_ANSI_Equal          = 0x18,
    kVK_ANSI_9              = 0x19,
    kVK_ANSI_7              = 0x1A,
    kVK_ANSI_Minus          = 0x1B,
    kVK_ANSI_8              = 0x1C,
    kVK_ANSI_0              = 0x1D,
    kVK_ANSI_RightBracket   = 0x1E,
    kVK_ANSI_O              = 0x1F,
    kVK_ANSI_U              = 0x20,
    kVK_ANSI_LeftBracket    = 0x21,
    kVK_ANSI_I              = 0x22,
    kVK_ANSI_P              = 0x23,
    kVK_Return              = 0x24,
    kVK_ANSI_L              = 0x25,
    kVK_ANSI_J              = 0x26,
    kVK_ANSI_Quote          = 0x27,
    kVK_ANSI_K              = 0x28,
    kVK_ANSI_Semicolon      = 0x29,
    kVK_ANSI_Backslash      = 0x2A,
    kVK_ANSI_Comma          = 0x2B,
    kVK_ANSI_Slash          = 0x2C,
    kVK_ANSI_N              = 0x2D,
    kVK_ANSI_M              = 0x2E,
    kVK_ANSI_Period         = 0x2F,
    kVK_Tab                 = 0x30,
    kVK_Space               = 0x31,
    kVK_ANSI_Grave          = 0x32,
    kVK_Delete              = 0x33,
    kVK_Escape              = 0x35,
    kVK_RightCommand        = 0x36, /* invented for Wine; co-opt unused key code */
    kVK_Command             = 0x37,
    kVK_Shift               = 0x38,
    kVK_CapsLock            = 0x39,
    kVK_Option              = 0x3A,
    kVK_Control             = 0x3B,
    kVK_RightShift          = 0x3C,
    kVK_RightOption         = 0x3D,
    kVK_RightControl        = 0x3E,
    kVK_Function            = 0x3F,
    kVK_F17                 = 0x40,
    kVK_ANSI_KeypadDecimal  = 0x41,
    kVK_ANSI_KeypadMultiply = 0x43,
    kVK_ANSI_KeypadPlus     = 0x45,
    kVK_ANSI_KeypadClear    = 0x47,
    kVK_VolumeUp            = 0x48,
    kVK_VolumeDown          = 0x49,
    kVK_Mute                = 0x4A,
    kVK_ANSI_KeypadDivide   = 0x4B,
    kVK_ANSI_KeypadEnter    = 0x4C,
    kVK_ANSI_KeypadMinus    = 0x4E,
    kVK_F18                 = 0x4F,
    kVK_F19                 = 0x50,
    kVK_ANSI_KeypadEquals   = 0x51,
    kVK_ANSI_Keypad0        = 0x52,
    kVK_ANSI_Keypad1        = 0x53,
    kVK_ANSI_Keypad2        = 0x54,
    kVK_ANSI_Keypad3        = 0x55,
    kVK_ANSI_Keypad4        = 0x56,
    kVK_ANSI_Keypad5        = 0x57,
    kVK_ANSI_Keypad6        = 0x58,
    kVK_ANSI_Keypad7        = 0x59,
    kVK_F20                 = 0x5A,
    kVK_ANSI_Keypad8        = 0x5B,
    kVK_ANSI_Keypad9        = 0x5C,
    kVK_JIS_Yen             = 0x5D,
    kVK_JIS_Underscore      = 0x5E,
    kVK_JIS_KeypadComma     = 0x5F,
    kVK_F5                  = 0x60,
    kVK_F6                  = 0x61,
    kVK_F7                  = 0x62,
    kVK_F3                  = 0x63,
    kVK_F8                  = 0x64,
    kVK_F9                  = 0x65,
    kVK_JIS_Eisu            = 0x66,
    kVK_F11                 = 0x67,
    kVK_JIS_Kana            = 0x68,
    kVK_F13                 = 0x69,
    kVK_F16                 = 0x6A,
    kVK_F14                 = 0x6B,
    kVK_F10                 = 0x6D,
    kVK_F12                 = 0x6F,
    kVK_F15                 = 0x71,
    kVK_Help                = 0x72,
    kVK_Home                = 0x73,
    kVK_PageUp              = 0x74,
    kVK_ForwardDelete       = 0x75,
    kVK_F4                  = 0x76,
    kVK_End                 = 0x77,
    kVK_F2                  = 0x78,
    kVK_PageDown            = 0x79,
    kVK_F1                  = 0x7A,
    kVK_LeftArrow           = 0x7B,
    kVK_RightArrow          = 0x7C,
    kVK_DownArrow           = 0x7D,
    kVK_UpArrow             = 0x7E,
};


/* Indexed by Mac virtual keycode values defined above. */
static const struct {
    WORD vkey;
    WORD scan;
    BOOL fixed;
} default_map[128] = {
    { 'A',                      0x1E,           FALSE },    /* kVK_ANSI_A */
    { 'S',                      0x1F,           FALSE },    /* kVK_ANSI_S */
    { 'D',                      0x20,           FALSE },    /* kVK_ANSI_D */
    { 'F',                      0x21,           FALSE },    /* kVK_ANSI_F */
    { 'H',                      0x23,           FALSE },    /* kVK_ANSI_H */
    { 'G',                      0x22,           FALSE },    /* kVK_ANSI_G */
    { 'Z',                      0x2C,           FALSE },    /* kVK_ANSI_Z */
    { 'X',                      0x2D,           FALSE },    /* kVK_ANSI_X */
    { 'C',                      0x2E,           FALSE },    /* kVK_ANSI_C */
    { 'V',                      0x2F,           FALSE },    /* kVK_ANSI_V */
    { VK_OEM_102,               0x56,           TRUE },     /* kVK_ISO_Section */
    { 'B',                      0x30,           FALSE },    /* kVK_ANSI_B */
    { 'Q',                      0x10,           FALSE },    /* kVK_ANSI_Q */
    { 'W',                      0x11,           FALSE },    /* kVK_ANSI_W */
    { 'E',                      0x12,           FALSE },    /* kVK_ANSI_E */
    { 'R',                      0x13,           FALSE },    /* kVK_ANSI_R */
    { 'Y',                      0x15,           FALSE },    /* kVK_ANSI_Y */
    { 'T',                      0x14,           FALSE },    /* kVK_ANSI_T */
    { '1',                      0x02,           FALSE },    /* kVK_ANSI_1 */
    { '2',                      0x03,           FALSE },    /* kVK_ANSI_2 */
    { '3',                      0x04,           FALSE },    /* kVK_ANSI_3 */
    { '4',                      0x05,           FALSE },    /* kVK_ANSI_4 */
    { '6',                      0x07,           FALSE },    /* kVK_ANSI_6 */
    { '5',                      0x06,           FALSE },    /* kVK_ANSI_5 */
    { VK_OEM_PLUS,              0x0D,           FALSE },    /* kVK_ANSI_Equal */
    { '9',                      0x0A,           FALSE },    /* kVK_ANSI_9 */
    { '7',                      0x08,           FALSE },    /* kVK_ANSI_7 */
    { VK_OEM_MINUS,             0x0C,           FALSE },    /* kVK_ANSI_Minus */
    { '8',                      0x09,           FALSE },    /* kVK_ANSI_8 */
    { '0',                      0x0B,           FALSE },    /* kVK_ANSI_0 */
    { VK_OEM_6,                 0x1B,           FALSE },    /* kVK_ANSI_RightBracket */
    { 'O',                      0x18,           FALSE },    /* kVK_ANSI_O */
    { 'U',                      0x16,           FALSE },    /* kVK_ANSI_U */
    { VK_OEM_4,                 0x1A,           FALSE },    /* kVK_ANSI_LeftBracket */
    { 'I',                      0x17,           FALSE },    /* kVK_ANSI_I */
    { 'P',                      0x19,           FALSE },    /* kVK_ANSI_P */
    { VK_RETURN,                0x1C,           TRUE },     /* kVK_Return */
    { 'L',                      0x26,           FALSE },    /* kVK_ANSI_L */
    { 'J',                      0x24,           FALSE },    /* kVK_ANSI_J */
    { VK_OEM_7,                 0x28,           FALSE },    /* kVK_ANSI_Quote */
    { 'K',                      0x25,           FALSE },    /* kVK_ANSI_K */
    { VK_OEM_1,                 0x27,           FALSE },    /* kVK_ANSI_Semicolon */
    { VK_OEM_5,                 0x2B,           FALSE },    /* kVK_ANSI_Backslash */
    { VK_OEM_COMMA,             0x33,           FALSE },    /* kVK_ANSI_Comma */
    { VK_OEM_2,                 0x35,           FALSE },    /* kVK_ANSI_Slash */
    { 'N',                      0x31,           FALSE },    /* kVK_ANSI_N */
    { 'M',                      0x32,           FALSE },    /* kVK_ANSI_M */
    { VK_OEM_PERIOD,            0x34,           FALSE },    /* kVK_ANSI_Period */
    { VK_TAB,                   0x0F,           TRUE },     /* kVK_Tab */
    { VK_SPACE,                 0x39,           TRUE },     /* kVK_Space */
    { VK_OEM_3,                 0x29,           FALSE },    /* kVK_ANSI_Grave */
    { VK_BACK,                  0x0E,           TRUE },     /* kVK_Delete */
    { 0,                        0,              FALSE },    /* 0x34 unused */
    { VK_ESCAPE,                0x01,           TRUE },     /* kVK_Escape */
    { VK_RMENU,                 0x38 | 0x100,   TRUE },     /* kVK_RightCommand */
    { VK_LMENU,                 0x38,           TRUE },     /* kVK_Command */
    { VK_LSHIFT,                0x2A,           TRUE },     /* kVK_Shift */
    { VK_CAPITAL,               0x3A,           TRUE },     /* kVK_CapsLock */
    { 0,                        0,              FALSE },    /* kVK_Option */
    { VK_LCONTROL,              0x1D,           TRUE },     /* kVK_Control */
    { VK_RSHIFT,                0x36,           TRUE },     /* kVK_RightShift */
    { 0,                        0,              FALSE },    /* kVK_RightOption */
    { VK_RCONTROL,              0x1D | 0x100,   TRUE },     /* kVK_RightControl */
    { 0,                        0,              FALSE },    /* kVK_Function */
    { VK_F17,                   0x68,           TRUE },     /* kVK_F17 */
    { VK_DECIMAL,               0x53,           TRUE },     /* kVK_ANSI_KeypadDecimal */
    { 0,                        0,              FALSE },    /* 0x42 unused */
    { VK_MULTIPLY,              0x37,           TRUE },     /* kVK_ANSI_KeypadMultiply */
    { 0,                        0,              FALSE },    /* 0x44 unused */
    { VK_ADD,                   0x4E,           TRUE },     /* kVK_ANSI_KeypadPlus */
    { 0,                        0,              FALSE },    /* 0x46 unused */
    { VK_OEM_CLEAR,             0x59,           TRUE },     /* kVK_ANSI_KeypadClear */
    { VK_VOLUME_UP,             0 | 0x100,      TRUE },     /* kVK_VolumeUp */
    { VK_VOLUME_DOWN,           0 | 0x100,      TRUE },     /* kVK_VolumeDown */
    { VK_VOLUME_MUTE,           0 | 0x100,      TRUE },     /* kVK_Mute */
    { VK_DIVIDE,                0x35 | 0x100,   TRUE },     /* kVK_ANSI_KeypadDivide */
    { VK_RETURN,                0x1C | 0x100,   TRUE },     /* kVK_ANSI_KeypadEnter */
    { 0,                        0,              FALSE },    /* 0x4D unused */
    { VK_SUBTRACT,              0x4A,           TRUE },     /* kVK_ANSI_KeypadMinus */
    { VK_F18,                   0x69,           TRUE },     /* kVK_F18 */
    { VK_F19,                   0x6A,           TRUE },     /* kVK_F19 */
    { VK_OEM_NEC_EQUAL,         0x0D | 0x100,   TRUE },     /* kVK_ANSI_KeypadEquals */
    { VK_NUMPAD0,               0x52,           TRUE },     /* kVK_ANSI_Keypad0 */
    { VK_NUMPAD1,               0x4F,           TRUE },     /* kVK_ANSI_Keypad1 */
    { VK_NUMPAD2,               0x50,           TRUE },     /* kVK_ANSI_Keypad2 */
    { VK_NUMPAD3,               0x51,           TRUE },     /* kVK_ANSI_Keypad3 */
    { VK_NUMPAD4,               0x4B,           TRUE },     /* kVK_ANSI_Keypad4 */
    { VK_NUMPAD5,               0x4C,           TRUE },     /* kVK_ANSI_Keypad5 */
    { VK_NUMPAD6,               0x4D,           TRUE },     /* kVK_ANSI_Keypad6 */
    { VK_NUMPAD7,               0x47,           TRUE },     /* kVK_ANSI_Keypad7 */
    { VK_F20,                   0x6B,           TRUE },     /* kVK_F20 */
    { VK_NUMPAD8,               0x48,           TRUE },     /* kVK_ANSI_Keypad8 */
    { VK_NUMPAD9,               0x49,           TRUE },     /* kVK_ANSI_Keypad9 */
    { 0xFF,                     0x7D,           TRUE },     /* kVK_JIS_Yen */
    { 0xC1,                     0x73,           TRUE },     /* kVK_JIS_Underscore */
    { VK_SEPARATOR,             0x7E,           TRUE },     /* kVK_JIS_KeypadComma */
    { VK_F5,                    0x3F,           TRUE },     /* kVK_F5 */
    { VK_F6,                    0x40,           TRUE },     /* kVK_F6 */
    { VK_F7,                    0x41,           TRUE },     /* kVK_F7 */
    { VK_F3,                    0x3D,           TRUE },     /* kVK_F3 */
    { VK_F8,                    0x42,           TRUE },     /* kVK_F8 */
    { VK_F9,                    0x43,           TRUE },     /* kVK_F9 */
    { 0xFF,                     0x72,           TRUE },     /* kVK_JIS_Eisu */
    { VK_F11,                   0x57,           TRUE },     /* kVK_F11 */
    { VK_OEM_RESET,             0x71,           TRUE },     /* kVK_JIS_Kana */
    { VK_F13,                   0x64,           TRUE },     /* kVK_F13 */
    { VK_F16,                   0x67,           TRUE },     /* kVK_F16 */
    { VK_F14,                   0x65,           TRUE },     /* kVK_F14 */
    { 0,                        0,              FALSE },    /* 0x6C unused */
    { VK_F10,                   0x44,           TRUE },     /* kVK_F10 */
    { 0,                        0,              FALSE },    /* 0x6E unused */
    { VK_F12,                   0x58,           TRUE },     /* kVK_F12 */
    { 0,                        0,              FALSE },    /* 0x70 unused */
    { VK_F15,                   0x66,           TRUE },     /* kVK_F15 */
    { VK_INSERT,                0x52 | 0x100,   TRUE },     /* kVK_Help */ /* map to Insert */
    { VK_HOME,                  0x47 | 0x100,   TRUE },     /* kVK_Home */
    { VK_PRIOR,                 0x49 | 0x100,   TRUE },     /* kVK_PageUp */
    { VK_DELETE,                0x53 | 0x100,   TRUE },     /* kVK_ForwardDelete */
    { VK_F4,                    0x3E,           TRUE },     /* kVK_F4 */
    { VK_END,                   0x4F | 0x100,   TRUE },     /* kVK_End */
    { VK_F2,                    0x3C,           TRUE },     /* kVK_F2 */
    { VK_NEXT,                  0x51 | 0x100,   TRUE },     /* kVK_PageDown */
    { VK_F1,                    0x3B,           TRUE },     /* kVK_F1 */
    { VK_LEFT,                  0x4B | 0x100,   TRUE },     /* kVK_LeftArrow */
    { VK_RIGHT,                 0x4D | 0x100,   TRUE },     /* kVK_RightArrow */
    { VK_DOWN,                  0x50 | 0x100,   TRUE },     /* kVK_DownArrow */
    { VK_UP,                    0x48 | 0x100,   TRUE },     /* kVK_UpArrow */
};


static BOOL char_matches_string(WCHAR wchar, UniChar *string, BOOL ignore_diacritics)
{
    BOOL ret;
    CFStringRef s1 = CFStringCreateWithCharactersNoCopy(NULL, (UniChar*)&wchar, 1, kCFAllocatorNull);
    CFStringRef s2 = CFStringCreateWithCharactersNoCopy(NULL, string, strlenW(string), kCFAllocatorNull);
    CFStringCompareFlags flags = kCFCompareCaseInsensitive | kCFCompareNonliteral | kCFCompareWidthInsensitive;
    if (ignore_diacritics)
        flags |= kCFCompareDiacriticInsensitive;
    ret = (CFStringCompare(s1, s2, flags) == kCFCompareEqualTo);
    CFRelease(s1);
    CFRelease(s2);
    return ret;
}


/***********************************************************************
 *              macdrv_compute_keyboard_layout
 */
void macdrv_compute_keyboard_layout(struct macdrv_thread_data *thread_data)
{
    int keyc;
    WCHAR vkey;
    const UCKeyboardLayout *uchr;
    const UInt32 modifier_combos[] = {
        0,
        shiftKey >> 8,
        cmdKey >> 8,
        (shiftKey | cmdKey) >> 8,
        optionKey >> 8,
        (shiftKey | optionKey) >> 8,
    };
    UniChar map[128][sizeof(modifier_combos) / sizeof(modifier_combos[0])][4 + 1];
    int combo;
    BYTE vkey_used[256];
    int ignore_diacritics;
    static const struct {
        WCHAR wchar;
        DWORD vkey;
    } symbol_vkeys[] = {
        { '-', VK_OEM_MINUS },
        { '+', VK_OEM_PLUS },
        { '_', VK_OEM_MINUS },
        { ',', VK_OEM_COMMA },
        { '.', VK_OEM_PERIOD },
        { '=', VK_OEM_PLUS },
        { '>', VK_OEM_PERIOD },
        { '<', VK_OEM_COMMA },
        { '|', VK_OEM_5 },
        { '\\', VK_OEM_5 },
        { '`', VK_OEM_3 },
        { '[', VK_OEM_4 },
        { '~', VK_OEM_3 },
        { '?', VK_OEM_2 },
        { ']', VK_OEM_6 },
        { '/', VK_OEM_2 },
        { ':', VK_OEM_1 },
        { '}', VK_OEM_6 },
        { '{', VK_OEM_4 },
        { ';', VK_OEM_1 },
        { '\'', VK_OEM_7 },
        { ':', VK_OEM_PERIOD },
        { ';', VK_OEM_COMMA },
        { '"', VK_OEM_7 },
        { 0x00B4, VK_OEM_4 }, /* 0x00B4 is ACUTE ACCENT */
        { '\'', VK_OEM_2 },
        { 0x00A7, VK_OEM_5 }, /* 0x00A7 is SECTION SIGN */
        { '*', VK_OEM_PLUS },
        { 0x00B4, VK_OEM_7 },
        { '`', VK_OEM_4 },
        { '[', VK_OEM_6 },
        { '/', VK_OEM_5 },
        { '^', VK_OEM_6 },
        { '*', VK_OEM_2 },
        { '{', VK_OEM_6 },
        { '~', VK_OEM_1 },
        { '?', VK_OEM_PLUS },
        { '?', VK_OEM_4 },
        { 0x00B4, VK_OEM_3 },
        { '?', VK_OEM_COMMA },
        { '~', VK_OEM_PLUS },
        { ']', VK_OEM_4 },
        { '\'', VK_OEM_3 },
        { 0x00A7, VK_OEM_7 },
    };
    int i;

    /* Vkeys that are suitable for assigning to arbitrary keys, organized in
       contiguous ranges. */
    static const struct {
        WORD first, last;
    } vkey_ranges[] = {
        { 'A', 'Z' },
        { '0', '9' },
        { VK_OEM_1, VK_OEM_3 },
        { VK_OEM_4, VK_ICO_CLEAR },
        { 0xe9, 0xf5 },
        { VK_OEM_NEC_EQUAL, VK_OEM_NEC_EQUAL },
        { VK_F1, VK_F24 },
        { 0, 0 }
    };
    int vkey_range;

    if (!thread_data->keyboard_layout_uchr)
    {
        ERR("no keyboard layout UCHR data\n");
        return;
    }

    memset(thread_data->keyc2vkey, 0, sizeof(thread_data->keyc2vkey));
    memset(vkey_used, 0, sizeof(vkey_used));

    for (keyc = 0; keyc < sizeof(default_map) / sizeof(default_map[0]); keyc++)
    {
        thread_data->keyc2scan[keyc] = default_map[keyc].scan;
        if (default_map[keyc].fixed)
        {
            vkey = default_map[keyc].vkey;
            thread_data->keyc2vkey[keyc] = vkey;
            vkey_used[vkey] = 1;
            TRACE("keyc 0x%04x -> vkey 0x%04x (fixed)\n", keyc, vkey);
        }
    }

    if (thread_data->iso_keyboard)
    {
        /* In almost all cases, the Mac key codes indicate a physical key position
           and this corresponds nicely to Win32 scan codes.  However, the Mac key
           codes differ in one case between ANSI and ISO keyboards.  For ANSI
           keyboards, the key to the left of the digits and above the Tab key
           produces key code kVK_ANSI_Grave.  For ISO keyboards, the key in that
           some position produces kVK_ISO_Section.  The additional key on ISO
           keyboards, the one to the right of the left Shift key, produces
           kVK_ANSI_Grave, which is just weird.

           Since we want the key in that upper left corner to always produce the
           same scan code (0x29), we need to swap the scan codes of those two
           Mac key codes for ISO keyboards. */
        DWORD temp = thread_data->keyc2scan[kVK_ANSI_Grave];
        thread_data->keyc2scan[kVK_ANSI_Grave] = thread_data->keyc2scan[kVK_ISO_Section];
        thread_data->keyc2scan[kVK_ISO_Section] = temp;
    }

    uchr = (const UCKeyboardLayout*)CFDataGetBytePtr(thread_data->keyboard_layout_uchr);

    /* Using the keyboard layout, build a map of key code + modifiers -> characters. */
    memset(map, 0, sizeof(map));
    for (keyc = 0; keyc < sizeof(map) / sizeof(map[0]); keyc++)
    {
        if (!thread_data->keyc2scan[keyc]) continue; /* not a known Mac key code */
        if (thread_data->keyc2vkey[keyc]) continue; /* assigned a fixed vkey */

        TRACE("keyc 0x%04x: ", keyc);

        for (combo = 0; combo < sizeof(modifier_combos) / sizeof(modifier_combos[0]); combo++)
        {
            UInt32 deadKeyState;
            UniCharCount len;
            OSStatus status;

            deadKeyState = 0;
            status = UCKeyTranslate(uchr, keyc, kUCKeyActionDown, modifier_combos[combo],
                thread_data->keyboard_type, kUCKeyTranslateNoDeadKeysMask,
                &deadKeyState, sizeof(map[keyc][combo])/sizeof(map[keyc][combo][0]) - 1,
                &len, map[keyc][combo]);
            if (status != noErr)
                map[keyc][combo][0] = 0;

            TRACE("%s%s", (combo ? ", " : ""), debugstr_w(map[keyc][combo]));
        }

        TRACE("\n");
    }

    /* First try to match key codes to the vkeys for the letters A through Z.
       Try unmodified first, then with various modifier combinations in succession.
       On the first pass, try to get a match lacking diacritical marks.  On the
       second pass, accept matches with diacritical marks. */
    for (ignore_diacritics = 0; ignore_diacritics <= 1; ignore_diacritics++)
    {
        for (combo = 0; combo < sizeof(modifier_combos) / sizeof(modifier_combos[0]); combo++)
        {
            for (vkey = 'A'; vkey <= 'Z'; vkey++)
            {
                if (vkey_used[vkey])
                    continue;

                for (keyc = 0; keyc < sizeof(map) / sizeof(map[0]); keyc++)
                {
                    if (thread_data->keyc2vkey[keyc] || !map[keyc][combo][0])
                        continue;

                    if (char_matches_string(vkey, map[keyc][combo], ignore_diacritics))
                    {
                        thread_data->keyc2vkey[keyc] = vkey;
                        vkey_used[vkey] = 1;
                        TRACE("keyc 0x%04x -> vkey 0x%04x (%s match %s)\n", keyc, vkey,
                              debugstr_wn(&vkey, 1), debugstr_w(map[keyc][combo]));
                        break;
                    }
                }
            }
        }
    }

    /* Next try to match key codes to the vkeys for the digits 0 through 9. */
    for (combo = 0; combo < sizeof(modifier_combos) / sizeof(modifier_combos[0]); combo++)
    {
        for (vkey = '0'; vkey <= '9'; vkey++)
        {
            if (vkey_used[vkey])
                continue;

            for (keyc = 0; keyc < sizeof(map) / sizeof(map[0]); keyc++)
            {
                if (thread_data->keyc2vkey[keyc] || !map[keyc][combo][0])
                    continue;

                if (char_matches_string(vkey, map[keyc][combo], FALSE))
                {
                    thread_data->keyc2vkey[keyc] = vkey;
                    vkey_used[vkey] = 1;
                    TRACE("keyc 0x%04x -> vkey 0x%04x (%s match %s)\n", keyc, vkey,
                          debugstr_wn(&vkey, 1), debugstr_w(map[keyc][combo]));
                    break;
                }
            }
        }
    }

    /* Now try to match key codes for certain common punctuation characters to
       the most common OEM vkeys (e.g. '.' to VK_OEM_PERIOD). */
    for (i = 0; i < sizeof(symbol_vkeys) / sizeof(symbol_vkeys[0]); i++)
    {
        vkey = symbol_vkeys[i].vkey;

        if (vkey_used[vkey])
            continue;

        for (combo = 0; combo < sizeof(modifier_combos) / sizeof(modifier_combos[0]); combo++)
        {
            for (keyc = 0; keyc < sizeof(map) / sizeof(map[0]); keyc++)
            {
                if (!thread_data->keyc2scan[keyc]) continue; /* not a known Mac key code */
                if (thread_data->keyc2vkey[keyc] || !map[keyc][combo][0])
                    continue;

                if (char_matches_string(symbol_vkeys[i].wchar, map[keyc][combo], FALSE))
                {
                    thread_data->keyc2vkey[keyc] = vkey;
                    vkey_used[vkey] = 1;
                    TRACE("keyc 0x%04x -> vkey 0x%04x (%s match %s)\n", keyc, vkey,
                          debugstr_wn(&symbol_vkeys[i].wchar, 1), debugstr_w(map[keyc][combo]));
                    break;
                }
            }

            if (vkey_used[vkey])
                break;
        }
    }

    /* For those key codes still without a vkey, try to use the default vkey
       from the default map, if it's still available. */
    for (keyc = 0; keyc < sizeof(default_map) / sizeof(default_map[0]); keyc++)
    {
        DWORD vkey = default_map[keyc].vkey;

        if (!thread_data->keyc2scan[keyc]) continue; /* not a known Mac key code */
        if (thread_data->keyc2vkey[keyc]) continue; /* already assigned */

        if (!vkey_used[vkey])
        {
            thread_data->keyc2vkey[keyc] = vkey;
            vkey_used[vkey] = 1;
            TRACE("keyc 0x%04x -> vkey 0x%04x (default map)\n", keyc, vkey);
        }
    }

    /* For any unassigned key codes which would map to a letter in the default
       map, but whose normal letter vkey wasn't available, try to find a
       different letter. */
    vkey = 'A';
    for (keyc = 0; keyc < sizeof(default_map) / sizeof(default_map[0]); keyc++)
    {
        if (default_map[keyc].vkey < 'A' || 'Z' < default_map[keyc].vkey)
            continue; /* not a letter in ANSI layout */
        if (!thread_data->keyc2scan[keyc]) continue; /* not a known Mac key code */
        if (thread_data->keyc2vkey[keyc]) continue; /* already assigned */

        while (vkey <= 'Z' && vkey_used[vkey]) vkey++;
        if (vkey <= 'Z')
        {
            thread_data->keyc2vkey[keyc] = vkey;
            vkey_used[vkey] = 1;
            TRACE("keyc 0x%04x -> vkey 0x%04x (spare letter)\n", keyc, vkey);
        }
        else
            break; /* no more unused letter vkeys, so stop trying */
    }

    /* Same thing but with the digits. */
    vkey = '0';
    for (keyc = 0; keyc < sizeof(default_map) / sizeof(default_map[0]); keyc++)
    {
        if (default_map[keyc].vkey < '0' || '9' < default_map[keyc].vkey)
            continue; /* not a digit in ANSI layout */
        if (!thread_data->keyc2scan[keyc]) continue; /* not a known Mac key code */
        if (thread_data->keyc2vkey[keyc]) continue; /* already assigned */

        while (vkey <= '9' && vkey_used[vkey]) vkey++;
        if (vkey <= '9')
        {
            thread_data->keyc2vkey[keyc] = vkey;
            vkey_used[vkey] = 1;
            TRACE("keyc 0x%04x -> vkey 0x%04x (spare digit)\n", keyc, vkey);
        }
        else
            break; /* no more unused digit vkeys, so stop trying */
    }

    /* Last chance.  Assign any available vkey. */
    vkey_range = 0;
    vkey = vkey_ranges[vkey_range].first;
    for (keyc = 0; keyc < sizeof(default_map) / sizeof(default_map[0]); keyc++)
    {
        if (!thread_data->keyc2scan[keyc]) continue; /* not a known Mac key code */
        if (thread_data->keyc2vkey[keyc]) continue; /* already assigned */

        while (vkey && vkey_used[vkey])
        {
            if (vkey == vkey_ranges[vkey_range].last)
            {
                vkey_range++;
                vkey = vkey_ranges[vkey_range].first;
            }
            else
                vkey++;
        }

        if (!vkey)
        {
            WARN("No more vkeys available!\n");
            break;
        }

        thread_data->keyc2vkey[keyc] = vkey;
        vkey_used[vkey] = 1;
        TRACE("keyc 0x%04x -> vkey 0x%04x (spare vkey)\n", keyc, vkey);
    }
}


/***********************************************************************
 *              macdrv_send_keyboard_input
 */
static void macdrv_send_keyboard_input(HWND hwnd, WORD vkey, WORD scan, DWORD flags, DWORD time)
{
    INPUT input;

    TRACE_(key)("hwnd %p vkey=%04x scan=%04x flags=%04x\n", hwnd, vkey, scan, flags);

    input.type              = INPUT_KEYBOARD;
    input.ki.wVk            = vkey;
    input.ki.wScan          = scan;
    input.ki.dwFlags        = flags;
    input.ki.time           = time;
    input.ki.dwExtraInfo    = 0;

    __wine_send_input(hwnd, &input);
}


/***********************************************************************
 *              macdrv_key_event
 *
 * Handler for KEY_PRESS and KEY_RELEASE events.
 */
void macdrv_key_event(HWND hwnd, const macdrv_event *event)
{
    struct macdrv_thread_data *thread_data = macdrv_thread_data();
    WORD vkey, scan;
    DWORD flags;

    TRACE_(key)("win %p/%p key %s keycode %hu modifiers 0x%08llx\n",
                hwnd, event->window, (event->type == KEY_PRESS ? "press" : "release"),
                event->key.keycode, event->key.modifiers);

    if (event->key.keycode < sizeof(thread_data->keyc2vkey)/sizeof(thread_data->keyc2vkey[0]))
    {
        vkey = thread_data->keyc2vkey[event->key.keycode];
        scan = thread_data->keyc2scan[event->key.keycode];
    }
    else
        vkey = scan = 0;

    TRACE_(key)("keycode %hu converted to vkey 0x%X scan 0x%02x\n",
                event->key.keycode, vkey, scan);

    if (!vkey) return;

    flags = 0;
    if (event->type == KEY_RELEASE) flags |= KEYEVENTF_KEYUP;
    if (scan & 0x100)               flags |= KEYEVENTF_EXTENDEDKEY;

    macdrv_send_keyboard_input(hwnd, vkey, scan & 0xff, flags, event->key.time_ms);
}


/***********************************************************************
 *              macdrv_keyboard_changed
 *
 * Handler for KEYBOARD_CHANGED events.
 */
void macdrv_keyboard_changed(const macdrv_event *event)
{
    struct macdrv_thread_data *thread_data = macdrv_thread_data();

    TRACE("new keyboard layout uchr data %p, type %u, iso %d\n", event->keyboard_changed.uchr,
          event->keyboard_changed.keyboard_type, event->keyboard_changed.iso_keyboard);

    if (thread_data->keyboard_layout_uchr)
        CFRelease(thread_data->keyboard_layout_uchr);
    thread_data->keyboard_layout_uchr = CFDataCreateCopy(NULL, event->keyboard_changed.uchr);
    thread_data->keyboard_type = event->keyboard_changed.keyboard_type;
    thread_data->iso_keyboard = event->keyboard_changed.iso_keyboard;

    macdrv_compute_keyboard_layout(thread_data);
}