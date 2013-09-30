/*
 * Copyright (C) 2013 Nathan Forbes
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BACON_HASH_H
#define BACON_HASH_H

#include "bacon.h"

#define BACON_HASH_DIGEST_SIZE 16
#define BACON_HASH_SIZE        BACON_HASH_DIGEST_SIZE * 2 + 1

typedef struct {
  char hash[BACON_HASH_SIZE];
} BaconHash;

void bacon_hash_from_file (BaconHash *hash, const char *filename);
bool bacon_hash_match     (const BaconHash *hash1, const BaconHash *hash2);

#endif /* BACON_HASH_H */

