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

#include "bacon-colors.h"
#include "bacon-out.h"

#define BACON_COLORIZED_STRING_MAX    256
#define BACON_COLOR_STRING_TABLE_SIZE 2048

extern BaconBoolean g_use_color;

static struct {
  char colorized[BACON_COLORIZED_STRING_MAX];
} s_table[BACON_COLOR_STRING_TABLE_SIZE];

static size_t
bacon_get_pos_from_table (void)
{
  size_t pos;

  for (pos = 0; pos < BACON_COLOR_STRING_TABLE_SIZE; ++pos)
    if (!*s_table[pos].colorized)
      break;

  if (pos == BACON_COLOR_STRING_TABLE_SIZE) {
    bacon_init_color ();
    return 0;
  }
  return pos;
}

void
bacon_init_color (void)
{
  size_t x;

  for (x = 0; x < BACON_COLOR_STRING_TABLE_SIZE; ++x)
    *s_table[x].colorized = '\0';
}

const char *
__bacon_color_str (int colorp, const char *str)
{
  size_t x;

  if (!g_use_color) {
    snprintf (s_table[x].colorized, BACON_COLORIZED_STRING_MAX, "%s", str);
    return s_table[x].colorized;
  }

  x = bacon_get_pos_from_table ();
  snprintf (s_table[x].colorized,
            BACON_COLORIZED_STRING_MAX,
            "%s%s%s%s%s",
            (colorp & BACON_BOLD) ? BACON_COLOR_CODE_BOLD : "",
            (colorp & BACON_UNDERLINE) ? BACON_COLOR_CODE_UNDERLINE : "",
            (colorp & BACON_BLACK) ? BACON_COLOR_CODE_BLACK :
              (colorp & BACON_RED) ? BACON_COLOR_CODE_RED :
              (colorp & BACON_GREEN) ? BACON_COLOR_CODE_GREEN :
              (colorp & BACON_YELLOW) ? BACON_COLOR_CODE_YELLOW :
              (colorp & BACON_BLUE) ? BACON_COLOR_CODE_BLUE :
              (colorp & BACON_MAGENTA) ? BACON_COLOR_CODE_MAGENTA :
              (colorp & BACON_CYAN) ? BACON_COLOR_CODE_CYAN :
              (colorp & BACON_WHITE) ? BACON_COLOR_CODE_WHITE :
              "",
            str,
            BACON_COLOR_CODE_NONE);
  return s_table[x].colorized;
}

const char *
__bacon_color_char (int colorp, char c)
{
  size_t x;

  if (!g_use_color) {
    snprintf (s_table[x].colorized, BACON_COLORIZED_STRING_MAX, "%c", c);
    return s_table[x].colorized;
  }

  x = bacon_get_pos_from_table ();
  snprintf (s_table[x].colorized,
            BACON_COLORIZED_STRING_MAX,
            "%s%s%s%c%s",
            (colorp & BACON_BOLD) ? BACON_COLOR_CODE_BOLD : "",
            (colorp & BACON_UNDERLINE) ? BACON_COLOR_CODE_UNDERLINE : "",
            (colorp & BACON_BLACK) ? BACON_COLOR_CODE_BLACK :
              (colorp & BACON_RED) ? BACON_COLOR_CODE_RED :
              (colorp & BACON_GREEN) ? BACON_COLOR_CODE_GREEN :
              (colorp & BACON_YELLOW) ? BACON_COLOR_CODE_YELLOW :
              (colorp & BACON_BLUE) ? BACON_COLOR_CODE_BLUE :
              (colorp & BACON_MAGENTA) ? BACON_COLOR_CODE_MAGENTA :
              (colorp & BACON_CYAN) ? BACON_COLOR_CODE_CYAN :
              (colorp & BACON_WHITE) ? BACON_COLOR_CODE_WHITE :
              "",
            c,
            BACON_COLOR_CODE_NONE);
  return s_table[x].colorized;
}

const char *
__bacon_color_int (int colorp, int n)
{
  size_t x;

  if (!g_use_color) {
    snprintf (s_table[x].colorized, BACON_COLORIZED_STRING_MAX, "%i", n);
    return s_table[x].colorized;
  }

  x = bacon_get_pos_from_table ();
  snprintf (s_table[x].colorized,
            BACON_COLORIZED_STRING_MAX,
            "%s%s%s%i%s",
            (colorp & BACON_BOLD) ? BACON_COLOR_CODE_BOLD : "",
            (colorp & BACON_UNDERLINE) ? BACON_COLOR_CODE_UNDERLINE : "",
            (colorp & BACON_BLACK) ? BACON_COLOR_CODE_BLACK :
              (colorp & BACON_RED) ? BACON_COLOR_CODE_RED :
              (colorp & BACON_GREEN) ? BACON_COLOR_CODE_GREEN :
              (colorp & BACON_YELLOW) ? BACON_COLOR_CODE_YELLOW :
              (colorp & BACON_BLUE) ? BACON_COLOR_CODE_BLUE :
              (colorp & BACON_MAGENTA) ? BACON_COLOR_CODE_MAGENTA :
              (colorp & BACON_CYAN) ? BACON_COLOR_CODE_CYAN :
              (colorp & BACON_WHITE) ? BACON_COLOR_CODE_WHITE :
              "",
            n,
            BACON_COLOR_CODE_NONE);
  return s_table[x].colorized;
}

