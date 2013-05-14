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
#include "thumbnailrequesthandler.h"
#include "pdfrasterizer.h"
#include <sstream>
#include <map>
#include <Poco/URI.h>
#include <Poco/NumberParser.h>
#include <Poco/Exception.h>
using namespace Poco::Net;

static std::map<std::string,std::string> parseQueryParams(const Poco::URI &uri){
    std::map<std::string,std::string> result;
    std::string query = uri.getRawQuery();
    size_t ampersandIndex = 0;
    do {
        size_t keyIndex = ampersandIndex ? ampersandIndex+1 :0;
        size_t valueIndex = query.find("=",keyIndex);
        if (valueIndex == std::string::npos) break; // Parse error;
        valueIndex++;
        ampersandIndex = query.find("&",valueIndex);
        std::string key;
        std::string value;
        Poco::URI::decode(query.substr(keyIndex,valueIndex-keyIndex-1), key);
        if (ampersandIndex == std::string::npos){
            Poco::URI::decode(query.substr(valueIndex), value);
            keyIndex = std::string::npos;
        } else {
            Poco::URI::decode(query.substr(valueIndex, ampersandIndex-valueIndex), value);
        }
        result[key] = value;
    } while (ampersandIndex != std::string::npos);
    return result;
}

ThumbnailRequestHandler::ThumbnailRequestHandler()
{
}

void ThumbnailRequestHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {
    try {
        if (request.getMethod()== HTTPRequest::HTTP_POST){
            std::map<std::string,std::string> query = parseQueryParams(Poco::URI(request.getURI()));
            unsigned int page = Poco::NumberParser::parseUnsigned(query["page"]);
            unsigned int height = 0;
            Poco::NumberParser::tryParseUnsigned(query["height"], height);
            unsigned int width = 0;
            Poco::NumberParser::tryParseUnsigned(query["width"], width);
            PdfRasterizer raster(request.stream());
            response.setStatus(HTTPServerResponse::HTTP_OK);
            response.setContentType("image/png");
            raster.renderToStream(page,width,height,response.send());
        } else {
            response.setStatus(HTTPServerResponse::HTTP_METHOD_NOT_ALLOWED);
            response.send();
        }
    } catch (Poco::DataException &){
        if (!response.sent()){
            response.setStatus(HTTPServerResponse::HTTP_BAD_REQUEST);
            response.send();
        }
    } catch (...) {
        if (!response.sent()){
            response.setStatus(HTTPServerResponse::HTTP_INTERNAL_SERVER_ERROR);
            response.send();
        }
    }
}
