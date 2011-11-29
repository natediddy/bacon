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

#include "bacon-log.h"
#include "bacon-net.h"
#include "bacon-prefs.h"
#include "bacon-util.h"

#ifdef PACKAGE_VERSION
#define BACON_AGENT "Bacon/"PACKAGE_VERSION
#else
#define BACON_AGENT "Bacon/0.00-test"
#endif

using std::string;

namespace bacon
{
  Net::Net(const string &request)
    : pCurl(curl_easy_init())
    , pStatus(CURLE_OK)
    , mUrl(prefs::get(KEY_CM_ROOT_SERVER) + request)
  {
    LOGI("connecting to: %s", mUrl.c_str());
  }

  Net::~Net()
  {
    if (pCurl && pStatus == CURLE_OK)
      curl_easy_cleanup(pCurl);
  }

  bool Net::fetch()
  {
    pStatus = curl_easy_perform(pCurl);
    return pStatus == CURLE_OK;
  }

  bool Net::setup()
  {
    pStatus = curl_easy_setopt(pCurl, CURLOPT_URL, mUrl.c_str());
    if (pStatus != CURLE_OK)
    {
      LOGE("curl_easy_setopt: %s", curl_easy_strerror(pStatus));
      return false;
    }

    pStatus = curl_easy_setopt(pCurl, CURLOPT_USERAGENT, BACON_AGENT);
    if (pStatus != CURLE_OK)
      LOGE("curl_easy_setopt: %s", curl_easy_strerror(pStatus));
    return pStatus == CURLE_OK;
  }
} /* namespace bacon */

