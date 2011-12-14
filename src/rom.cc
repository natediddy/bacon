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

#include <time.h>

#include "env.h"
#include "file.h"
#include "progress.h"
#include "rom.h"
#include "util.h"

BACON_NAMESPACE_BEGIN

using std::string;

long gStartEpoch = 0;

BACON_PRIVATE_NAMESPACE_BEGIN

string formRequest(const string &name)
{
    string result("get/");

    result += name;
    return result;
}

BACON_PRIVATE_NAMESPACE_END

size_t write_CB(void *ptr, size_t size, size_t nmemb, FILE *fp)
{
    return fwrite(ptr, size, nmemb, fp);
}

Rom::Rom(const string &name, const string &path)
    : Net(formRequest(name))
    , File(path)
{
    setup();
}

Rom::~Rom()
{}

bool Rom::setup()
{
    if (!Net::setup())
        return false;

    Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    if (Net::pStatus != CURLE_OK) {
        BACON_LOGE("curl_easy_setopt: %s", curl_easy_strerror(Net::pStatus));
        return false;
    }

    if (!open("wb")) {
        BACON_LOGE("could not open file `%s' on disk", name().c_str());
        return false;
    }

    Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_WRITEDATA,
        (void *)File::mStream);
    if (Net::pStatus != CURLE_OK) {
        BACON_LOGE("curl_easy_setopt: %s", curl_easy_strerror(Net::pStatus));
        return false;
    }

    Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_WRITEFUNCTION,
        (void *)write_CB);
    if (Net::pStatus != CURLE_OK) {
        BACON_LOGE("curl_easy_setopt: %s", curl_easy_strerror(Net::pStatus));
        return false;
    }

    Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_NOPROGRESS, false);
    if (Net::pStatus != CURLE_OK) {
        BACON_LOGE("curl_easy_setopt: %s", curl_easy_strerror(Net::pStatus));
        return false;
    }

    Net::pStatus = curl_easy_setopt(Net::pCurl, CURLOPT_PROGRESSFUNCTION,
        progressBar);

    if (Net::pStatus != CURLE_OK)
        BACON_LOGE("curl_easy_setopt: %s", curl_easy_strerror(Net::pStatus));
    return Net::pStatus == CURLE_OK;
}

bool Rom::fetch()
{
    gStartEpoch = time(0);
    fprintf(stdout, "\n=> %s\n", baseName().c_str());
    Net::pStatus = curl_easy_perform(Net::pCurl);
    close();
    return Net::pStatus == CURLE_OK;
}

BACON_NAMESPACE_END

