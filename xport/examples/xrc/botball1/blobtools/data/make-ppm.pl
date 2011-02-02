#!/usr/bin/perl -w

system "mogrify -geometry 356x292 -format ppm @ARGV";
