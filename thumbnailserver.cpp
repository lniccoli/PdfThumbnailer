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
#include "thumbnailserver.h"
#include "handlerfactory.h"
#include "Poco/NumberParser.h"
#include "Poco/Util/IntValidator.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Environment.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>
ThumbnailServer::ThumbnailServer():
    _help(false){
    setUnixOptions(true);
}

void ThumbnailServer::defineOptions(Poco::Util::OptionSet &options) {
    ServerApplication::defineOptions(options);
    options.addOption(Poco::Util::Option("help", "h", "display help information")
                .required(false)
                .repeatable(false)
                .callback(Poco::Util::OptionCallback<ThumbnailServer>(this, &ThumbnailServer::displayHelp))
                );
    options.addOption(
                Poco::Util::Option("port","p","The port on which the server will listen.")
                .required(true)
                .repeatable(false)
                .binding("port")
                .argument("[number]")
                .validator(new Poco::Util::IntValidator(1,65535))
                );
    options.addOption(
                Poco::Util::Option("address","a","The address on which the server will listen.")
                .required(false)
                .repeatable(false)
                .binding("address")
                .argument("[address]")
                );
    options.addOption(
                Poco::Util::Option("threads","t","The number of threads the server will run.")
                .required(false)
                .repeatable(false)
                .binding("thread")
                .argument("[number]")
                .validator(new Poco::Util::IntValidator(1,32))
                );
}

int ThumbnailServer::main(const std::vector<std::string> &) {
    if (_help) {
        Poco::Util::HelpFormatter helpFormatter(options());
        helpFormatter.setUnixStyle(true);
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("A simple RESTful web service providing PDF rasterization.");
        helpFormatter.format(std::cout);
        return Application::EXIT_OK;
    }
    Poco::UInt16 port = config().getInt("port");
    std::string address = config().getString("address", "0.0.0.0");
    int threads = config().getInt("threads", Poco::Environment::processorCount());
    Poco::Net::HTTPServerParams* params = new Poco::Net::HTTPServerParams();
    params->setMaxQueued(256);
    params->setMaxThreads(threads);
    Poco::Net::ServerSocket socket(Poco::Net::SocketAddress(address, port));
    Poco::Net::HTTPServer server(new HandlerFactory(), socket, params);
    server.start();
    waitForTerminationRequest();
    return Application::EXIT_OK;
}

void ThumbnailServer::displayHelp(const std::string&, const std::string&) {
    _help = true;
    stopOptionsProcessing();
}
