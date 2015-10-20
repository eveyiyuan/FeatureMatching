How to run:
$ ./harris <path to image> <number of features to find>

Output:
   .pgm image highlighting feature points
   .yml file of selected feature points
Example:
$ ./harris ./sample/marshill_image1.pgm

Output:
  ./sample/marshill_image1_features.pgm
  ./sample/marshill_image1_Points.yml

Note: 150 features is what I have used to produce the images
Note: this program outputs a .pgm image file

Notes on Parameters:

threshold:
For the parameters to this function, I selected the cutoff threshold as 0.5
(50%) because it is a value low enough such that harder to see features
may be let through, but not low enough that the number of features let
through starts to become noise. The lower threshold is more tolerant, allowing
for a greater choice in features when it comes to sorting them. The sorting 
of the features by fValue also helps make sure that the N features of the 
highest fValue are the most salient features. 

window size:
The selection of the window size was a balance between efficiency (larger
windows will take a very long time to execute) and accuracy

k value:
A k value that is too high will filter out too many features, often causing
the program to not meet the set quota of features to find. A k value too low
will cause too many lower quality features to cloud the image. A middle valued
k value allows for a larger amount of features to choose from without
compromising too much on their quality.
