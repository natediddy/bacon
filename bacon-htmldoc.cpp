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

#include <cstdlib>
#include <cstring>

#include "bacon-htmldoc.h"

#define DEVICE_PARAM "device="
#define TYPE_PARAM "type="

using std::string;

namespace
{
  string formRequest(const string & device_id, const string & device_type)
  {
    string result = "";

    if (!device_id.empty()) {
      result = "?";
      result += DEVICE_PARAM;
      result += device_id;
    }

    if (!device_type.empty()) {
      if (result.empty()) {
        result = "?";
      } else {
        result += "&";
      }
      result += TYPE_PARAM;
      result += device_type;
    }
    return result;
  }
}

namespace bacon
{
  size_t write_CB(void * buf, size_t size, size_t nmemb, void * userp)
  {
    size_t real = size * nmemb;
    HtmlDoc::MemoryChunk * mem = (HtmlDoc::MemoryChunk *)userp;

    mem->mem = (char *)realloc(mem->mem, mem->size + real + 1);
    if (!mem->mem) {
      return 0;
    }

    memcpy(&(mem->mem[mem->size]), buf, real);
    mem->size += real;
    mem->mem[mem->size] = 0;
    return real;
  }

  HtmlDoc::HtmlDoc(const string & device_id, const string & device_type)
    : Net(formRequest(device_id, device_type))
    , mContent("")
  {
    mMemoryChunk.mem = (char *)malloc(1);
    mMemoryChunk.size = 0;
    setup();
  }

  HtmlDoc::~HtmlDoc()
  {
    if (mMemoryChunk.mem) {
      free((void *)mMemoryChunk.mem);
      mMemoryChunk.mem = 0;
    }
    mMemoryChunk.size = 0;
  }

  bool HtmlDoc::setup()
  {
    if (!Net::setup()) {
      return false;
    }

    Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_WRITEDATA,
        (void *)&mMemoryChunk);
    if (Net::pStatus != CURLE_OK) {
      return false;
    }

    Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_WRITEFUNCTION,
        write_CB);
    return Net::pStatus == CURLE_OK;
  }

  bool HtmlDoc::fetch()
  {
    Net::pStatus = curl_easy_perform(Net::pCurl);
    if (mMemoryChunk.mem) {
      mContent = mMemoryChunk.mem;
    }
    return Net::pStatus == CURLE_OK;
  }

  string HtmlDoc::content() const
  {
    return mContent;
  }
}

