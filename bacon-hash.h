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

#ifndef BACON_HASH_H
#define BACON_HASH_H

#include "bacon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BACON_HASH_DIGEST_SIZE 16
#define BACON_HASH_SIZE        BACON_HASH_DIGEST_SIZE * 2 + 1

typedef struct {
  char hash[BACON_HASH_SIZE];
} BaconHash;

void bacon_hash_from_file (BaconHash *hash, const char *filename);
BaconBoolean bacon_hash_match (const BaconHash *hash1,
                               const BaconHash *hash2);

#ifdef __cplusplus
}
#endif

#endif /* BACON_HASH_H */

