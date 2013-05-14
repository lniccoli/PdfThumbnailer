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
#include "pdfrasterizer.h"
#include <sstream>
#include <stdexcept>

extern "C" {
    #include <mupdf.h>
    #include <fitz-internal.h>
    #include <zlib.h>
}

static inline void big32(unsigned char *buf, unsigned int v)
{
    buf[0] = (v >> 24) & 0xff;
    buf[1] = (v >> 16) & 0xff;
    buf[2] = (v >> 8) & 0xff;
    buf[3] = (v) & 0xff;
}

static inline void put32(unsigned int v, std::ostream &out)
{
    out.put(v >> 24);
    out.put(v >> 16);
    out.put(v >> 8);
    out.put(v);
}

static void putchunk(const char *tag, const char *data, int size, std::ostream &out)
{
    unsigned int sum;
    put32(size, out);
    out.write(tag, 4);
    out.write(data, size);
    sum = crc32(0, NULL, 0);
    sum = crc32(sum, (unsigned char*)tag, 4);
    sum = crc32(sum, (unsigned char*)data, size);
    put32(sum, out);
}

static void write_png(fz_context *ctx, fz_pixmap *pixmap, std::ostream &out, int savealpha)
{
    static const unsigned char pngsig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    unsigned char head[13];
    unsigned char *udata = NULL;
    unsigned char *cdata = NULL;
    unsigned char *sp, *dp;
    uLong usize, csize;
    int y, x, k, sn, dn;
    int color;
    int err;

    fz_var(udata);
    fz_var(cdata);

    if (pixmap->n != 1 && pixmap->n != 2 && pixmap->n != 4)
        fz_throw(ctx, (char*)"pixmap must be grayscale or rgb to write as png");

    sn = pixmap->n;
    dn = pixmap->n;
    if (!savealpha && dn > 1)
        dn--;

    switch (dn)
    {
    default:
    case 1: color = 0; break;
    case 2: color = 4; break;
    case 3: color = 2; break;
    case 4: color = 6; break;
    }

    usize = (pixmap->w * dn + 1) * pixmap->h;
    csize = compressBound(usize);
    fz_try(ctx)
    {
        udata = (unsigned char *)fz_malloc(ctx, usize);
        cdata = (unsigned char *)fz_malloc(ctx, csize);
    }
    fz_catch(ctx)
    {
        fz_free(ctx, udata);
        fz_rethrow(ctx);
    }

    sp = pixmap->samples;
    dp = udata;
    for (y = 0; y < pixmap->h; y++)
    {
        *dp++ = 1; /* sub prediction filter */
        for (x = 0; x < pixmap->w; x++)
        {
            for (k = 0; k < dn; k++)
            {
                if (x == 0)
                    dp[k] = sp[k];
                else
                    dp[k] = sp[k] - sp[k-sn];
            }
            sp += sn;
            dp += dn;
        }
    }

    err = compress(cdata, &csize, udata, usize);
    if (err != Z_OK)
    {
        fz_free(ctx, udata);
        fz_free(ctx, cdata);
        fz_throw(ctx, (char*)"cannot compress image data");
    }

    big32(head+0, pixmap->w);
    big32(head+4, pixmap->h);
    head[8] = 8; /* depth */
    head[9] = color;
    head[10] = 0; /* compression */
    head[11] = 0; /* filter */
    head[12] = 0; /* interlace */

    out.write((const char *)pngsig, 8);
    putchunk((const char *)"IHDR", (const char *)head, 13, out);
    putchunk((const char *)"IDAT", (const char *)cdata, csize, out);
    putchunk((const char *)"IEND", (const char *)head, 0, out);

    fz_free(ctx, udata);
    fz_free(ctx, cdata);
}

PdfRasterizer::PdfRasterizer(const std::string &document):
    data(document){
    init();
}

PdfRasterizer::PdfRasterizer(std::istream &input) {
    std::string buffer;
    char tmp[1024];
    while(input.read(tmp, sizeof(tmp))){
        buffer.append(tmp, sizeof(tmp));
    }
    buffer.append(tmp, input.gcount());
    data.swap(buffer);
    init();
}

PdfRasterizer::~PdfRasterizer() {
    fz_close_document(doc);
    fz_close(stream);
    fz_free_context(ctx);
}

void PdfRasterizer::init() {
    ctx = fz_new_context(NULL,NULL,FZ_STORE_UNLIMITED);
    if (!ctx) throw new std::runtime_error("Cannot clone Fitz context");
    fz_try(ctx) {
        stream = fz_open_memory(ctx, (unsigned char *)data.data(), data.length());
        doc = (fz_document*)pdf_open_document_with_stream(stream);
    }
    fz_catch(ctx){
        throw Poco::DataFormatException("Cannot parse PDF document");
    }
}

void PdfRasterizer::renderToStream(unsigned int pageNumber, unsigned int width, unsigned int height, std::ostream &output) {
    fz_page *page = NULL;
    fz_pixmap *pix = NULL;
    fz_device *dev = NULL;
    fz_try(ctx){
        page = fz_load_page(doc, pageNumber);
        fz_rect rect = fz_bound_page(doc, page);
        float origWidth = rect.x1 - rect.x0;
        float origHeight = rect.y1 - rect.y0;
        float destWidth = width;
        float destHeight = height;
        if (!width && !height){
            destWidth  = origWidth;
            destHeight = origHeight;
        } else if (!width){
            destWidth = origWidth*destHeight/origHeight;
        } else if (!height){
            destHeight = origHeight*destWidth/origWidth;
        }
        float scale = std::min(destWidth/origWidth, destHeight/origHeight);
        fz_matrix rectTransform = fz_scale(destWidth/origWidth, destHeight/origHeight);
        rect = fz_transform_rect(rectTransform, rect);
        fz_bbox bbox = fz_round_rect(rect);
        fz_matrix drawTransform = fz_concat(fz_scale(scale,scale),fz_translate((destWidth-scale*origWidth)/2, (destHeight-scale*origHeight)/2));
        pix = fz_new_pixmap_with_bbox(ctx, fz_device_rgb, bbox);
        fz_clear_pixmap(ctx, pix);
        dev = fz_new_draw_device(ctx, pix);
        fz_run_page(doc, page, dev, drawTransform, NULL);
        write_png(ctx,pix,output,true);
    }
    fz_always(ctx){
        if (dev) fz_free_device(dev);
        if (pix) fz_drop_pixmap(ctx, pix);
        if (page) fz_free_page(doc,page);
    }
    fz_catch(ctx){
        throw Poco::RuntimeException("Cannot render page");
    }


}
