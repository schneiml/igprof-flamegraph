igprof-flamegraph
=================

A simple program that reads the igprof [profile statistics](http://igprof.org/dump-format.html) file format and converts it to a format suitable for Brendan Greggs [FlameGraph](http://www.brendangregg.com/flamegraphs.html) visualization.

The igprof format is a pain to parse properly. I still tried to do it properly, which makes the code a bit more complicated than it has to be, and a lot of information is read that is not needed for the current output. No symbol demangling is done, so you might want to add `c++filt` yourself.

Install
-------

Compile and copy to $PATH. No dependencies except C++11 `std`.

Usage
-------

    zcat igprof-outfile.gz | igprof-flamegraph | c++filt | flamegraph.pl > flamegraph.svg

License
-------

Probably GPLv3.
