//------------------------------------------------------------------------------
// File:        example4.cpp
//
// Description: Copy thumbnail image from one file to another
//
// Syntax:      example4 SRCFILE DSTFILE
//
// License:     Copyright 2013-2019, Phil Harvey (philharvey66 at gmail.com)
//
//              This is software, in whole or part, is free for use in
//              non-commercial applications, provided that this copyright notice
//              is retained.  A licensing fee may be required for use in a
//              commercial application.
//
// Created:     2013-11-29 - Phil Harvey
//------------------------------------------------------------------------------

#include <iostream>
#include "ExifTool.h"

using namespace std;

int main(int argc, char **argv)
{
    if (argc < 3) {
        cout << "Example4: Copy ThumbnailImage from one file to another." << endl;
        cout << "Please specify source and destination file names" << endl;
        return 1;
    }
    // create our ExifTool object
    ExifTool *et = new ExifTool();

    // get ThumbnailImage from source file
    TagInfo *info = et->ImageInfo(argv[1], "-b\n-ThumbnailImage", 10);
    if (!info) {
        cerr << "Error extracting information from " << argv[1] << endl;
        return 1;
    }
    // returned info should have at least 2 tags: SourceFile and ThumbnailImage
    if (!info->next) {
        cerr << "Source file doesn't contain a ThumbnailImage" << endl;
        return 1;
    }
    // write ThumbnailImage to destination file
    et->WriteInfo(argv[2], NULL, info);

    // wait for exiftool to finish writing
    int result = et->Complete(10); 
    if (result <= 0) cerr << "Error executing exiftool!" << endl;

    // print any exiftool messages
    char *out = et->GetOutput();
    if (out) cout << out;
    char *err = et->GetError();
    if (err) cerr << err;
    delete et;
    return 0;
}

// end
