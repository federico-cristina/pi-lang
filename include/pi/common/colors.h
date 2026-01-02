#pragma once

/**
 * @copyright   Copyright (c) 2025 Federico Cristina
 *
 *              Licensed under the Apache License, Version 2.0 (the "License");
 *              you may not use this file except in compliance with the License.
 *              You may obtain a copy of the License at
 *
 *                  http://www.apache.org/licenses/LICENSE-2.0
 *
 *              Unless required by applicable law or agreed to in writing, software
 *              distributed under the License is distributed on an "AS IS" BASIS,
 *              WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *              See the License for the specific language governing permissions and
 *              limitations under the License.
 */

#ifndef _PI_COMMON_COLORS_H
#define _PI_COMMON_COLORS_H

/* =---- Output Colors and Styles ------------------------------= */

/**
 * +---- Colors ---------------------------+
 */

#ifndef PI_Color
/**
 * @brief   This macro, when the `PI_USE_COLORS` option is enabled, constructs a string
 *          that, based on the 4-bit ANSI color encoding, changes the color of the text
 *          when printed in the terminal.
 */
#   if PI_USE_COLORS
#       define PI_Color(colorCode) "\033[" colorCode "m"
#   else
#       define PI_Color(colorCode) ""
#   endif
#endif

/* Resets the output color and style. */
#define PI_RESET                    PI_Color("0")

/* Set the text color to white. */
#define PI_WHITE                    PI_Color("97")
/* Set the text color to light gray. */
#define PI_LIGHT_GRAY               PI_Color("37")
/* Set the text color to gray. */
#define PI_GRAY                     PI_Color("90")
/* Set the text color to black. */
#define PI_BLACK                    PI_Color("30")

/* Set the text color to red. */
#define PI_RED                      PI_Color("91")
/* Set the text color to green. */
#define PI_GREEN                    PI_Color("92")
/* Set the text color to yellow. */
#define PI_YELLOW                   PI_Color("93")
/* Set the text color to blue. */
#define PI_BLUE                     PI_Color("94")
/* Set the text color to magenta. */
#define PI_MAGENTA                  PI_Color("95")
/* Set the text color to cyan. */
#define PI_CYAN                     PI_Color("96")

/* Set the text color to dark red. */
#define PI_DARK_RED                 PI_Color("31")
/* Set the text color to dark green. */
#define PI_DARK_GREEN               PI_Color("32")
/* Set the text color to dark yellow. */
#define PI_DARK_YELLOW              PI_Color("33")
/* Set the text color to dark blue. */
#define PI_DARK_BLUE                PI_Color("34")
/* Set the text color to dark magenta. */
#define PI_DARK_MAGENTA             PI_Color("35")
/* Set the text color to dark cyan. */
#define PI_DARK_CYAN                PI_Color("36")

/* Adds bold style to text. */
#define PI_BOLD                     PI_Color("1")
/* Adds darkened style to text. */
#define PI_DARKENED                 PI_Color("2")
/* Adds italic style to text. */
#define PI_ITALIC                   PI_Color("3")
/* Adds underlined style to text. */
#define PI_UNDERLINED               PI_Color("4")
/* Adds blinking style to text. */
#define PI_BLINK                    PI_Color("5")
/* Adds reversed colors style to text. */
#define PI_REVERSED                 PI_Color("7")
/* Adds invisible style to text. */
#define PI_INVISIBLE                PI_Color("8")
/* Adds removed style to text. */
#define PI_REMOVED                  PI_Color("9")

/**
 * +---- Stylized Colors ------------------+
 */

#ifndef PI_StylizedColor
/**
 * @brief   This macro, when the `PI_USE_COLORS` option is enabled, constructs a string
 *          that, based on the 4-bit ANSI color encoding, changes the and the style color
 *          of the text when printed in the terminal.
 */
#   if PI_USE_COLORS
#       define PI_StylizedColor(styleCode, colorCode) "\033[" styleCode ";" colorCode "m"
#   else
#       define PI_StylizedColor(styleCode, colorCode) ""
#   endif
#endif

/* Set the text color to bold white. */
#define PI_BOLD_WHITE               PI_StylizedColor("1", "97")
/* Set the text color to bold light gray. */
#define PI_BOLD_LIGHT_GRAY          PI_StylizedColor("1", "37")
/* Set the text color to bold gray. */
#define PI_BOLD_GRAY                PI_StylizedColor("1", "90")
/* Set the text color to bold black. */
#define PI_BOLD_BLACK               PI_StylizedColor("1", "30")

/* Set the text color to bold red. */
#define PI_BOLD_RED                 PI_StylizedColor("1", "91")
/* Set the text color to bold green. */
#define PI_BOLD_GREEN               PI_StylizedColor("1", "92")
/* Set the text color to bold yellow. */
#define PI_BOLD_YELLOW              PI_StylizedColor("1", "93")
/* Set the text color to bold blue. */
#define PI_BOLD_BLUE                PI_StylizedColor("1", "94")
/* Set the text color to bold magenta. */
#define PI_BOLD_MAGENTA             PI_StylizedColor("1", "95")
/* Set the text color to bold cyan. */
#define PI_BOLD_CYAN                PI_StylizedColor("1", "96")

/* Set the text color to bold dark red. */
#define PI_BOLD_DARK_RED            PI_StylizedColor("1", "31")
/* Set the text color to bold dark green. */
#define PI_BOLD_DARK_GREEN          PI_StylizedColor("1", "32")
/* Set the text color to bold dark yellow. */
#define PI_BOLD_DARK_YELLOW         PI_StylizedColor("1", "33")
/* Set the text color to bold dark blue. */
#define PI_BOLD_DARK_BLUE           PI_StylizedColor("1", "34")
/* Set the text color to bold dark magenta. */
#define PI_BOLD_DARK_MAGENTA        PI_StylizedColor("1", "35")
/* Set the text color to bold dark cyan. */
#define PI_BOLD_DARK_CYAN           PI_StylizedColor("1", "36")

/* Set the text color to darkened white. */
#define PI_DARKENED_WHITE           PI_StylizedColor("2", "97")
/* Set the text color to darkened light gray. */
#define PI_DARKENED_LIGHT_GRAY      PI_StylizedColor("2", "37")
/* Set the text color to darkened gray. */
#define PI_DARKENED_GRAY            PI_StylizedColor("2", "90")
/* Set the text color to darkened black. */
#define PI_DARKENED_BLACK           PI_StylizedColor("2", "30")

/* Set the text color to darkened red. */
#define PI_DARKENED_RED             PI_StylizedColor("2", "91")
/* Set the text color to darkened green. */
#define PI_DARKENED_GREEN           PI_StylizedColor("2", "92")
/* Set the text color to darkened yellow. */
#define PI_DARKENED_YELLOW          PI_StylizedColor("2", "93")
/* Set the text color to darkened blue. */
#define PI_DARKENED_BLUE            PI_StylizedColor("2", "94")
/* Set the text color to darkened magenta. */
#define PI_DARKENED_MAGENTA         PI_StylizedColor("2", "95")
/* Set the text color to darkened cyan. */
#define PI_DARKENED_CYAN            PI_StylizedColor("2", "96")

/* Set the text color to darkened dark red. */
#define PI_DARKENED_DARK_RED        PI_StylizedColor("2", "31")
/* Set the text color to darkened dark green. */
#define PI_DARKENED_DARK_GREEN      PI_StylizedColor("2", "32")
/* Set the text color to darkened dark yellow. */
#define PI_DARKENED_DARK_YELLOW     PI_StylizedColor("2", "33")
/* Set the text color to darkened dark blue. */
#define PI_DARKENED_DARK_BLUE       PI_StylizedColor("2", "34")
/* Set the text color to darkened dark magenta. */
#define PI_DARKENED_DARK_MAGENTA    PI_StylizedColor("2", "35")
/* Set the text color to darkened dark cyan. */
#define PI_DARKENED_DARK_CYAN       PI_StylizedColor("2", "36")

/* Set the text color to italic white. */
#define PI_ITALIC_WHITE             PI_StylizedColor("3", "97")
/* Set the text color to italic light gray. */
#define PI_ITALIC_LIGHT_GRAY        PI_StylizedColor("3", "37")
/* Set the text color to italic gray. */
#define PI_ITALIC_GRAY              PI_StylizedColor("3", "90")
/* Set the text color to italic black. */
#define PI_ITALIC_BLACK             PI_StylizedColor("3", "30")

/* Set the text color to italic red. */
#define PI_ITALIC_RED               PI_StylizedColor("3", "91")
/* Set the text color to italic green. */
#define PI_ITALIC_GREEN             PI_StylizedColor("3", "92")
/* Set the text color to italic yellow. */
#define PI_ITALIC_YELLOW            PI_StylizedColor("3", "93")
/* Set the text color to italic blue. */
#define PI_ITALIC_BLUE              PI_StylizedColor("3", "94")
/* Set the text color to italic magenta. */
#define PI_ITALIC_MAGENTA           PI_StylizedColor("3", "95")
/* Set the text color to italic cyan. */
#define PI_ITALIC_CYAN              PI_StylizedColor("3", "96")

/* Set the text color to italic dark red. */
#define PI_ITALIC_DARK_RED          PI_StylizedColor("3", "31")
/* Set the text color to italic dark green. */
#define PI_ITALIC_DARK_GREEN        PI_StylizedColor("3", "32")
/* Set the text color to italic dark yellow. */
#define PI_ITALIC_DARK_YELLOW       PI_StylizedColor("3", "33")
/* Set the text color to italic dark blue. */
#define PI_ITALIC_DARK_BLUE         PI_StylizedColor("3", "34")
/* Set the text color to italic dark magenta. */
#define PI_ITALIC_DARK_MAGENTA      PI_StylizedColor("3", "35")
/* Set the text color to italic dark cyan. */
#define PI_ITALIC_DARK_CYAN         PI_StylizedColor("3", "36")

/* Set the text color to underlined white. */
#define PI_UNDERLINED_WHITE         PI_StylizedColor("4", "97")
/* Set the text color to underlined light gray. */
#define PI_UNDERLINED_LIGHT_GRAY    PI_StylizedColor("4", "37")
/* Set the text color to underlined gray. */
#define PI_UNDERLINED_GRAY          PI_StylizedColor("4", "90")
/* Set the text color to underlined black. */
#define PI_UNDERLINED_BLACK         PI_StylizedColor("4", "30")

/* Set the text color to underlined red. */
#define PI_UNDERLINED_RED           PI_StylizedColor("4", "91")
/* Set the text color to underlined green. */
#define PI_UNDERLINED_GREEN         PI_StylizedColor("4", "92")
/* Set the text color to underlined yellow. */
#define PI_UNDERLINED_YELLOW        PI_StylizedColor("4", "93")
/* Set the text color to underlined blue. */
#define PI_UNDERLINED_BLUE          PI_StylizedColor("4", "94")
/* Set the text color to underlined magenta. */
#define PI_UNDERLINED_MAGENTA       PI_StylizedColor("4", "95")
/* Set the text color to underlined cyan. */
#define PI_UNDERLINED_CYAN          PI_StylizedColor("4", "96")

/* Set the text color to underlined dark red. */
#define PI_UNDERLINED_DARK_RED      PI_StylizedColor("4", "31")
/* Set the text color to underlined dark green. */
#define PI_UNDERLINED_DARK_GREEN    PI_StylizedColor("4", "32")
/* Set the text color to underlined dark yellow. */
#define PI_UNDERLINED_DARK_YELLOW   PI_StylizedColor("4", "33")
/* Set the text color to underlined dark blue. */
#define PI_UNDERLINED_DARK_BLUE     PI_StylizedColor("4", "34")
/* Set the text color to underlined dark magenta. */
#define PI_UNDERLINED_DARK_MAGENTA  PI_StylizedColor("4", "35")
/* Set the text color to underlined dark cyan. */
#define PI_UNDERLINED_DARK_CYAN      PI_StylizedColor("4", "36")

/* Set the text color to blink white. */
#define PI_BLINK_WHITE               PI_StylizedColor("5", "97")
/* Set the text color to blink light gray. */
#define PI_BLINK_LIGHT_GRAY          PI_StylizedColor("5", "37")
/* Set the text color to blink gray. */
#define PI_BLINK_GRAY                PI_StylizedColor("5", "90")
/* Set the text color to blink black. */
#define PI_BLINK_BLACK               PI_StylizedColor("5", "30")

/* Set the text color to blink red. */
#define PI_BLINK_RED                 PI_StylizedColor("5", "91")
/* Set the text color to blink green. */
#define PI_BLINK_GREEN               PI_StylizedColor("5", "92")
/* Set the text color to blink yellow. */
#define PI_BLINK_YELLOW              PI_StylizedColor("5", "93")
/* Set the text color to blink blue. */
#define PI_BLINK_BLUE                PI_StylizedColor("5", "94")
/* Set the text color to blink magenta. */
#define PI_BLINK_MAGENTA             PI_StylizedColor("5", "95")
/* Set the text color to blink cyan. */
#define PI_BLINK_CYAN                PI_StylizedColor("5", "96")

/* Set the text color to blink dark red. */
#define PI_BLINK_DARK_RED            PI_StylizedColor("5", "31")
/* Set the text color to blink dark green. */
#define PI_BLINK_DARK_GREEN          PI_StylizedColor("5", "32")
/* Set the text color to blink dark yellow. */
#define PI_BLINK_DARK_YELLOW         PI_StylizedColor("5", "33")
/* Set the text color to blink dark blue. */
#define PI_BLINK_DARK_BLUE           PI_StylizedColor("5", "34")
/* Set the text color to blink dark magenta. */
#define PI_BLINK_DARK_MAGENTA        PI_StylizedColor("5", "35")
/* Set the text color to blink dark cyan. */
#define PI_BLINK_DARK_CYAN           PI_StylizedColor("5", "36")

/* Set the text color to reversed white. */
#define PI_REVERSED_WHITE            PI_StylizedColor("7", "97")
/* Set the text color to reversed light gray. */
#define PI_REVERSED_LIGHT_GRAY       PI_StylizedColor("7", "37")
/* Set the text color to reversed gray. */
#define PI_REVERSED_GRAY             PI_StylizedColor("7", "90")
/* Set the text color to reversed black. */
#define PI_REVERSED_BLACK            PI_StylizedColor("7", "30")

/* Set the text color to reversed red. */
#define PI_REVERSED_RED              PI_StylizedColor("7", "91")
/* Set the text color to reversed green. */
#define PI_REVERSED_GREEN            PI_StylizedColor("7", "92")
/* Set the text color to reversed yellow. */
#define PI_REVERSED_YELLOW           PI_StylizedColor("7", "93")
/* Set the text color to reversed blue. */
#define PI_REVERSED_BLUE             PI_StylizedColor("7", "94")
/* Set the text color to reversed magenta. */
#define PI_REVERSED_MAGENTA          PI_StylizedColor("7", "95")
/* Set the text color to reversed cyan. */
#define PI_REVERSED_CYAN             PI_StylizedColor("7", "96")

/* Set the text color to reversed dark red. */
#define PI_REVERSED_DARK_RED         PI_StylizedColor("7", "31")
/* Set the text color to reversed dark green. */
#define PI_REVERSED_DARK_GREEN       PI_StylizedColor("7", "32")
/* Set the text color to reversed dark yellow. */
#define PI_REVERSED_DARK_YELLOW      PI_StylizedColor("7", "33")
/* Set the text color to reversed dark blue. */
#define PI_REVERSED_DARK_BLUE        PI_StylizedColor("7", "34")
/* Set the text color to reversed dark magenta. */
#define PI_REVERSED_DARK_MAGENTA     PI_StylizedColor("7", "35")
/* Set the text color to reversed dark cyan. */
#define PI_REVERSED_DARK_CYAN        PI_StylizedColor("7", "36")

/* Set the text color to invisible white. */
#define PI_INVISIBLE_WHITE           PI_StylizedColor("8", "97")
/* Set the text color to invisible light gray. */
#define PI_INVISIBLE_LIGHT_GRAY      PI_StylizedColor("8", "37")
/* Set the text color to invisible gray. */
#define PI_INVISIBLE_GRAY            PI_StylizedColor("8", "90")
/* Set the text color to invisible black. */
#define PI_INVISIBLE_BLACK           PI_StylizedColor("8", "30")

/* Set the text color to invisible red. */
#define PI_INVISIBLE_RED             PI_StylizedColor("8", "91")
/* Set the text color to invisible green. */
#define PI_INVISIBLE_GREEN           PI_StylizedColor("8", "92")
/* Set the text color to invisible yellow. */
#define PI_INVISIBLE_YELLOW          PI_StylizedColor("8", "93")
/* Set the text color to invisible blue. */
#define PI_INVISIBLE_BLUE            PI_StylizedColor("8", "94")
/* Set the text color to invisible magenta. */
#define PI_INVISIBLE_MAGENTA         PI_StylizedColor("8", "95")
/* Set the text color to invisible cyan. */
#define PI_INVISIBLE_CYAN            PI_StylizedColor("8", "96")

/* Set the text color to invisible dark red. */
#define PI_INVISIBLE_DARK_RED        PI_StylizedColor("8", "31")
/* Set the text color to invisible dark green. */
#define PI_INVISIBLE_DARK_GREEN      PI_StylizedColor("8", "32")
/* Set the text color to invisible dark yellow. */
#define PI_INVISIBLE_DARK_YELLOW     PI_StylizedColor("8", "33")
/* Set the text color to invisible dark blue. */
#define PI_INVISIBLE_DARK_BLUE       PI_StylizedColor("8", "34")
/* Set the text color to invisible dark magenta. */
#define PI_INVISIBLE_DARK_MAGENTA    PI_StylizedColor("8", "35")
/* Set the text color to invisible dark cyan. */
#define PI_INVISIBLE_DARK_CYAN       PI_StylizedColor("8", "36")

/* Set the text color to removed white. */
#define PI_REMOVED_WHITE             PI_StylizedColor("9", "97")
/* Set the text color to removed light gray. */
#define PI_REMOVED_LIGHT_GRAY        PI_StylizedColor("9", "37")
/* Set the text color to removed gray. */
#define PI_REMOVED_GRAY              PI_StylizedColor("9", "90")
/* Set the text color to removed black. */
#define PI_REMOVED_BLACK             PI_StylizedColor("9", "30")

/* Set the text color to removed red. */
#define PI_REMOVED_RED               PI_StylizedColor("9", "91")
/* Set the text color to removed green. */
#define PI_REMOVED_GREEN             PI_StylizedColor("9", "92")
/* Set the text color to removed yellow. */
#define PI_REMOVED_YELLOW            PI_StylizedColor("9", "93")
/* Set the text color to removed blue. */
#define PI_REMOVED_BLUE              PI_StylizedColor("9", "94")
/* Set the text color to removed magenta. */
#define PI_REMOVED_MAGENTA           PI_StylizedColor("9", "95")
/* Set the text color to removed cyan. */
#define PI_REMOVED_CYAN              PI_StylizedColor("9", "96")

/* Set the text color to removed dark red. */
#define PI_REMOVED_DARK_RED          PI_StylizedColor("9", "31")
/* Set the text color to removed dark green. */
#define PI_REMOVED_DARK_GREEN        PI_StylizedColor("9", "32")
/* Set the text color to removed dark yellow. */
#define PI_REMOVED_DARK_YELLOW       PI_StylizedColor("9", "33")
/* Set the text color to removed dark blue. */
#define PI_REMOVED_DARK_BLUE         PI_StylizedColor("9", "34")
/* Set the text color to removed dark magenta. */
#define PI_REMOVED_DARK_MAGENTA      PI_StylizedColor("9", "35")
/* Set the text color to removed dark cyan. */
#define PI_REMOVED_DARK_CYAN         PI_StylizedColor("9", "36")

/**
 * +---- Specific Colors ------------------+
 */

/* Set the colors of a single quoted char literal. */
#define PI_CharColor(c)             PI_DARK_YELLOW "'" c "'" PI_RESET
/* Set the colors of a single quoted escape sequence literal. */
#define PI_EscapedCharColor(c)      PI_DARK_YELLOW "'" PI_YELLOW c PI_DARK_YELLOW "'" PI_RESET

/* Set the colors of an erroneous slice of text. */
#define PI_ErroneousColor(text)     PI_RED text PI_RESET
/* Set the colors of an erroneous slice of text. */
#define PI_ErroneousColor2(text)    PI_BOLD_RED text PI_RESET

/* Set the colors of an invalid hexadecimal literal. */
#define PI_InvalidHexColor(hex)     PI_BLUE "'" PI_DARK_CYAN hex PI_BLUE "'" PI_RESET

/* =------------------------------------------------------------= */

#endif
