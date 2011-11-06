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
#define PROGRESSBAR_NOT_CHAR      '-'

using std::string;

extern int gStartEpoch;

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
      spaceTilEnd = width;
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
      spaceTilEnd--;
      if (space) {
        mBuffer += ' ';
        spaceTilEnd--;
      }
    }

    void add(const string & s, const bool space = true)
    {
      mBuffer += s;
      spaceTilEnd -= s.size();
      if (space) {
        mBuffer += ' ';
        spaceTilEnd--;
      }
    }

    void display(FILE * stream = stdout)
    {
      if (mBuffer.empty()) {
        return;
      }
      for (int i = 0; i < (spaceTilEnd - 2); i++) {
        mBuffer += ' ';
      }
      mBuffer += '\r';
      fprintf(stream, "%s", mBuffer.c_str());
      fflush(stream);
      mBuffer = "";
      spaceTilEnd = width;
    }

    unsigned int count;
    int width;

  private:
    int spaceTilEnd;
    string mBuffer;
  } *pBar = 0;

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

  string percentString(const double & d)
  {
    string result;
    char buf[6];

    sprintf(buf, "%3.0f%%", d * 100);
    result = buf;
    return result;
  }

  string etaString(const int & bytesRemaining, const int & speed)
  {
    int timeLeft = bytesRemaining / speed;
    int mins = timeLeft / 60;
    int secs = timeLeft % 60;
    string result("");

    if (mins || secs) {
      char buf[20];
      snprintf(buf, 20, "(eta %02d:%02d)", mins, secs);
      result += buf;
    } else if (!mins && !secs) {
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
    /* TODO: add an ETA string to end of progress bar */
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

    pBar->add(percentString(fractionDownloaded));
    pBar->add(dlSoFarString, false);
    pBar->add('/', false);
    pBar->add(totalToDlString);
    pBar->add(PROGRESSBAR_START_CHAR, false);

    barPosStopPoint = pBar->width -
      (dlSoFarString.size() + totalToDlString.size() +
       speed.size() + eta.size() + 18);
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
    pBar->display();

    if (downloadedSoFar >= totalToDownload) {
      delete pBar;
      pBar = 0;
    }
    return 0;
  }
}

