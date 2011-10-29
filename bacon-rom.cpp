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

#include "bacon-env.h"
#include "bacon-file.h"
#include "bacon-progress.h"
#include "bacon-rom.h"

using std::string;

namespace
{
  string formRequest(const string & romFileName)
  {
    string result = "get/";

    result += romFileName;
    return result;
  }
}

namespace bacon
{
  size_t write_CB(void * ptr, size_t size, size_t nmemb, FILE * fp)
  {
    return fwrite(ptr, size, nmemb, fp);
  }

  Rom::Rom(const string & romFileName, const string & romPath)
    : Net(formRequest(romFileName))
    , File(romPath)
  {
    setup();
  }

  Rom::~Rom()
  {}

  bool Rom::setup()
  {
    if (!Net::setup()) {
      return false;
    }

    Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    if (Net::pStatus != CURLE_OK) {
      return false;
    }

    if (!open("wb")) {
      return false;
    }

    Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_WRITEDATA,
        (void *)File::mStream);
    if (Net::pStatus != CURLE_OK) {
      return false;
    }

    Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_WRITEFUNCTION,
        (void *)write_CB);
    if (Net::pStatus != CURLE_OK) {
      return false;
    }

    Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_NOPROGRESS, 0L);
    if (Net::pStatus != CURLE_OK) {
      return false;
    }

    //Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_PROGRESSFUNCTION, );
    //if (Net::pStatus != CURLE_OK) {
      //return false;
    //}

    //Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_PROGRESSDATA, );
    return Net::pStatus == CURLE_OK;
  }

  bool Rom::fetch()
  {
    Net::pStatus = curl_easy_perform(Net::pCurl);
    close();
    return Net::pStatus == CURLE_OK;
  }
}

