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

#ifndef BACON_PROGRESS_H
#define BACON_PROGRESS_H

void bacon_progress_init   (void);
void bacon_progress_deinit (bool newline);
void bacon_progress_file   (double total, double current);
void bacon_progress_page   (double total, double current);

#endif /* BACON_PROGRESS_H */

