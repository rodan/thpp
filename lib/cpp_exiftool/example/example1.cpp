//------------------------------------------------------------------------------
// File:        example1.cpp
//
// Description: Simple example to read metadata from a file
//
// Syntax:      example1 FILE
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
        cout << "Example1: Read metadata from an image." << endl;
        cout << "Please specify input file name" << endl;
        return 1;
    }
    // create our ExifTool object
    ExifTool *et = new ExifTool();
    // read metadata from the image
    TagInfo *info = et->ImageInfo(argv[1],NULL,5);
    if (info) {
        // print returned information
        for (TagInfo *i=info; i; i=i->next) {
            cout << i->name << " = " << i->value << endl;
            cout << "  group[0] = " << (i->group[0] ? i->group[0] : "<null>") << endl;// family 0 group name
            cout << "  group[1] = " << (i->group[1] ? i->group[1] : "<null>") << endl;// family 1 group name
            cout << "  group[2] = " << (i->group[2] ? i->group[2] : "<null>") << endl;// family 2 group name
            cout << "  name = " << (i->name ? i->name : "<null>") << endl;      // tag name
            cout << "  desc = " << (i->desc ? i->desc : "<null>") << endl;      // tag description
            cout << "  id = " << (i->id ? i->id : "<null>" ) << endl;           // tag ID
            cout << "  value = " << (i->value ? i->value : "<null>") << endl;   // converted value
            cout << "  valueLen = " << i->valueLen << endl;                     // length of value in bytes (not including null terminator)
            cout << "  num = " << (i->num ? i->num :"<null>") << endl;          // "numerical" value
            cout << "  numLen = " << i->numLen << endl;                         // length of numerical value
            cout << "  copyNum = " << i->copyNum << endl;                       // copy number for this tag name
        }
        // we are responsible for deleting the information when done
        delete info;
    } else if (et->LastComplete() <= 0) {
        cerr << "Error executing exiftool!" << endl;
    }
    // print exiftool stderr messages
    char *err = et->GetError();
    if (err) cerr << err;
    delete et;      // delete our ExifTool object
    return 0;
}

// end
