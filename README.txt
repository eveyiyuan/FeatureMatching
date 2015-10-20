How to run:
$ ./feature_matching <path to image1> <path to image2>

Output:
  .pgm image of feature trace

Example:
$ ./feature_matching ./sampleImages/marshill_image1.pgm
  ./sampleImages/marshill_image2.pgm

Output:
 ./sampleImages/marshill_image2_matched.pgm

Notes on param choices:
 threshold/ratio filter: Through trial and error, this value seems to be the
 best at keeping the maximum correct matches and filtering out all incorrect
 matches. The choice is a mid range threshold, allowing a wide range of close
 matches (smaller ratio) and keeping out nonmatches (large ratio)

 window size: The large window size makes it more accurate at finding matches.
 This program is less resource intensive than the feature finding program, so
 it can afford to take advantage of a larger window size without slowing down
 performance too much.

to make from scratch, delete all "cmake..." files
$ cmake .
$ make
