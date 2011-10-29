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

#ifndef BACON_HTMLDOC_H_INCLUDED
#define BACON_HTMLDOC_H_INCLUDED

#include "bacon-net.h"

namespace bacon
{
  class HtmlDoc : public Net {
  public:
    HtmlDoc(const std::string & device_id = "",
            const std::string & device_type = "");
    ~HtmlDoc();

    std::string content() const;
    bool fetch();

  protected:
    bool setup();

  private:
    struct MemoryChunk {
      char * mem;
      size_t size;
    } mMemoryChunk;
    std::string mContent;
    friend size_t write_CB(void *, size_t, size_t, void *);
  };
}

#endif /* !BACON_HTMLDOC_H_INCLUDED */

