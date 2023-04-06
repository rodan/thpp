//------------------------------------------------------------------------------
// File:        example3.cpp
//
// Description: Example to read or write metadata from files
//
// Syntax:      example3 [OPTIONS] FILE [FILE...]
//
// Options:     Any single-argument options supported by the exiftool
//              application, including options of the form -TAG=VALUE to write
//              information to the specified file(s).
//
// License:     Copyright 2013-2019, Phil Harvey (philharvey66 at gmail.com)
//
//              This is software, in whole or part, is free for use in
//              non-commercial applications, provided that this copyright notice
//              is retained.  A licensing fee may be required for use in a
//              commercial application.
//
// Created:     2013-11-23 - Phil Harvey
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ExifTool.h"

int main(int argc, char **argv)
{
    int isWriting = 0;
    int files = 0;
    char *opts = NULL;

    ExifTool *et = new ExifTool();
/*
 * Prepare arguments for ExifTool
 *
 * (we could have blindly concatenated these arguments with newlines and
 *  passed them straight to ExifTool::Command(), then printed the raw
 *  output, but we are doing this to test the functional interface)
 */
    for (int i=1; i<argc; ++i) {
        if (argv[i][0] != '-') {
            ++files;
            continue;
        }
        char *pt = strchr(argv[i], '=');
        if (pt) {
            // we are writing a new tag value (argument of form "-TAG=VALUE")
            int tagLen = pt - argv[i] - 1;
            char *tag = new char[tagLen + 1];
            if (!tag) return 1;
            memcpy(tag, argv[i]+1, tagLen);
            tag[tagLen] = '\0';
            char *value = pt + 1;
            et->SetNewValue(tag, value);    // set new value for this tag
            delete [] tag;
            isWriting = 1;
        } else {
            // assume arguments beginning with "-" are options that we
            // will pass to the exiftool process (Note: this doesn't
            // handle options which require additional parameters)
            int n = opts ? strlen(opts) : 0;
            char *op = new char[n + strlen(argv[i]) + 2];
            if (!op) return 1;
            if (opts) {
                strcpy(op, opts);
            } else {
                op[0] = '\0';
            }
            strcat(op, argv[i]);
            strcat(op, "\n");
            delete [] opts;
            opts = op;
        }
    }
    if (!files) {
        printf("Example3: Read/write metadata\n");
        printf("Please specify image file name(s)\n");
        return 1;
    }
/*
 * Read or write the specified files
 */
    for (int i=1, n=0; i<argc; ++i) {

        // check to see if exiftool process is still running
        // (if not, the calls below will just return error codes,
        //  but we might as well know sooner rather than later
        //  if we have a problem with this process)
        if (!et->IsRunning()) {
            printf("ExifTool process is not running\n");
            break;
        }
        if (argv[i][0] == '-') continue;
        printf("======== %s (%d/%d)\n", argv[i], ++n, files);

        if (isWriting) {

            // write the new values that we have assigned
            et->WriteInfo(argv[i], opts);
            
            // wait for the command to complete so we can see the result
            et->Complete(30);  // (use long timeout of 30 seconds)
            
            // print the stdout message returned by exiftool
            char *out = et->GetOutput();
            if (out) printf("%s", out);

        } else {

            // extract information from this file
            TagInfo *infoList = et->ImageInfo(argv[i], opts);
            
            // run through the returned tag list, printing the tag information
            for (TagInfo *info=infoList; info; info=info->next) {
                if (info->group[0]) {
                    printf("[%s:%s:%s] %s: %s\n",info->group[0],info->group[1],
                        info->group[2],info->name,info->value);
                } else {
                    // this must be a SoureFile line:
                    printf("-------- %s\n", info->value);
                }
            }
            // we are responsible for deleting the tag information when done
            // (note that this will delete the entire linked list)
            delete infoList;

        }

        // print the stderr message returned by exiftool (if any)
        char *err = et->GetError();
        if (err) printf(">>>>>>>>\n%s", err);
    }
    delete et;  // must delete our ExifTool object when done
    return 0;
}

// end
