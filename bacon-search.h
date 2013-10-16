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

#ifndef BACON_SEARCH_H
#define BACON_SEARCH_H

#include "bacon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BACON_SEARCH_TOKEN_MAX 256

typedef struct BaconSearchTokenList BaconSearchTokenList;

struct BaconSearchTokenList {
  char token[BACON_SEARCH_TOKEN_MAX];
  BaconSearchTokenList *next;
  BaconSearchTokenList *prev;
};

typedef enum {
  BACON_SEARCH_RESULT_NO_MATCHES = -1,
  BACON_SEARCH_RESULT_PARTIAL_MATCHES,
  BACON_SEARCH_RESULT_ALL_MATCHES
} BaconSearchResult;

BaconSearchTokenList *bacon_search_token_list_new (const char *query);
void bacon_search_token_list_free (BaconSearchTokenList *list);
size_t bacon_search_token_list_total (BaconSearchTokenList *list);
BaconSearchResult bacon_search (const char *content,
                                BaconSearchTokenList *list);

#ifdef __cplusplus
}
#endif

#endif /* BACON_SEARCH_H */

