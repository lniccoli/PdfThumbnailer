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
#ifndef PDFRASTERIZER_H
#define PDFRASTERIZER_H
#include <vector>
#include <string>
#include <iostream>
#include <Poco/Exception.h>
extern "C" {
    #include <fitz.h>
}

/**
 * @brief Parses a PDF document and renders the pages in PNG format.
 */
class PdfRasterizer
{
public:
    /**
     * @brief Construct a PdfRasterizer from a string.
     * @param document The PDF document bytes.
     * @throw Poco::DataFormatException
     */
    PdfRasterizer(const std::string &document);
    /**
     * @brief Construct a PdfRasterizer from an input stream.
     * @param input A stream from wich the PDF document can be read.
     * @throw Poco::DataFormatException
     */
    PdfRasterizer(std::istream &input);
    ~PdfRasterizer();
    /**
     * @brief renderToStream Renders a page of the PDF document as PNG.
     *
     * @param page The 0-based page number.
     * @param width The width of the PNG in pixels.
     * @param height The height of the PNG in pixels.
     * @param output The stream to wich the PNG will be written.
     * @throw Poco::RuntimeException
     *
     * If both width and height are 0, the page will be rendered at 72dpi.
     * If only one of them is 0, that dimension will be automatically set to preserve the aspect ratio of the page.
     * If both are set, the image will be scaled and translated to best fit the selected dimensions on a transparent background of exactly width x height pixels.
     */
    void renderToStream(unsigned int page, unsigned int width, unsigned int height, std::ostream &output);
private:
    void init();
    std::string data;
    fz_document *doc;
    fz_stream *stream;
    fz_context *ctx;
};

#endif // PDFRASTERIZER_H
