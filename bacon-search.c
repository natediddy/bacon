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

#include "bacon.h"

#include <string.h>

#include "bacon-ctype.h"
#include "bacon-search.h"
#include "bacon-str.h"
#include "bacon-util.h"

BaconSearchTokenList *
bacon_search_token_list_new (const char *query)
{
  size_t n;
  size_t token_pos;
  size_t query_pos;
  BaconSearchTokenList *p;
  BaconSearchTokenList *list;

  list = NULL;
  if (query && *query) {
    query_pos = 0;
    n = strlen (query);
    char buffer[n + 1];
    strncpy (buffer, query, n);
    buffer[n] = '\0';
    /* remove trailing whitespace */
    while (bacon_isspace (buffer[n - 1]))
      buffer[n-- - 1] = '\0';
    while (BACON_TRUE) {
      /* skip leading whitespace */
      while (buffer[query_pos] && bacon_isspace (buffer[query_pos]))
        query_pos++;
      if (buffer[query_pos]) {
        bacon_list_append (BaconSearchTokenList, list, p);
        token_pos = 0;
        /* add token to list */
        while (buffer[query_pos]) {
          p->token[token_pos++] = bacon_tolower (buffer[query_pos++]);
          if ((token_pos == (BACON_SEARCH_TOKEN_MAX - 1)) ||
              bacon_isspace (buffer[query_pos]))
            break;
        }
        p->token[token_pos] = '\0';
        bacon_list_rewind (list, p);
        /* no more tokens */
        if (!buffer[query_pos])
          break;
      }
    }
  }
  return list;
}

void
bacon_search_token_list_free (BaconSearchTokenList *list)
{
  bacon_list_free (list);
}

size_t
bacon_search_token_list_total (BaconSearchTokenList *list)
{
  unsigned int result;
  BaconSearchTokenList *p;

  result = 0;
  for (p = list; p; p = p->next) {
    result++;
    if (!p->next)
      break;
  }
  return result;
}

BaconSearchResult
bacon_search (const char *content, BaconSearchTokenList *list)
{
  size_t n_matches;
  size_t total_tokens;
  size_t n_content;
  BaconSearchTokenList *p;

  n_matches = 0;
  total_tokens = bacon_search_token_list_total (list);

  if (list) {
    n_content = strlen (content);
    if (n_content) {
      char buffer[n_content + 1];
      bacon_strtolower (buffer, n_content, content);
      for (p = list; p; p = p->next) {
        if (strstr (buffer, p->token))
          n_matches++;
        if (!p->next)
          break;
      }
    }
  }

  if (n_matches == total_tokens)
    return BACON_SEARCH_RESULT_ALL_MATCHES;
  if ((n_matches < total_tokens) && (n_matches > 0))
    return BACON_SEARCH_RESULT_PARTIAL_MATCHES;
  return BACON_SEARCH_RESULT_NO_MATCHES;
}

