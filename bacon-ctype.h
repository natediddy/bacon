/*
 * bacon - A command line tool for viewing/downloading CyanogenMod ROMs
 *         for Android devices.
 *
 * Copyright (C) 2013  Nathan Forbes
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BACON_CTYPE_H
#define BACON_CTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#if 0
BaconBoolean bacon_isalpha (char c);
BaconBoolean bacon_isblank (char c);
BaconBoolean bacon_isdigit (char c);
BaconBoolean bacon_islower (char c);
BaconBoolean bacon_isspace (char c);
char bacon_tolower (char c);
#endif

/* The macros below are very basic <ctype.h>
   replacements that do not assume the ASCII character set. */

#define bacon_isalpha(c) \
  (((c) == 'A') || ((c) == 'B') || ((c) == 'C') || ((c) == 'D') || \
   ((c) == 'E') || ((c) == 'F') || ((c) == 'G') || ((c) == 'H') || \
   ((c) == 'I') || ((c) == 'J') || ((c) == 'K') || ((c) == 'L') || \
   ((c) == 'M') || ((c) == 'N') || ((c) == 'O') || ((c) == 'P') || \
   ((c) == 'Q') || ((c) == 'R') || ((c) == 'S') || ((c) == 'T') || \
   ((c) == 'U') || ((c) == 'V') || ((c) == 'W') || ((c) == 'X') || \
   ((c) == 'Y') || ((c) == 'Z') || ((c) == 'a') || ((c) == 'b') || \
   ((c) == 'c') || ((c) == 'd') || ((c) == 'e') || ((c) == 'f') || \
   ((c) == 'g') || ((c) == 'h') || ((c) == 'i') || ((c) == 'j') || \
   ((c) == 'k') || ((c) == 'l') || ((c) == 'm') || ((c) == 'n') || \
   ((c) == 'o') || ((c) == 'p') || ((c) == 'q') || ((c) == 'r') || \
   ((c) == 's') || ((c) == 't') || ((c) == 'u') || ((c) == 'v') || \
   ((c) == 'w') || ((c) == 'x') || ((c) == 'y') || ((c) == 'z'))

#define bacon_isblank(c) \
  (((c) == ' ') || ((c) == '\t'))

#define bacon_isdigit(c) \
  (((c) == '0') || ((c) == '1') || ((c) == '2') || ((c) == '3') || \
   ((c) == '4') || ((c) == '5') || ((c) == '6') || ((c) == '7') || \
   ((c) == '8') || ((c) == '9'))

#define bacon_islower(c) \
  (((c) == 'a') || ((c) == 'b') || ((c) == 'c') || ((c) == 'd') || \
   ((c) == 'e') || ((c) == 'f') || ((c) == 'g') || ((c) == 'h') || \
   ((c) == 'i') || ((c) == 'j') || ((c) == 'k') || ((c) == 'l') || \
   ((c) == 'm') || ((c) == 'n') || ((c) == 'o') || ((c) == 'p') || \
   ((c) == 'q') || ((c) == 'r') || ((c) == 's') || ((c) == 't') || \
   ((c) == 'u') || ((c) == 'v') || ((c) == 'w') || ((c) == 'x') || \
   ((c) == 'y') || ((c) == 'z'))

#define bacon_isspace(c) \
  (((c) == ' ') || ((c) == '\t') || ((c) == '\n') || ((c) == '\v') || \
   ((c) == '\f') || ((c) == '\r'))

/*
 * XXX: this can't be used with something like:
 *
 *          foo[i++] = bacon_tolower (bar[j++]);
 *
 *      Because it will increment 'j' until the character
 *      is found (or 26 times if not and leave the result empty).
 */
#define bacon_tolower(c) \
  (((c) == 'A') ? 'a' : ((c) == 'B') ? 'b' : ((c) == 'C') ? 'c' : \
   ((c) == 'D') ? 'd' : ((c) == 'E') ? 'e' : ((c) == 'F') ? 'f' : \
   ((c) == 'G') ? 'g' : ((c) == 'H') ? 'h' : ((c) == 'I') ? 'i' : \
   ((c) == 'J') ? 'j' : ((c) == 'K') ? 'k' : ((c) == 'L') ? 'l' : \
   ((c) == 'M') ? 'm' : ((c) == 'N') ? 'n' : ((c) == 'O') ? 'o' : \
   ((c) == 'P') ? 'p' : ((c) == 'Q') ? 'q' : ((c) == 'R') ? 'r' : \
   ((c) == 'S') ? 's' : ((c) == 'T') ? 't' : ((c) == 'U') ? 'u' : \
   ((c) == 'V') ? 'v' : ((c) == 'W') ? 'w' : ((c) == 'X') ? 'x' : \
   ((c) == 'Y') ? 'y' : ((c) == 'Z') ? 'z' : (c))

#ifdef __cplusplus
}
#endif

#endif /* BACON_CTYPE_H */

