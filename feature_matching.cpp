/* Author: Yi Yuan
*  Input (command line args): <path to image1> <path to image2>
*  Output: image with matched features (circled in red) tracked (with blue lines)
*  This function tracks similar features between two images
*  Note, this will write images out as .pgm
*/
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include <string>

using namespace cv;
using namespace std;

const int window = 15;
const int windHalf = window/2;
double threshold_val = 0.5;

//struct allows us to keep a point and the ssd computed with it together
//useful for comparing ssd values to select the best match

struct feature
{
    Point p;
    int ssd;
    bool operator < (const feature& feat) const
    {
        return (ssd < feat.ssd);
    }
};

int main(int argc, char** argv)
{
    //vector of points to store the feature points calculated from part 3
    vector<Point> points1;
    vector<Point> points2;
    
    //loading the first .yml file
    //should have the same name as image file + _Points.yml
    string filename = string(argv[1]);
    filename = filename.substr(0, filename.find(".pgm"));
    string pointfilename = filename+"_Points.yml";
    FileStorage fs(pointfilename, FileStorage::READ);
    FileNode ptFileNode;
    
    int num1 = 0;
    while(true)
    {
        string pointName = "point";
        Point tempPoint;
        stringstream ss;
            ss << num1;
            string temp;
            ss >> temp;
        pointName+=temp;
        ptFileNode = fs[pointName];
        if(ptFileNode.empty() == true)
        {
            break; //break when finding an empty filenode, this means we've reached the end of our list
        }
        ptFileNode >> tempPoint;
        points1.push_back(tempPoint);
        num1++;
    }

    //loading the second file
    string filename2 = string(argv[2]);
    filename2 = filename2.substr(0, filename2.find(".pgm"));
    string pointfilename2 = filename2+"_Points.yml";
    FileStorage fs2(pointfilename2, FileStorage::READ);
    FileNode ptFileNode2;
    
    int num2 = 0;
    while(true)
    {
        string pointName = "point";
        Point tempPoint;
        stringstream ss;
            ss << num2;
            string temp;
            ss >> temp;
        pointName+=temp;
        ptFileNode2 = fs2[pointName];
        if(ptFileNode2.empty() == true)
        {
            break;
        }
        ptFileNode2 >> tempPoint;
        points2.push_back(tempPoint);
        num2++;
    }

    Mat image;
    Mat image2;

    //reads image name from command line
    image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    image2 = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);
    Mat imageColor;
    imageColor = imread(argv[2], 1);
    int sum;
    int difference;
    Scalar im1;
    Scalar im2;
    vector<feature> potentialMatches;
    for(int j=0; j<points2.size(); j++)
    {
        Point currPoint2 = points2[j]; //iterate through all features in the reference frame
        for(int k=0; k<points1.size(); k++)
        {
            Point currPoint1 = points1[k]; //iterate through all features in the other image
            sum = 0;
            for(int r = -windHalf; r <= windHalf; r++)
            {
                for(int c = -windHalf; c <= windHalf; c++)
                {
                    //check a window x window sized square of pixels to compute ssd
                    // do thie for every feature of image 1 with respect to image 2 (reference
                    Point addPoint = Point(r,c);
                    Point squarePoint2 = currPoint2+addPoint;
                    Point squarePoint1 = currPoint1+addPoint;
                    if(squarePoint2.x < 0 || squarePoint1.x < 0 || squarePoint2.x >= image.cols
                        || squarePoint1.x >= image.cols)
                    {
                        continue; //make sure the square doesn't go out of bounds
                    }
                    if(squarePoint2.y < 0 || squarePoint1.y < 0 || squarePoint2.y >= image.rows
                        || squarePoint1.y >= image.rows)
                    {
                        continue;
                    }
                    im2 = image2.at<unsigned char>(squarePoint2);
                    im1 = image.at<unsigned char>(squarePoint1);
                    difference = im1.val[0] - im2.val[0];
                    sum += difference*difference;
                    
                }
            }
            feature tempF;
            tempF.ssd = sum;
            tempF.p = currPoint1;
            potentialMatches.push_back(tempF);
        }
        sort(potentialMatches.begin(), potentialMatches.end()); //sort matches by ssd value from low to high
        if((double)potentialMatches[0].ssd/potentialMatches[1].ssd < threshold_val)
        {
            //if below threshold, it is a significant match, so draw a line between the points
            line(imageColor, currPoint2, potentialMatches[0].p, Scalar(255, 0, 0));
        }
        potentialMatches.clear();
    }
    for(int i=0; i<points2.size(); i++)
    {
        circle(imageColor, points2[i], 3, Scalar(0,0,255));
    }
    imshow("Matched", imageColor);

    //save image to file
    string outputName = argv[2];
    outputName = outputName.substr(0, outputName.find(".pgm"));
    outputName += "_matched.pgm";
    imwrite(outputName, imageColor);
    waitKey(0);
    return(0);
}
