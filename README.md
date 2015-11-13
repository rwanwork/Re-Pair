# Re-Pair and Des-Pair

## Introduction

Re-Pair is the name of the algorithm and the software which implements the  recursive pairing algorithm.  Its corresponding decompressor is Des-Pair.

The Re-Pair algorithm reduces a message by recursively pairing adjacent  symbols and replacing these symbols with a new symbol.  This continues  until no pair of adjacent symbols occur twice.  A dictionary of phrases  (the phrase hierarchy) and a sequence of symbols are produced as output.   Re-Pair operates off-line as it commences after the entire message has  been read.  For larger messages, fixed-sized blocks can be created.


## About The Source Code

The source code is written in C and compiled using v4.1.2 of gcc for  Debian v4.0 (etch).  It has been compiled and successfully run on both an  Intel Pentium 4 CPU (32-bit) and an Intel Core 2 Duo CPU (64-bit).  However, it takes no advantages of the 64-bit architecture.

In 2015, it could be compiled on an Ubuntu 15.10 (64-bit) using gcc version 5.2.1 .

This main purpose of this software was for research.  Therefore, additional checks and extraneous information has been added into the source code which, if removed, may have a small improvement on the execution time.


## Compiling

The archive includes a `CMakeLists.txt`.  To create an out-of-source build, create a `build/` directory and run `cmake` version 3.2 or higher.  A `Makefile` will be created.  Then type `make`.  For example, if you're in the source directory,

```
    mkdir build
    cd build
    cmake ..
    make
```

To compress a file, run it as:  `repair -i <filename>`.  Two outputs are  produced:  `filename.prel` and `filename.seq`.  The first file is the  phrase hierarchy (or prelude).  It has been encoded using interpolative  coding (see the above cites for more information).  The second file is the sequence and is simply a file of 4-byte integers which map to the  hierarchy, with zeroes ("0") marking the end of a block.  An entropy coder (such as a minimum redundancy (Huffman)) could be used, which is NOT included with this archive.  For example, previous work made use of the [minimum-redundancy coder](http://people.eng.unimelb.edu.au/ammoffat/mr_coder/) on Prof. Alistair Moffat's homepage to compress the sequence.

In order to decompress a file, run it as:  `despair -i <filename>`.  The  decompressed file will have the same filename as the original, except with  a `.u` suffix added.

Run either executable without any arguments to see the list of options.


## Citing

The algorithm is originally described in:
```
    N. J. Larsson and A. Moffat, "Offline Dictionary-Based Compression".
    Proc. IEEE, 88(11)1722-1732, November 2000
```

Extensions so that Re-Pair can be used for phrase browsing are covered by:
```
    R. Wan. "Browsing and Searching Compressed Documents". PhD thesis,
    University of Melbourne, Australia, December 2003.
```

If you are considering citing this software, then if Re-Pair is being used  on character (i.e., single-byte) data without any alignment to word  boundaries, please cite the Proc. IEEE journal.

However, if it is being used in the context of phrase browsing with the  other software systems such as Pre-Pair (word-based pre-processing),  Re-Merge (block merging), and Re-Phine (phrase browsing), then please cite  the thesis.  Re-Phine is not yet publicly available, so e-mail me if you would not mind an intermediate version.

Of course, providing the web site where you got this software (see below)  is also possible.


## Contact

This software was implemented by me (Raymond Wan) for my PhD thesis at  the University of Melbourne (under the supervision of [Prof. Alistair Moffat](http://people.eng.unimelb.edu.au/ammoffat/).

Currently, I'm at the Hong Kong University of Science and Technology:

     E-mails:  rwan.work@gmail.com
               OR
               raymondwan@ust.hk

My homepage is [here](http://www.rwanwork.info/).

The latest version of Re-Pair can be downloaded from [GitHub](https://github.com/rwanwork/Re-Pair).

If you have any information about bugs, suggestions for the documentation or just have some general comments, feel free to write to the above address.


## Version

Changes to this software are recorded in the file ChangeLog up until April 2007.  Since moving the source code to Github on November 13, 2014, any changes are recorded there.


## Copyright and License

Copyright (C) 2003-2015 by Raymond Wan (rwan.work@gmail.com)

Re-Pair / Des-Pair is free software; you can redistribute it and/or modify  it under the terms of the GNU General Public License as published by the  Free Software Foundation; either version 3 of the License, or (at your  option) any later version.  Please see the accompanying file, COPYING.v3 for further details.


About This Repository
---------------------

This GitHub repository was created from the original tarball on my homepage a few years ago.  Initially, it was identical to the version as used in my PhD thesis.  Hopefully, it will be easier for me to maintain in GitHub.  (No, I have not given up on it!)


    Raymond Wan
    November 13, 2015

