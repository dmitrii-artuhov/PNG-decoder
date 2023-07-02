# PNG Specification Conspect

Specification link - [click](http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html).

## Data Representation

1. Integers and byte order
    - the most significant byte comes first (big-endian)
    - bits numeration from 7 to 0 (highest to lowest)
    - by default every numner is `unsigned` in [0, 2^31]
    - is signed then value in [-2^31, 2^31]
2. Colors
    - Grayscal (represents luminance "яркость")
    - RGB (`?` calibrated color information (if the cHRM chunk is present) or uncalibrated device-dependent color (if cHRM is absent))
    - `?` gAMA chunk specifies the relationship between sample values and display output intensity 
3. Image layout
    - pixels appearing left-to-right within each scanline, and scanlines appearing top-to-bottom
    - The size of each pixel is determined by the `bit depth`, which is the number of bits per sample in the image data
    - pixel types:
        - `indexed-color` pixel (represented by a single sample that is an index into a supplied palette. The image bit depth determines the maximum number of palette entries, but not the color precision within the palette)
        - `grayscale` pixel (represented by a single sample that is a grayscale level, where zero is black and the largest value for the bit depth is white)
        - `truecolor` pixel (represented by three samples: red (zero = black, max = red) appears first, then green (zero = black, max = green), then blue (zero = black, max = blue). **The bit depth specifies the size of each sample, not the total pixel size**)
    - `grayscale` and `truecolor` pixels can also include an alpha sample, as described in the next section
    - PNG permits multi-sample pixels only with 8- and 16-bit samples, so multiple samples of a single pixel are never packed into one byte
    - **Scanlines always begin on byte boundaries. When pixels have fewer than 8 bits and the scanline width is not evenly divisible by the number of pixels per byte, the low-order bits in the last byte of each scanline are wasted. The contents of these wasted bits are unspecified**
    - **An additional "filter-type" byte is added to the beginning of every scanline**. The filter-type byte is not considered part of the image data, but it is included in the datastream sent to the compression step.
4. Alpha channel
    - alpha channel, representing transparency information on a per-pixel basis, can be included in grayscale and truecolor PNG images
    - An alpha value of zero represents full transparency, and a value of 2bitdepth represents a fully opaque pixel
    - **Alpha channels can be included with images that have either 8 or 16 bits per sample, but not with images that have fewer than 8 bits per sample**
    - **The alpha sample for each pixel is stored immediately following the grayscale or RGB samples of the pixel**
    - Transparency control is also possible without the storage cost of a full alpha channel. In an indexed-color image, an alpha value can be defined for each palette entry. In grayscale and truecolor images, a single pixel value can be identified as being "transparent". These techniques are controlled by the tRNS ancillary chunk type
    - If no alpha channel nor tRNS chunk is present, all pixels in the image are to be treated as fully opaque
5. `?` Filtering:
    - The filter algorithm is specified for each scanline by a filter-type byte that precedes the filtered scanline in the precompression datastream
6. Interlaced data order (черезстрочный порядок данных)
    - `interlace method 0`: pixels are stored sequentially from left to right, and scanlines sequentially from top to bottom (no interlacing).
    - `interlace method 1 (Adam7)`: seven distinct passes over the image, the pass in which each pixel is transmitted is defined by replicating the following 8-by-8 pattern over the entire image, starting at the upper left corner:
        ```txt
        1 6 4 6 2 6 4 6
        7 7 7 7 7 7 7 7
        5 6 5 6 5 6 5 6
        7 7 7 7 7 7 7 7
        3 6 4 6 3 6 4 6
        7 7 7 7 7 7 7 7
        5 6 5 6 5 6 5 6
        7 7 7 7 7 7 7 7

        (numbering from 0,0 at the upper left corner)
        - pass 2 contains pixel 4 of scanline 0
        - pass 7 contains the entirety of scanlines 1, 3, 5
        ```
    - This one is hard to undertsand, see http://www.libpng.org/pub/png/spec/1.2/PNG-DataRep.html (2.6 Interlaced data order)
7. Gamma correction
    - `gAMA chunk` contains the power function relating the desired display output with the image samples
    - Gamma correction is not applied to the alpha channel, if any
    - `cHRM chunk` contains the exact chromaticity of the RGB data
    - `sRGB chunk` contains same as above if the RGB data conforms to the [sRGB specification](http://www.libpng.org/pub/png/spec/1.2/PNG-References.html#B.sRGB)
    - `iCCP chunk` can be used to embed an ICC profile [ICC](http://www.libpng.org/pub/png/spec/1.2/PNG-References.html#B.ICC) containing detailed color space information
    - Don't understand it, see http://www.libpng.org/pub/png/spec/1.2/PNG-DataRep.html (2.7. Gamma correction)
8. Text strings
    - A PNG file can store text associated with the image, such as an image description or copyright notice. Keywords are used to indicate what each text string represents
    - `tEXt and zTXt chunks`: ISO/IEC 8859-1 (Latin-1) is the character set recommended for use in the text strings appearing in this chunks (It is a superset of 7-bit ASCII)
    - `iTXt chunk`: If it is necessary to add characters outside of the Latin-1 set
    - compressed text is able to be stored as well

## File Structure:

1. PNG file signature
    - 8 bytes: `137 80 78 71 13 10 26 10` - each value represents the value of the corresponding byte
2. Chunk layout
    - Length
        - 4 bytes: unsigned integer giving the number of bytes in the `chunk's data` field
        - **its value must not exceed 2^31 bytes**
    - Chunk Type
        - 4 bytes: chunk code
    - Chunk Data
        - The data bytes appropriate to the chunk type, if any. This field can be of zero length
    - CRC (Cyclic Redundancy Check, `?` циклическая проверка избыточности)
        - 4-байтовый CRC (циклическая проверка избыточности), вычисляемый для предыдущих байтов в блоке, включая поля типа блока и данных блока, но не включая поле длины. CRC присутствует всегда, даже для блоков, не содержащих данных. [Смотрите алгоритм CRC](http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html#CRC-algorithm).
    - Chunks can appear in any order, but `IHDR` must appear first and `IEND` must appear last
    - Multiple chunks of the same type can appear, but only if specifically permitted for that type
3. Chunk naming conventions
    - Four bits of the type code, namely bit 5 (value 32) of each byte, are used to convey chunk properties
    - Decoders must recognize type codes by a simple four-byte literal comparison (it is incorrect to perform case conversion on type codes)
    - Name structure:
        - `Ancillary bit`: bit 5 of first byte (0 (uppercase) = critical, 1 (lowercase) = ancillary: decoder encountering an unknown chunk in which the ancillary bit is 1 can safely ignore the chunk and proceed to display the image)
        - `Private bit`: bit 5 of second byte (0 (uppercase) = public, 1 (lowercase) = private: **decoders do not need to test the private-chunk property bit**)
        - `Reserved bit`: bit 5 of third byte (Must be 0 (uppercase) in files conforming to this version of PNG, **Decoders should not complain about a lowercase third letter, however, as some future version of the PNG specification could define a meaning for this bit. It is sufficient to treat a chunk with a lowercase third letter in the same way as any other unknown chunk type**)
        - `Safe-to-copy bit`: bit 5 of fourth byte (0 (uppercase) = unsafe to copy, 1 (lowercase) = safe to copy, **This property bit is not of interest to pure decoders**)
        ```txt
        For example, the hypothetical chunk type name bLOb has the property bits:
        
        bLOb  <-- 32 bit chunk type code represented in text form
        ||||
        |||+- Safe-to-copy bit is 1 (lowercase letter; bit 5 is 1)
        ||+-- Reserved bit is 0     (uppercase letter; bit 5 is 0)
        |+--- Private bit is 0      (uppercase letter; bit 5 is 0)
        +---- Ancillary bit is 1    (lowercase letter; bit 5 is 1)
        ```
4. CRC algorithm
    - Не пон, смотри http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html (3.4. CRC algorithm)

## Chunk Specifications

**All implementations must understand and successfully render the standard critical chunks. A valid PNG image must contain an `IHDR` chunk, *one or more* `IDAT` chunks, and an `IEND` chunk.**

### Critical chunks:

1. **IHDR Image header**
    - The `IHDR chunk` must appear **FIRST**
    - Structure:
        ```txt
        Width:              4 bytes
        Height:             4 bytes
        Bit depth:          1 byte
        Color type:         1 byte
        Compression method: 1 byte
        Filter method:      1 byte
        Interlace method:   1 byte
        ```
    - `Width, height`: give the image dimensions in pixels. They are 4-byte integers. Zero is an invalid value. The maximum for each is 231 
    - `Bit depth`: is a single-byte integer giving the number of bits per sample or per palette index (not per pixel). Valid values are 1, 2, 4, 8, and 16, although not all values are allowed for all color types
    - `Color type`: is a single-byte integer that describes the interpretation of the image data. Color type codes represent sums of the following values: 1 (palette used), 2 (color used), and 4 (alpha channel used). Valid values are 0, 2, 3, 4, and 6.
        - Decoders must support all valid combinations of bit depth and color type. The allowed combinations are:
            ```
            Color    Allowed       Interpretation
            Type     Bit Depths
        
            0        1,2,4,8,16    Each pixel is a grayscale sample.
        
            2        8,16          Each pixel is an R,G,B triple.
        
            3        1,2,4,8       Each pixel is a palette index;
                                a PLTE chunk must appear.
        
            4        8,16          Each pixel is a grayscale sample,
                                followed by an alpha sample.
        
            6        8,16          Each pixel is an R,G,B triple,
                                followed by an alpha sample.
            ```
        - **The `sample depth` is the same as the `bit depth` except in the case of color type 3, in which the `sample depth` is always 8 bits**
    - `Compression method`: single-byte integer that indicates the method used to compress the image data. At present, only compression method 0 (`deflate/inflate` compression with a sliding window of at most 32768 bytes) is defined. **Decoders must check this byte and report an error if it holds an unrecognized code. See [Deflate/Inflate Compression](http://www.libpng.org/pub/png/spec/1.2/PNG-Compression.html) for details.**
    - `Filter method`: single-byte integer that indicates the preprocessing method applied to the image data before compression. At present, only filter method 0 (adaptive filtering with five basic filter types) is defined. **Decoders must check this byte and report an error if it holds an unrecognized code. See [Filter Algorithms](http://www.libpng.org/pub/png/spec/1.2/PNG-Filters.html) for details**
    - `Interlace method`: single-byte integer that indicates the transmission order of the image data. **Two values are currently defined: 0 (no interlace) or 1 (Adam7 interlace). See [Interlaced data order](http://www.libpng.org/pub/png/spec/1.2/PNG-DataRep.html#DR.Interlaced-data-order) for details.**
2. **PLTE Palette**
    - The `PLTE chunk` contains from 1 to 256 palette entries, each a three-byte series of the form:
        ```txt
        Red:   1 byte (0 = black, 255 = red)
        Green: 1 byte (0 = black, 255 = green)
        Blue:  1 byte (0 = black, 255 = blue)
        ```
    - **The number of entries is determined from the chunk length. A chunk length not divisible by 3 is an error.**
    - **This chunk must appear for color type 3, and can appear for color types 2 and 6;**
    - **it must not appear for color types 0 and 4**
    - **If this chunk does appear, it must precede the first IDAT chunk**
    - **There must not be more than one PLTE chunk**
    - **For color type 3 (indexed color), the PLTE chunk is required.**
    - **The first entry in PLTE is referenced by pixel value 0, the second by pixel value 1, etc. The number of palette entries must not exceed the range that can be represented in the image bit depth (for example, 24 = 16 for a bit depth of 4).**
    -  **It is permissible to have fewer entries than the bit depth would allow. In that case, any out-of-range pixel value found in the image data is an error.**
    - For color types 2 and 6 (truecolor and truecolor with alpha), the PLTE chunk is optional
    -  If present, it provides a suggested set of from 1 to 256 colors to which the truecolor image can be quantized (if viewer cannot draw the truecolor directly)
    - **Palette uses 8 bits (1 byte) per sample regardless of the image bit depth specification**
3. **IDAT Image data**
    - The `IDAT chunk` contains the actual image data
    - Steps to retrieve data:
        1. Decompress the data using the compression method specified by the `IHDR chunk`.
        2. Defilter the image data according to the filtering method specified by the `IHDR chunk`. (Note that with filter method 0, the only one currently defined, this implies prepending a filter-type byte to each scanline.)
        3. Get image scanlines represented as described in *Image layout*; the layout and total size of this raw data are determined by the fields of `IHDR chunk`
    - **There can be multiple IDAT chunks; if so, they must appear consecutively with no other intervening chunks**: the compressed datastream is then the **concatenation of the contents of all the `IDAT chunks`**
    - zero-length IDAT chunks are valid
4. **IEND Image trailer**
    - The `IEND chunk` must appear **LAST**. It marks the end of the PNG datastream. The chunk's data field is empty.


### Ancillary chunks:

Our decoder ignores them.

## Deflate/Inflate Compression

For PNG compression method 0, the zlib compression method/flags code must specify method code 8 ("deflate" compression) and an LZ77 window size of not more than 32768 bytes

The check value stored at the end of the zlib datastream is calculated on the uncompressed data represented by the datastream. Note that the algorithm used is not the same as the CRC calculation used for PNG chunk check values. The zlib check value is useful mainly as a cross-check that the deflate and inflate algorithms are implemented correctly. Verifying the chunk CRCs provides adequate confidence that the PNG file has been transmitted undamaged.

**In a PNG file, the concatenation of the contents of all the IDAT chunks makes up a zlib datastream**.

**This datastream decompresses to filtered image data**.


## Filter Algorithms

PNG filter method 0 defines five basic filter types:
```txt
Type    Name

0       None
1       Sub
2       Up
3       Average
4       Paeth
```

**In the image data sent to the compression step, each scanline is preceded by a filter-type byte that specifies the filter algorithm used for that scanline.**

Filtering algorithms are applied to **bytes**, not to pixels, regardless of the bit depth or color type of the image.

The filtering algorithms work on the byte sequence formed by a scanline that has been represented as described in Image layout. If the image includes an alpha channel, the alpha data is filtered in the same way as the image data.

**When the image is interlaced, each pass of the interlace pattern is treated as an independent image for filtering purposes**

**To reverse the effect of a filter, the decoder must use the decoded values of the prior pixel on the same line, the pixel immediately above the current pixel on the prior line, and the pixel just to the left of the pixel above**

All filter types are specified [here](http://www.libpng.org/pub/png/spec/1.2/PNG-Filters.html).
