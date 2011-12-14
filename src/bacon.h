/*
 * This file is part of bacon.
 *
 * bacon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bacon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bacon.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BACON_H_INCLUDED
#define BACON_H_INCLUDED

#include "log.h"

/* global defines */

#define BACON_FREE(p) \
    do { \
        if ((p)) { \
            delete (p); \
            (p) = NULL; \
        } \
    } while (0)

#define BACON_FREE_ARRAY(p) \
    do { \
        if ((p)) { \
            delete [](p); \
            (p) = NULL; \
        } \
  } while (0)

#define BACON_LOG_MSG_(tag, msg, ...) \
  do { \
      if (bacon::log::_isActive()) { \
          bacon::log::_write( \
          bacon::log::_prefix(tag, __FILE__, __LINE__), \
          msg, __VA_ARGS__); \
      } \
  } while (0)

#define BACON_LOGI(msg, ...) BACON_LOG_MSG_("INFO", msg, __VA_ARGS__)
#define BACON_LOGW(msg, ...) BACON_LOG_MSG_("WARNING", msg, __VA_ARGS__)
#define BACON_LOGE(msg, ...) BACON_LOG_MSG_("ERROR", msg, __VA_ARGS__)

#define BACON_NAMESPACE_BEGIN namespace bacon {
#define BACON_NAMESPACE_END   }

#define BACON_PRIVATE_NAMESPACE_BEGIN namespace {
#define BACON_PRIVATE_NAMESPACE_END   }

#endif /* !BACON_H_INCLUDED */

