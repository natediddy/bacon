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
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include "bacon-progress.h"

#define DEFAULT_PROGRESSBAR_WIDTH 40
#define PROGRESSBAR_START_CHAR '['
#define PROGRESSBAR_END_CHAR   ']'
#define PROGRESSBAR_HAS_CHAR   '#'
#define PROGRESSBAR_NOT_CHAR   '-'

namespace
{
  int consoleWidth()
  {
    int width = 0;
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
    return width;
  }

  char * humanReadableSize(const double &size)
  {
    return NULL;
  }

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
}

namespace bacon
{
  int progressBar(void * data,
                  double totalToDownload,
                  double downloadedSoFar,
                  double totalToUpload,
                  double uploadedSoFar)
  {
    int size = consoleWidth() - 20;
    double fractionDownloaded = downloadedSoFar / totalToDownload;
    double percent = fractionDownloaded * 100;
    int pos = roundFraction(fractionDownloaded * size);
    int i;

    if (size <= 0) {
      size = DEFAULT_PROGRESSBAR_WIDTH;
    }

    fprintf(stdout, "%3.0f%% %c", percent, PROGRESSBAR_START_CHAR);

    for (i = 0; i < pos; i++) {
      fputc(PROGRESSBAR_HAS_CHAR, stdout);
    }

    for (; i < size; i++) {
      fputc(PROGRESSBAR_NOT_CHAR, stdout);
    }

    fputc(PROGRESSBAR_END_CHAR, stdout);
    fputs("\r", stdout);
    fflush(stdout);
    return 0;
  }
}

