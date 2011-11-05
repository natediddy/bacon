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

#include <cryptopp/files.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <cryptopp/md5.h>

#include "bacon-file.h"
#include "bacon-htmldoc.h"
#include "bacon-htmlparser.h"
#include "bacon-md5.h"
#include "bacon-util.h"

using std::string;
using namespace CryptoPP;

namespace bacon
{
  Md5::Md5(const string & path,
           const string & device_id,
           const string & device_type)
    : mLocalHash("")
    , mRemoteHash("")
  {
    File p(path);
    size_t size = 2 * Weak::MD5::DIGESTSIZE;
    byte buffer[size];
    Weak::MD5 hash;
    FileSource src(p.name().c_str(), true, new HashFilter(hash,
          new HexEncoder(new ArraySink(buffer, size))));
    HtmlDoc doc(device_id, device_type);

    mLocalHash = util::toLowerCase(string((const char *)buffer, size));
    if (doc.fetch()) {
      HtmlParser parser(doc.content());
      mRemoteHash = parser.checksumStringForFile(p.baseName());
    }
  }

  bool Md5::verify() const
  {
    return mLocalHash == mRemoteHash;
  }
}

