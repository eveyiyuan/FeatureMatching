/* Author: Yi Yuan
*  Input (command line args): <path to image> <number of features to find>
*  Output: image with figures marked in green circles overlaid and .yml file of feature points
*  This function implements Harris corner finding to find corner features on an image
*  Note, this will write images out as .pgm
*/
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include <string>

using namespace cv;
using namespace std;

#define _USE_MATH_DEFINES

Mat compute_h_gradient(Mat m);
Mat compute_v_gradient(Mat m);
double apply_gauss(double sig, int i, int j, int n);

const int wind = 7;
const int windHalf = 3;
const double sig = (double) 7.0/5.0; 
const int searchVal = 3;
const double cutOff = 0.5;
double thresholdVal;
const int k_val = 0.5;

//struct to sort features by fval
struct feature
{
    double fval;
    Point point;
    bool operator < (const feature& feat) const
    {
        return (fval < feat.fval);
    }
};

int main(int argc, char** argv)
{
    Mat image;
    Mat imageColor;
    //reads image name from command line
    image = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    imageColor = imread(argv[1], 1);
    //creates a duplicate of the image to apply the changes to
    Mat gradH = Mat(image.rows, image.cols, CV_32S); 
    Mat gradV = Mat(image.rows, image.cols, CV_32S);

    int numFeat = atoi(argv[2]);

    gradH = compute_h_gradient(image);
    gradV = compute_v_gradient(image);

    
    Scalar Ix;
    Scalar Iy;
    
    Mat hMats[4];
    Mat gaussMats[4];
    
    // the hMats matricies are ordered as such
    // [ hMats[0], hMats[1]
    //   hMats[2], hMats[3]]

    for(int i=0; i<4; i++)
    {
        hMats[i] = Mat(image.rows, image.cols, CV_32S);
        gaussMats[i] = Mat(image.rows, image.cols, CV_64F);
    }

    //Calculate the H matrix values from the gradient values
    for(int r=0; r<image.rows; r++)
    {
        for(int c=0; c<image.cols; c++)
        {
            Ix = gradH.at<int>(r,c);
            Iy = gradV.at<int>(r,c);
            hMats[0].at<int>(r,c) =  ((int) Ix.val[0]) * ((int) Ix.val[0]);
            hMats[1].at<int>(r,c) =  ((int) Ix.val[0]) * ((int) Iy.val[0]);
            hMats[2].at<int>(r,c) =  ((int) Iy.val[0]) * ((int) Ix.val[0]);
            hMats[3].at<int>(r,c) =  ((int) Iy.val[0]) * ((int) Iy.val[0]);
        }
    }
    
    cout << "H mat values calculated\n";
    
    Mat hKern;
    hKern = Mat(wind, wind, CV_64F);
    double kernelValSum=0;
    for(int i=0; i<wind; i++)
    {
        for(int j=0; j<wind; j++)
        {
            //call the function that computes the kernel value
            hKern.at<double>(i,j) = apply_gauss(sig, i-windHalf, j-windHalf, wind);
            kernelValSum += apply_gauss(sig, i-windHalf, j-windHalf, wind);
        }   
    }
    hKern *= (1/kernelValSum); //makesure the kernel values add up to 1
    cout << "Gauss kernel calculated\n";
    for(int t=0; t<4; t++)
    {
        for(int a = windHalf; a<image.rows - windHalf; a++)
        {
            for(int b = windHalf; b<image.cols - windHalf; b++)
            {
                Scalar pix;
                Scalar kernScal;
                double sum = 0;
                int val;
                //apply the kernel and sum the results to each element of the H matrix for each pixel
                // the results
                for(int k=-1*windHalf; k<=windHalf; k++)
                {
                    for(int l=-1*windHalf; l<=windHalf; l++)
                    {
                        pix = hMats[t].at<int>(a - k, b - l);
                        kernScal = hKern.at<double>(k + windHalf,l + windHalf);
                        sum += pix.val[0]*kernScal.val[0];
                    }
                }
                gaussMats[t].at<double>(a,b) = sum;   
            }
        }
    }
     
    cout << "gauss applied\n";
    Mat fVals(image.rows, image.cols, CV_64F);
    double fval;
    double fMax = -1*DBL_MAX;
    double fMin = DBL_MAX;
    //calculate the f values using determinant and trace of the H matrix values
    for(int f = windHalf; f<image.rows - windHalf; f++)
    {
        for(int u = windHalf; u<image.cols - windHalf; u++)
        {
            Scalar H1v = gaussMats[0].at<double>(f,u);
            Scalar H2v = gaussMats[1].at<double>(f,u);
            Scalar H3v = gaussMats[2].at<double>(f,u);
            Scalar H4v = gaussMats[3].at<double>(f,u);

            fval = (H1v.val[0]*H4v.val[0] - H2v.val[0]*H3v.val[0]) -
                k_val * pow((H1v.val[0]+H4v.val[0])/2, 2);
            if(fval>fMax)
            {
                fMax = fval;
            }
            else if(fval<fMin)
            {
                fMin = fval;
            }
            fVals.at<double>(f,u) = fval;
        }
    }
    //calculate the threshold using fMin and fMax
    thresholdVal = cutOff*(fMax - fMin) + fMin;
    cout << "f values calculated\n";

    int locMax_row;
    int locMax_col;
    double locMax = DBL_MIN;
    Scalar temp1;
    Scalar temp2;
    int cornerCounter=0;
    Vector<feature> feats;
    //search a square of side length searchVal*2+1
    //included a 2 pixel buffer due to lack of information processing on the edge pixels during gradient
    //computation
    for(int s=searchVal+2; s<image.rows-searchVal-2; s++)
    {
        for(int q=searchVal+2; q<image.cols-searchVal-2; q++)
        {
            locMax = -1*DBL_MAX;
            for(int k=-1*searchVal; k<=searchVal; k++)
            {
                for(int l=-1*searchVal; l<=searchVal; l++)
                {
                    temp1 = fVals.at<double>(s-k, q-l);
                            
                    if(temp1.val[0] > thresholdVal && temp1.val[0] > locMax)
                    {
                            locMax = temp1.val[0];
                            locMax_row = s-k;
                            locMax_col = q-l;
                    }
                }
            }
            if(locMax_row == s && locMax_col == q)
            {
                feature tempF;
                temp2 = fVals.at<double>(s,q);
                tempF.fval = temp2.val[0];
                tempF.point = Point(q,s);
                feats.push_back(tempF);
                cornerCounter++;
            }
           
        }
    }
    //sort the features from highest f value to lowest
    sort(feats.begin(), feats.end());
    reverse(feats.begin(), feats.end());
    int realFeatNum;
    if(cornerCounter < numFeat)
    {
        realFeatNum = cornerCounter;
    }
    else
    {
        realFeatNum = numFeat;
    }
    cout << "Number of Features found is " << realFeatNum << "\n";

    
    
    
    
    //display the N (command line arg 2) highest fVal number features
    
    for(int i=0; i<realFeatNum; i++)
    {
        circle(imageColor, feats[i].point, 3, Scalar(0,255,0));
    }
    imshow("found features", imageColor);
    string filename = string(argv[1]);
    filename = filename.substr(0, filename.find(".pgm"));
    string filename2 = filename+"_Points.yml";
    filename += "_features.pgm";
    imwrite(filename,imageColor);

    //write selected feature points to .yml file
    FileStorage fsi(filename2, FileStorage::WRITE);
    for(int i=0; i<realFeatNum; i++)
    {
        string pointName = "point";
        stringstream ss;
            ss << i;
            string temp;
            ss >> temp;
        pointName+=temp;
        
        fsi << pointName << feats[i].point;
    }
    waitKey(0);
    return(0);
}

Mat compute_h_gradient(Mat m)
{
    Mat temp(m.rows, m.cols, CV_32S);
    //the kernel we are applying
    float kern[5] = {-2, -1, 0, 1, 2};

    int min = 0;
    int max = 0;
    for(int i = 0; i<m.rows; i++)
    {
        for(int j = 2; j<m.cols-2; j++)
        {
            Scalar pix;
            int sum = 0;
            int val;
                //compute the new value of the pixel by applying the kernel and summing
                // the results
            for(int k=-2; k<=2; k++)
            {
                pix = m.at<unsigned char>(i, j-k);
                val = (int) pix.val[0];
                sum += val*kern[k+2];
            }
            if(sum > max)
            {
                max = sum;
            }
            else if(sum < min)
            {
                min = sum;
            }
           temp.at<int>(i,j) = sum;
        }
    }
    return temp;
}

Mat compute_v_gradient(Mat m)
{
    Mat temp(m.rows, m.cols, CV_32S);
    //the kernel we are applying
    float kern[5] = {-2, -1, 0, 1, 2};

    int min = 0;
    int max = 0;
    for(int i = 0; i<m.cols; i++)
    {
        for(int j = 2; j<m.rows-2; j++)
        {
            Scalar pix;
            int sum = 0;
            int val;
                //compute the new value of the pixel by applying the kernel and summing
                // the results
            for(int k=-2; k<=2; k++)
            {
                pix = m.at<unsigned char>(j-k,i);
                val = (int) pix.val[0];
                sum += val*kern[k+2];
            }
            if(sum > max)
            {
                max = sum;
            }
            else if(sum < min)
            {
                min = sum;
            }
           temp.at<int>(j,i) = sum;
        }
    }
    return temp;
}
double apply_gauss(double sig, int i, int j, int n)
{
    //computes the kernel value given sigma, N, and the i and j index in the kernel
    double kern;
    int k = (n-1)/2; 
    kern = (1/(2*M_PI*sig*sig))*pow(M_E, -(i*i+j*j)/(2*sig*sig));
    return kern;
}
