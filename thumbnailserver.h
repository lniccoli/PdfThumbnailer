/*
 *  Copyright (C) 2013 AwTech s.r.l.
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef THUMBNAILER_H
#define THUMBNAILER_H
#include <Poco/Util/ServerApplication.h>

class ThumbnailServer : public Poco::Util::ServerApplication
{
public:
    ThumbnailServer();
    void defineOptions(Poco::Util::OptionSet &options);
    int main(const std::vector<std::string> &args);
private:
    void displayHelp(const std::string &name, const std::string &value);
    bool _help;
};

#endif // THUMBNAILER_H
