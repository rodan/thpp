//------------------------------------------------------------------------------
// File:        example2.cpp
//
// Description: Simple example to write XMP:Title and XMP:Description to a file
//
// Syntax:      example2 FILE [TITLE] [DESCRIPTION]
//
// License:     Copyright 2013-2019, Phil Harvey (philharvey66 at gmail.com)
//
//              This is software, in whole or part, is free for use in
//              non-commercial applications, provided that this copyright notice
//              is retained.  A licensing fee may be required for use in a
//              commercial application.
//
// Created:     2013-11-28 - Phil Harvey
//------------------------------------------------------------------------------

#include <iostream>
#include "ExifTool.h"

using namespace std;

int main(int argc, char **argv)
{
    if (argc < 2) {
        cout << "Example2: Write metadata to an image." << endl;
        cout << "Please specify file name" << endl;
        return 1;
    }
    const char *title = "default title";
    const char *desc = "default description";
    if (argc >= 3) title = argv[2];
    if (argc >= 4) desc = argv[3];

    // create our ExifTool object
    ExifTool *et = new ExifTool();

    // set new values of tags to write
    et->SetNewValue("XMP:Title", title);
    et->SetNewValue("XMP:Description", desc);

    // write the information
    et->WriteInfo(argv[1]);

    // wait for exiftool to finish writing
    int result = et->Complete(10); 

    if (result > 0) {
        // checking either the number of updated images or the number of update
        // errors should be sufficient since we are only updating one file,
        // but check both for completeness
        int n_updated = et->GetSummary(SUMMARY_IMAGE_FILES_UPDATED);
        int n_errors = et->GetSummary(SUMMARY_FILE_UPDATE_ERRORS);
        if (n_updated == 1 && n_errors <= 0) {
            cout << "Success!" << endl;
        } else {
            cerr << "The exiftool application was not successful." << endl;
        }
        // print any exiftool messages
        char *out = et->GetOutput();
        if (out) cout << out;
        char *err = et->GetError();
        if (err) cerr << err;
    } else {
        cerr << "Error executing exiftool command!" << endl;
    }
    delete et;
    return 0;
}

// end
