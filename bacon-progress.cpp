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

#include <cstdio>
#include <ctime>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include "bacon-progress.h"
#include "bacon-util.h"

#define DEFAULT_PROGRESSBAR_WIDTH 40

#define PROGRESSBAR_START_CHAR    '['
#define PROGRESSBAR_END_CHAR      ']'
#define PROGRESSBAR_HAS_CHAR      '#'
#define PROGRESSBAR_NOT_CHAR      '.'

using std::string;

extern long gStartEpoch;

namespace
{
  class ProgressBar {
  public:
    ProgressBar()
      : count(0)
      , width(0)
      , mBuffer("")
    {
      updateWidth();
      mSpaceTilEnd = width;
    }

    ~ProgressBar()
    {}

    void updateWidth()
    {
#ifdef _WIN32
      CONSOLE_SCREEN_BUFFER_INFO cb;

      if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cb)) {
        width = cb.dwSize.X;
      }
#else
      struct winsize ws;

      ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
      width = ws.ws_col;
#endif

      if (width <= 0) {
        width = DEFAULT_PROGRESSBAR_WIDTH;
      }
    }

    void add(const char & c, const bool space = true)
    {
      mBuffer += c;
      --mSpaceTilEnd;
      if (space) {
        mBuffer += ' ';
        --mSpaceTilEnd;
      }
    }

    void add(const string & s,
             const bool space = true,
             const bool last = false)
    {
      if (last) {
        for (int i = 0; i < (mSpaceTilEnd - 4); ++i) {
          mBuffer += ' ';
        }
      }
      mBuffer += s;
      mSpaceTilEnd -= s.size();
      if (space) {
        mBuffer += ' ';
        --mSpaceTilEnd;
      }
    }

    void display(FILE * stream = stdout)
    {
      if (mBuffer.empty()) {
        return;
      }
      mBuffer += '\r';
      fprintf(stream, "%s", mBuffer.c_str());
      fflush(stream);
      mBuffer = "";
      mSpaceTilEnd = width;
    }

    unsigned int count;
    int width;

  private:
    int mSpaceTilEnd;
    string mBuffer;
  } * pBar = 0;

  int roundFraction(const double & fraction)
  {
    int ret;

    if (fraction >= 0) {
      ret = (int)(fraction + 0.5);
    } else {
      ret = (int)(fraction - 0.5);
    }
    return ret;
  }

  bool isNaN(const double d)
  {
    volatile double check = d;

    if (check != check) {
      return true;
    }
    return false;
  }

  string percentString(const double d)
  {
    if (isNaN(d)) {
      return string("0%");
    }

    char buf[6];
    sprintf(buf, "%3.0f%%", d * 100);
    return string(buf);
  }

  string etaString(const int bytesRemaining, const int speed)
  {
    string result("");

    if (speed) {
      char buf[20];
      snprintf(buf, 20, "(eta %02d:%02d)",
          ((bytesRemaining / speed) / 60),
          ((bytesRemaining / speed) % 60));
      result += buf;
    } else {
      result += "(eta --:--)";
    }
    return result;
  }
}

namespace bacon
{
  int progressBar(void * data,
                  double totalToDownload,
                  double downloadedSoFar,
                  double totalToUpload,
                  double uploadedSoFar)
  {
    string dlSoFarString =
      util::bytesToReadable(6, (long)downloadedSoFar, true);
    string totalToDlString =
      util::bytesToReadable(6, (long)totalToDownload, true);
    double fractionDownloaded = downloadedSoFar / totalToDownload;
    string speed("");
    string eta("");
    int dlSpeed;
    int barPosStopPoint;
    int pos;
    int i;

    if (!pBar) {
      pBar = new ProgressBar;
    } else {
      pBar->count++;
      pBar->updateWidth();
    }

    if (downloadedSoFar) {
      dlSpeed = downloadedSoFar / (gStartEpoch - time(0));
      if (dlSpeed < 0) {
        dlSpeed = -dlSpeed;
      }
      speed = util::bytesToReadable(6, dlSpeed);
      speed += "/s";
      eta = etaString(totalToDownload - downloadedSoFar, dlSpeed);
    }

    pBar->add(dlSoFarString, false);
    pBar->add('/', false);
    pBar->add(totalToDlString);
    pBar->add(PROGRESSBAR_START_CHAR, false);

    barPosStopPoint = pBar->width -
      (dlSoFarString.size() + totalToDlString.size() +
       speed.size() + eta.size() + 11);
    pos = roundFraction(fractionDownloaded * barPosStopPoint);

    for (i = 0; i < pos; ++i) {
      pBar->add(PROGRESSBAR_HAS_CHAR, false);
    }

    for (; i < barPosStopPoint; ++i) {
      pBar->add(PROGRESSBAR_NOT_CHAR, false);
    }

    pBar->add(PROGRESSBAR_END_CHAR);
    pBar->add(speed);
    pBar->add(eta);
    pBar->add(percentString(fractionDownloaded), false, true);
    pBar->display();

    if (downloadedSoFar >= totalToDownload) {
      delete pBar;
      pBar = 0;
    }
    return 0;
  }
}

