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

#ifndef _PI_COLORS_H
#define _PI_COLORS_H

/* =---- Output Colors and Styles ------------------------------= */

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
#define PI_RESET            PI_Color("0")

/* Set the text color to white. */
#define PI_WHITE            PI_Color("97")
/* Set the text color to light gray. */
#define PI_LIGHT_GRAY       PI_Color("37")
/* Set the text color to gray. */
#define PI_GRAY             PI_Color("90")
/* Set the text color to black. */
#define PI_BLACK            PI_Color("30")

/* Set the text color to red. */
#define PI_RED              PI_Color("91")
/* Set the text color to green. */
#define PI_GREEN            PI_Color("92")
/* Set the text color to yellow. */
#define PI_YELLOW           PI_Color("93")
/* Set the text color to blue. */
#define PI_BLUE             PI_Color("94")
/* Set the text color to magenta. */
#define PI_MAGENTA          PI_Color("95")
/* Set the text color to cyan. */
#define PI_CYAN             PI_Color("96")

/* Set the text color to dark red. */
#define PI_DARK_RED         PI_Color("31")
/* Set the text color to dark green. */
#define PI_DARK_GREEN       PI_Color("32")
/* Set the text color to dark yellow. */
#define PI_DARK_YELLOW      PI_Color("33")
/* Set the text color to dark blue. */
#define PI_DARK_BLUE        PI_Color("34")
/* Set the text color to dark magenta. */
#define PI_DARK_MAGENTA     PI_Color("35")
/* Set the text color to dark cyan. */
#define PI_DARK_CYAN        PI_Color("36")

/* Adds bold style to text. */
#define PI_BOLD             PI_Color("1")
/* Adds darkened style to text. */
#define PI_DARKENED         PI_Color("2")
/* Adds italic style to text. */
#define PI_ITALIC           PI_Color("3")
/* Adds underlined style to text. */
#define PI_UNDERLINED       PI_Color("4")
/* Adds blinking style to text. */
#define PI_BLINK            PI_Color("5")
/* Adds reversed colors style to text. */
#define PI_REVERSED         PI_Color("7")
/* Adds invisible style to text. */
#define PI_INVISIBLE        PI_Color("8")
/* Adds removed style to text. */
#define PI_REMOVED          PI_Color("9")

/* =------------------------------------------------------------= */

#endif
