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

#undef BACON_ASCII

#if (' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) && \
    ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) && \
    (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) && \
    ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) && \
    ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) && \
    ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) && \
    ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) && \
    ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) && \
    ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) && \
    ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) && \
    ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) && \
    ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) && \
    ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) && \
    ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) && \
    ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) && \
    ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) && \
    ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) && \
    ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) && \
    ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) && \
    ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) && \
    ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) && \
    ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) && \
    ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126)
# define BACON_ASCII
#endif


/* The macros below are very basic <ctype.h>
   replacements that do not assume the ASCII character set. */

#ifdef BACON_ASCII
# define bacon_isalpha(c) \
  (((((int) (c)) & ~0x20) >= 'A') && ((((int) (c)) & ~0x20) <= 'Z'))
#else
# define bacon_isalpha(c) \
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
#endif

#define bacon_isblank(c) \
  (((c) == ' ') || ((c) == '\t'))

#ifdef BACON_ASCII
# define bacon_isdigit(c) ((((int) (c)) >= '0') && (((int) (c)) <= '9'))
#else
# define bacon_isdigit(c) \
  (((c) == '0') || ((c) == '1') || ((c) == '2') || ((c) == '3') || \
   ((c) == '4') || ((c) == '5') || ((c) == '6') || ((c) == '7') || \
   ((c) == '8') || ((c) == '9'))
#endif

#ifdef BACON_ASCII
# define bacon_islower(c) ((((int) (c)) >= 'a') && (((int) (c)) <= 'z'))
#else
# define bacon_islower(c) \
  (((c) == 'a') || ((c) == 'b') || ((c) == 'c') || ((c) == 'd') || \
   ((c) == 'e') || ((c) == 'f') || ((c) == 'g') || ((c) == 'h') || \
   ((c) == 'i') || ((c) == 'j') || ((c) == 'k') || ((c) == 'l') || \
   ((c) == 'm') || ((c) == 'n') || ((c) == 'o') || ((c) == 'p') || \
   ((c) == 'q') || ((c) == 'r') || ((c) == 's') || ((c) == 't') || \
   ((c) == 'u') || ((c) == 'v') || ((c) == 'w') || ((c) == 'x') || \
   ((c) == 'y') || ((c) == 'z'))
#endif

#define bacon_isspace(c) \
  (((c) == ' ') || ((c) == '\t') || ((c) == '\n') || ((c) == '\v') || \
   ((c) == '\f') || ((c) == '\r'))

/*
 * XXX: this should never be used with something like:
 *          foo[i++] = bacon_tolower (bar[j++]);
 *      Because 'j' will be incremented at least a handful of times.
 */
#ifdef BACON_ASCII
# define bacon_tolower(c) \
  ((((int) (c)) >= 'A') && (((int) (c)) <= 'Z')) ? ((c) - 'A' + 'a') : (c)
#else
# define bacon_tolower(c) \
  (((c) == 'A') ? 'a' : ((c) == 'B') ? 'b' : ((c) == 'C') ? 'c' : \
   ((c) == 'D') ? 'd' : ((c) == 'E') ? 'e' : ((c) == 'F') ? 'f' : \
   ((c) == 'G') ? 'g' : ((c) == 'H') ? 'h' : ((c) == 'I') ? 'i' : \
   ((c) == 'J') ? 'j' : ((c) == 'K') ? 'k' : ((c) == 'L') ? 'l' : \
   ((c) == 'M') ? 'm' : ((c) == 'N') ? 'n' : ((c) == 'O') ? 'o' : \
   ((c) == 'P') ? 'p' : ((c) == 'Q') ? 'q' : ((c) == 'R') ? 'r' : \
   ((c) == 'S') ? 's' : ((c) == 'T') ? 't' : ((c) == 'U') ? 'u' : \
   ((c) == 'V') ? 'v' : ((c) == 'W') ? 'w' : ((c) == 'X') ? 'x' : \
   ((c) == 'Y') ? 'y' : ((c) == 'Z') ? 'z' : (c))
#endif

#ifdef __cplusplus
}
#endif

#endif /* BACON_CTYPE_H */

