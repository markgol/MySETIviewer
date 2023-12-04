#pragma once

#pragma pack(push, 1)
typedef struct IMAGINGHEADER {
	short Endian;		// 0 MAC format, -1 PC format
	short ID;			// 0xaaaa, the header always starts with 0,ID or -1,ID
						// A file not starting with this is not the correct filetype
	short HeaderSize;	// number of bytes in header
	LONG32 Xsize;			// number of columns in image (type long allows for long linear bitstreams)
	LONG32 Ysize;			// number of rows image
	short PixelSize;	// pixel size, 1-byte (uchar), 2-uint16 (ushort), 4-int32 (int)
	short NumFrames;	// Number of image frames in the file
	short Version;		// header version  number
						// 1 - this 32 byte header
	short Padding[6];	// dummy entries reserved for other uses
} IMAGINGHEADER;
#pragma pack(pop)

union PIXEL {
	BYTE Byte[4];
	USHORT uShort;
	LONG Long;
};

