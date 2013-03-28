#include "pixels.h"

// DevIL/OpenIL isn't multithreaded
std::mutex Pixels::lock;

Pixels::Pixels()
    :w(0), h(0), loaded(false)
{
}

// type is either IL_JPG or IL_PNM in this case
Pixels::Pixels(ILenum type, const char* lump, const int size, const std::string& fn)
    :w(0), h(0), loaded(false), fn(fn)
{
    // Only execute in one thread since DevIL/OpenIL doesn't support multithreading
    std::unique_lock<std::mutex> lck(lock);

    ILuint name;
    ilGenImages(1, &name);
    ilBindImage(name);

    if (ilLoadL(type, lump, static_cast<ILuint>(size)))
    {
        w = ilGetInteger(IL_IMAGE_WIDTH);
        h = ilGetInteger(IL_IMAGE_HEIGHT);
        
        // If the image height or width is larger than int's max, it will appear
        // to be negative. Just don't use extremely large (many gigapixel) images.
        if (w < 0 || h < 0)
            throw std::runtime_error("use a smaller image, can't store dimensions in int");

        // 3 because IL_RGB
        const int total = w*h*3;
        unsigned char* data = new unsigned char[total];

        ilCopyPixels(0, 0, 0, w, h, 1, IL_RGB, IL_UNSIGNED_BYTE, data);
        
        // Move data into a nicer format
        int x = 0;
        int y = 0;
        p = std::vector<std::vector<unsigned char>>(h, std::vector<unsigned char>(w));

        // Start at third
        for (int i = 2; i < total; i+=3)
        {
            // Average for grayscale
            p[y][x] = smartFloor((1.0*data[i-2]+data[i-1]+data[i])/3);
            
            // Increase y every time we get to end of row
            if (x+1 == w)
            {
                x=0;
                ++y;
            }
            else
            {
                ++x;
            }
        }

        loaded = true;
        delete[] data;
    }
    else
    {
        throw std::runtime_error("could not read image");
    }
    
    ilDeleteImages(1, &name);
}

void Pixels::mark(const Coord& c, int size)
{
    if (c.x >= 0 && c.y >= 0 &&
        c.x < w  && c.y < h)
        marks.push_back(Mark(c, size));
}

void Pixels::line(const Coord& p1, const Coord& p2)
{
    const int min_x = std::min(p1.x, p2.x);
    const int max_x = std::max(p1.x, p2.x);
    const int min_y = std::min(p1.y, p2.y);
    const int max_y = std::max(p1.y, p2.y);

    // One for vertical, one for all other cases
    if (min_x == max_x)
        for (int y = min_y; y <= max_y; ++y)
            mark(Coord(lineFunctionX(p1, p2, y), y), 1);
    else
        for (int x = min_x; x <= max_x; ++x)
            mark(Coord(x, lineFunctionY(p1, p2, x)), 1);
}

void Pixels::save(const std::string& filename, bool show_marks, bool dim, bool bw) const
{
    // We'll use the default color unless we're dimming the image. Then we'll use
    // black since the default color might blend in.
    unsigned char color = MARK_COLOR;

    // Work on a separate copy of this image
    std::vector<std::vector<unsigned char>> copy = p;

    // Converting both at once would be faster
    if (bw && dim)
    {
        color = 0;

        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                copy[y][x] = (copy[y][x]>GRAY_SHADE)?255:170; // 255-255/3 = 170
    }
    // Convert to black and white
    else if (bw)
    {
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                copy[y][x] = (copy[y][x]>GRAY_SHADE)?255:0;
    }
    // Dim the image
    else if (dim)
    {
        color = 0;

        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                copy[y][x] = 170 + copy[y][x]/3; // 255-255/3 = 170
    }

    // Draw the marks on a copy of the image
    if (show_marks)
    {
        for (const Mark& m : marks)
        {
            if (m.size > 1)
            {
                // Left
                for (int i = m.coord.x; i > m.coord.x-m.size && i >= 0; --i)
                    copy[m.coord.y][i] = color;
                // Right
                for (int i = m.coord.x; i < m.coord.x+m.size && i < w; ++i)
                    copy[m.coord.y][i] = color;
                // Up
                for (int i = m.coord.y; i > m.coord.y-m.size && i >= 0; --i)
                    copy[i][m.coord.x] = color;
                // Down
                for (int i = m.coord.y; i < m.coord.y+m.size && i < h; ++i)
                    copy[i][m.coord.x] = color;
            }
            else
            {
                copy[m.coord.y][m.coord.x] = color;
            }
        }
    }

    // One thread again
    std::unique_lock<std::mutex> lck(lock);

    // Convert this back to a real black and white image
    ILuint name;
    ilGenImages(1, &name);
    ilBindImage(name);
    ilEnable(IL_FILE_OVERWRITE);
    
    // 3 because IL_RGB
    const int total = w*h*3;
    unsigned char* data = new unsigned char[total];

    // Position in data
    int pos = 0;

    // For some reason the image is flipped vertically when loading it with
    // ilTexImage, so start at the bottom and go up.
    for (int y = h-1; y >= 0; --y)
    {
        for (int x = 0; x < w; ++x)
        {
            // Black or white for RGB
            const unsigned char val = copy[y][x];
            data[pos]   = val;
            data[pos+1] = val;
            data[pos+2] = val;

            // For R, G, and B that we just set
            pos+=3;
        }
    }
    
    ilTexImage(w, h, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, data);
    
    if (!ilSaveImage(filename.c_str()) || ilGetError() == IL_INVALID_PARAM)
        throw std::runtime_error("could not save image");
    
    ilDeleteImages(1, &name);
}

// TODO:
//   We have the same sort of rotation code three times below. Somehow we should
//   simplify that.
//

// In the future, it may be a good idea to implement something like the "Rotation by
// Area Mapping" talked about on http://www.leptonica.com/rotation.html
void Pixels::rotate(double rad, const Coord& point)
{
    // Right size, default to white (255 or 1111 1111)
    std::vector<std::vector<unsigned char>> copy(h, std::vector<unsigned char>(w, 0xff));

    // -rad because we're calculating the rotation to get from the new rotated
    // image to the original image. We're walking the new image instead of the
    // original so as to not get blank spots from rounding.
    const double sin_rad = std::sin(-rad);
    const double cos_rad = std::cos(-rad);

    // Rotate image
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            // "Translate" it, and then add back in the point's x and y
            const int trans_x = x - point.x;
            const int trans_y = y - point.y;

            // Find where to copy from
            const int old_x = smartFloor(trans_x*cos_rad + trans_y*sin_rad) + point.x;
            const int old_y = smartFloor(trans_y*cos_rad - trans_x*sin_rad) + point.y;

            // Get rid of invalid points
            if (old_x >= 0 && old_y >= 0 &&
                old_x < w  && old_y < h)
                copy[y][x] = p[old_y][old_x];
        }
    }

    p = copy;

    // Rotate marks as well. This is the same code as rotateVector,
    // but it has to be a bit different since marks also include a size
    const double mark_sin_rad = std::sin(rad);
    const double mark_cos_rad = std::cos(rad);

    for (Mark& m : marks)
    {
        // Translate to origin
        const int trans_x = m.coord.x - point.x;
        const int trans_y = m.coord.y - point.y;

        // Rotate + translate back
        // Using std::round seems to make them closer to what is expected
        const int new_x = std::round(trans_x*mark_cos_rad + trans_y*mark_sin_rad) + point.x;
        const int new_y = std::round(trans_y*mark_cos_rad - trans_x*mark_sin_rad) + point.y;

        if (new_x >= 0 && new_y >= 0 &&
            new_x < w && new_y < h)
            m = Mark(Coord(new_x, new_y), m.size);
    }
}

// Rotate all points in a vector (more or less the same as rotating the image)
// This is in Pixels since it uses the width and height of an image
void Pixels::rotateVector(std::vector<Coord>& v, const Coord& point, double rad) const
{
    const double sin_rad = std::sin(rad);
    const double cos_rad = std::cos(rad);

    for (Coord& m : v)
    {
        // Translate to origin
        const int trans_x = m.x - point.x;
        const int trans_y = m.y - point.y;

        // Rotate + translate back
        // Using std::round seems to make them closer to what is expected
        const int new_x = std::round(trans_x*cos_rad + trans_y*sin_rad) + point.x;
        const int new_y = std::round(trans_y*cos_rad - trans_x*sin_rad) + point.y;

        if (new_x >= 0 && new_y >= 0 &&
            new_x < w && new_y < h)
            m = Coord(new_x, new_y);
    }
}
