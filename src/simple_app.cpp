// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// The program converts a mixed-case file to upper case:
// read "in", convert to upper case, write to "out"
//
// command line options
// --cpu_time N: use about N CPU seconds after copying files
//

#include "config.h"
#include <cstdio>
#include <cctype>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <unistd.h>

#include "str_util.h"
#include "util.h"
#include "filesys.h"
#include "boinc_api.h"
#include "mfile.h"

using std::string;

#define INPUT_FILENAME "in"
#define OUTPUT_FILENAME "out"

double cpu_time = 20, comp_result; // FIXME - remove, no longer needed.

// reads char, converts to upper case, writes char.
//
void upper_case(FILE* in, MFILE out) {
	char c;
	int i;

    for (i=0; ; i++) {
        c = fgetc(in);
        if (c == EOF) break;
        c = toupper(c);
        out._putchar(c);
    }
}

/**
 * TODO:
 * 1 - refactor code to include (uc, cp, etc in separated functions)
 * 2 - get input file, load it in libtorrent
 * 3 - one its all done, perform computation
 * 4 - create .torrent for the output
 * 5 - mv input and output files, and copy .torrent files (input and output) to a shared dir (with other WUs). 
 */
int main(int argc, char **argv) {
    int i;
    int c, retval;
    char input_path[512], output_path[512], buf[256];
    MFILE out;
    FILE* infile;

    // process command line args.
    // FIXME - remove, no longer used.
    for (i=0; i<argc; i++) {
        if (strstr(argv[i], "cpu_time")) {
            cpu_time = atof(argv[++i]);
        }
    }

    // init boinc
    //
    retval = boinc_init();
    if (retval) {
        fprintf(stderr,
        		"%s boinc_init returned %d\n",
        		boinc_msg_prefix(buf, sizeof(buf)), retval);
        exit(retval);
    }

    // open the input file (resolve logical name first)
    // FIXME
    boinc_resolve_filename(INPUT_FILENAME, input_path, sizeof(input_path));
    infile = boinc_fopen(input_path, "r");
    if (!infile) {
        fprintf(stderr,
                "%s Couldn't find input file, resolved name %s.\n",
                boinc_msg_prefix(buf, sizeof(buf)), input_path);
        exit(-1);
    }

    // open the output file (resolve logical name first)
    // FIXME
    boinc_resolve_filename(OUTPUT_FILENAME, output_path, sizeof(output_path));
    retval = out.open(output_path, "wb");
    if (retval) {
        fprintf(stderr, "%s APP: upper_case output open failed:\n",
            boinc_msg_prefix(buf, sizeof(buf))
        );
        fprintf(stderr, "%s resolved name %s, retval %d\n",
            boinc_msg_prefix(buf, sizeof(buf)), output_path, retval
        );
        perror("open");
        exit(1);
    }

    // main loop - read characters, convert to UC, write
    //
    upper_case(infile, out);

    // flush output file.
    //
    retval = out.flush();
    if (retval) {
        fprintf(stderr,
        		"%s APP: upper_case flush failed %d\n",
                boinc_msg_prefix(buf, sizeof(buf)), retval);
        exit(1);
    }

    boinc_fraction_done(1);
    boinc_finish(0);
}
