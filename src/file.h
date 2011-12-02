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

#ifndef BACON_FILE_H_INCLUDED
#define BACON_FILE_H_INCLUDED

#include <cstdio>
#include <string>

namespace bacon {

class File {
public:
    File(const std::string &name);
    ~File();

    std::string name() const;
    std::string dirName() const;
    std::string baseName() const;
    bool isOpen() const;
    bool open(const char *mode = "r");
    bool close();
    bool dispose();
    bool makeDir();
    bool makeDirs();
    bool exists() const;
    bool isDir() const;
    bool isFile() const;
    size_t sizeInBytes() const;
    std::string sizeString() const;
    void change(const std::string &name);
    void writeLine(const std::string &line);
    std::string readLine();

protected:
    FILE *mStream;

private:
    std::string mName;

    class FilePropImpl;
    FilePropImpl *mProp;
};

} /* namespace bacon */

#endif /* !BACON_FILE_H_INCLUDED */

