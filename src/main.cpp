/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  Cross-platform command line version of Oleh Yuchuk's Paperbak, a 
 *                  (relatively) high-density paper backup solution
 *
 *        Version:  1.2
 *        Created:  07/27/2017 03:04:03 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  scuti@teknik.io
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <iostream>
#include <string>
#include <stdbool.h>

#include "paperbak.h"
#include "Resource.h"
#include <getopt.h>
using namespace std;

#define VERSIONHI 1
#define VERSIONLO 2

#ifdef CXXSCRAP
#include "cxxopts.hpp"

inline bool isSwitchValid(int value) {
  return ( value < 0 || value > 1 );
}

// redundancy 1 to 10
// dot size 50 to 100
// dpi 40 to 300
bool validate(cxxopts::Options &o) {
    bool is_ok = true;
    if ((o["mode"].as<string>().compare("encode") != 0) && 
         o["mode"].as<string>().compare("decode") != 0) {
        cerr << "error: invalid mode given" << endl;
        is_ok = false;
    }
    if (o["i"].as<string>().empty()) {
        cerr << "error: no input file given" << endl;
        is_ok = false;
    }
    if (o["o"].as<string>().empty()) {
        cerr << "error: no output file given" << endl;
        is_ok = false;
    }
    if (o["s"].as<int>() < 50 || o["s"].as<int>() > 100) {
        cerr << "error: invalid value for dot size" << endl;
        is_ok = false;
    }
    if (o["d"].as<int>() < 40 || o["d"].as<int>() > 300) {
        cerr << "error: invalid value for dpi" << endl;
        is_ok = false;
    }
    if (o["r"].as<int>() < 2 || o["r"].as<int>() > 10) {
        cerr << "error: invalid value for redundancy" << endl;
        is_ok = false;
    }
    if ( isSwitchValid(o["no-header"].as<int>()) ) {
        cerr << "error: invalid value given for no-header switch" << endl;
        is_ok = false;
    }
    if ( isSwitchValid(o["border"].as<int>()) ) {
        cerr << "error: invalid value given for border switch" << endl;
        is_ok = false;
    }

    return is_ok;
}

cxxopts::Options arguments(int ac, char **av) {
    cxxopts::Options o(av[0],
                       "Encodes or decodes high-density printable file backups.");
    vector<string> parg = {"mode", "input", "output"};
    o.add_options()
        ("h,help", "displays help")
        ("v,version", "Display version and information relevant to that version")
        ("mode", 
            "encode or decode, operation on input and output", 
            cxxopts::value<string>())
        ("i,input", 
            "file to encode to or decode from",
            cxxopts::value<string>(),
            "FILE")
        ("o,output",
            "file as a result of program",
            cxxopts::value<string>(),
            "FILE")
        ("d,dpi", 
            "dots per inch of input or output bitmap, between 40 and 300", 
            cxxopts::value<int>() -> default_value("200"))
        ("s,dotsize",
            "size of the dots in bitmap as percentage of maximum dot size in pixels, between 50 and 100",
            cxxopts::value<int>() -> default_value("70"))
        ("r,redundancy", 
            "data redundancy ratio of input or output bitmap as a reciprocal, between 2 and 10", 
            cxxopts::value<int>() -> default_value("5"))
        ("n,no-header", 
            "disable printing of file name, last modify date and time, file size, and page number as a header",
            cxxopts::value<int>() -> default_value("0")-> implicit_value("1"))
        ("b,border", 
            "print a black border around the block", 
            cxxopts::value<int>() -> default_value("0")-> implicit_value("1"))
        ;
    o.parse_positional(parg);
    o.parse(ac, av);
    if (o.count("help")) {
        cout << o.help() << endl;
        exit(EXIT_SUCCESS);
    }else if (o.count("version")) {
      cout << "\nPaperBack v" << VERSIONHI << "." << VERSIONLO << endl
           << "Copyright © 2007 Oleh Yuschuk" << endl << endl
           << "----- THIS SOFTWARE IS FREE -----" << endl
           << "Released under GNU Public License (GPL 3+)" << endl
           << "Full sources available" << endl << endl
           << "Reed-Solomon ECC:" << endl
           << "Copyright © 2002 Phil Karn (GPL)" << endl << endl;
    }else if (!validate(o)) {
      exit(EXIT_FAILURE);
    }
    return o;
}

bool setglobals(cxxopts::Options &options) {
  bool isEncode;
  try {
    // set arguments to extern (global) variables
    ::pb_dpi = options["dpi"].as<int>();
    ::pb_dotpercent = options["dotsize"].as<int>();
    ::pb_redundancy = options["redundancy"].as<int>();
    ::pb_printheader = ( ! options["no-header"].as<int>() );
    ::pb_printborder = options["border"].as<int>(); 
    // decode = !encode
    isEncode = options["mode"].as<string>().compare("encode") == 0;

    // externs (also have matching values in printdata and/or procdata)
    std::string infile = options["input"].as<string>();
    std::string outfile = options["output"].as<string>();
    strcpy (::pb_infile, infile.c_str());
    strcpy (::pb_outbmp, outfile.c_str());
    strcpy (::pb_outfile, outfile.c_str());
  }
  catch (const cxxopts::OptionException& e) {
    cerr << "error parsing options: " << e.what() << endl;
    exit(1);
  }
  catch (const std::exception& e) {
    cerr << "An unexpected error occurred: " << e.what() << endl;
    exit(1);
  }
    return isEncode;
}
#endif


// Global forward declarations
t_fproc   pb_fproc[NFILE];        // Processed file
int       pb_resx, pb_resy;        // Printer resolution, dpi (may be 0!)
t_printdata pb_printdata;          // Print control structure
int       pb_orientation;          // Orientation of bitmap (-1: unknown)
t_procdata pb_procdata;            // Descriptor of processed data
char      pb_infile[MAXPATH];      // Last selected file to read
char      pb_outbmp[MAXPATH];      // Last selected bitmap to save
char      pb_inbmp[MAXPATH];       // Last selected bitmap to read
char      pb_outfile[MAXPATH];     // Last selected data file to save
char      pb_password[PASSLEN];    // Encryption password
int       pb_dpi;                  // Dot raster, dots per inch
int       pb_dotpercent;           // Dot size, percent of dpi
int       pb_compression;          // 0: none, 1: fast, 2: maximal
int       pb_redundancy;           // Redundancy (NGROUPMIN..NGROUPMAX)
int       pb_printheader;          // Print header and footer
int       pb_printborder;          // Border around bitmap
int       pb_autosave;             // Autosave completed files
int       pb_bestquality;          // Determine best quality
int       pb_encryption;           // Encrypt data before printing
int       pb_opentext;             // Enter passwords in open text
int       pb_marginunits;          // 0:undef, 1:inches, 2:millimeters
int       pb_marginleft;           // Left printer page margin
int       pb_marginright;          // Right printer page margin
int       pb_margintop;            // Top printer page margin
int       pb_marginbottom;         // Bottom printer page margin

void dhelp() {
    printf("placeholder - help");
}

void dversion() {
    printf("placeholder - version");
}

int arguments(int ac, char **av) {
    bool is_ok = true;
    int displayhelp = 0, displayversion = 0, isencode;
    struct option long_options[] = {
        // options that set flags
        {"help",    no_argument, &displayhelp,      1},
        {"version", no_argument, &displayversion,   1},
        {"encode",      no_argument, &isencode, 1},
        {"decode",      no_argument, &isencode, 0},
        // options that assign values in switch
        {"input",       required_argument, NULL, 'i'},
        {"output",      required_argument, NULL, 'o'},
        {"dpi",         required_argument, NULL, 'd'},
        {"dotsize",     required_argument, NULL, 's'},
        {"redundancy",  required_argument, NULL, 'r'},
        {"no-header",   no_argument, NULL, 'n'},
        {"border",      no_argument, NULL, 'b'},
        {0, 0, 0, 0}
    };
    int c;
    while(is_ok) {
        int options_index = 0;
        c = getopt_long(ac, av, "i:o:d:s:r:bn", long_options, &options_index);
        if (c == -1) {
            break;
        }
        switch(c) {
            case 0:
                break;
            case 'i':
                if (optarg == NULL) {
                    fprintf(stderr, "error: arg is NULL ! \n");
                    is_ok = false;
                } else {
                    strcpy (::pb_infile, optarg);
                }
                break;
            case 'o':
                if (optarg == NULL) {
                    fprintf(stderr, "error: arg is null \n");
                    is_ok = false;
                } else {
                    strcpy (::pb_outfile, optarg);
                }
                break;
            case 'd':
                ::pb_dpi         = atoi(optarg);
                break;
            case 's':
                ::pb_dotpercent  = atoi(optarg);
                break;
            case 'r':
                ::pb_redundancy  = atoi(optarg);
                break;
            case 'n':
                ::pb_printheader = !(atoi(optarg));
                break;
            case 'b':
                ::pb_printborder = atoi(optarg);
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }
    if (displayhelp) {
        dhelp();
        exit(EXIT_SUCCESS);
    }
    if (displayversion) {
        dversion();
        exit(EXIT_SUCCESS);
    }
    if (strlen(::pb_infile) == 0) {
        fprintf(stderr, "error: no input file given\n");
        is_ok = false;
    }
    if (strlen(::pb_outfile) == 0) {
        fprintf(stderr, "error: no output file given\n");
        is_ok = false;
    }
    if (::pb_dotpercent < 50 || ::pb_dotpercent > 100) {
        fprintf(stderr, "error: invalid dotsize given\n");
        is_ok = false;
    }
    if (::pb_dpi < 40 || ::pb_dpi > 300) {
        fprintf(stderr, "error: invalid dotsize given\n");
        is_ok = false;
    }
    if (::pb_redundancy < 2 || ::pb_redundancy > 10) {
        fprintf(stderr, "error: invalid dotsize given\n");
        is_ok = false;
    }
    if (::pb_printheader < 0 || ::pb_printheader > 1) {
        fprintf(stderr, "error: invalid dotsize given\n");
        is_ok = false;
    }
    if (::pb_printborder < 0 || ::pb_printborder > 1) {
        fprintf(stderr, "error: invalid dotsize given\n");
        is_ok = false;
    }
    if (!is_ok) {
        exit(EXIT_FAILURE);
    }
    if (isencode) {
        printf("program is set to encode.\n");
    } else {
        printf("program is set to decode.\n");
    }
    return isencode;
}


int main(int argc, char ** argv) {
    // set default values for vars affected by arg. parsing.
    ::pb_infile[0]   = 0;
    ::pb_outfile[0]  = 0;
    ::pb_dpi         = 200;
    ::pb_dotpercent  = 70;
    ::pb_redundancy  = 5;
    ::pb_printheader = 0;
    ::pb_printborder = 0;

  bool isEncode = arguments(argc, argv);
  if (isEncode) {
    Printfile(::pb_infile, ::pb_outbmp);
    // begin the process to write the bitmap 
    while (::pb_printdata.step != 0) {
      //cout << "Step: " << ::pb_printdata.step << endl;
      Nextdataprintingstep (&::pb_printdata);
    }
  }
  else {
    Decodebitmap (::pb_infile);
    while (::pb_procdata.step != 0) {
      //cout << "Step: " << ::pb_procdata.step << endl;
      Nextdataprocessingstep (&::pb_procdata);
    }
  }

  return 0;
}

