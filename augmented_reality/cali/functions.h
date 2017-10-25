#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/core/core.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <math.h>

using namespace std;
using namespace cv;

#define TIME 1

#define ESC 27
#define INF 999999
#define RINGS2 24
#define RINGS 12
#define FRAMES_NUMBER 15

#define WARP_DIST 30
#define FAR_DIST 1.7
#define JOIN_DIST 10

int const max_BINARY_value = 255;

typedef size_t ST;
typedef string S;
typedef int I;
typedef float F;
typedef Point P;
typedef Point2f P2f;
typedef Point3f P3f;
typedef vector<P> vP;
typedef vector<P2f> vP2f;
typedef vector<P3f> vP3f;
typedef pair<P,F> pPF;

F numX = 3;   //PadronCirculos
F numY = 2;    //PadronCirculos

int numCornersHor = 4;
int numCornersVer = 3;
int numSquares = numCornersHor * numCornersVer;

F euclidean_dist(P2f &p, P2f &q) {
    P2f diff = p - q;
    return sqrt(diff.x*diff.x + diff.y*diff.y);
}

P2f get_center_contours(vP cp){    
    RotatedRect ellipse = fitEllipse(Mat(cp));
    return ellipse.center;
}

P2f get_center(vP2f cp){
	P2f sum(0,0);
	for(P2f p: cp){
		sum += p;
	}
    return sum/=(float)cp.size();
}

bool is_circle(vP contour, F multLimit){
    P2f center = get_center_contours(contour);
    F max_dist = 0, min_dist = INF;
    for(P p : contour){
        P2f p2f(p.x,p.y);
        F new_dist = euclidean_dist(center, p2f);
        if( new_dist > max_dist)
            max_dist = new_dist;
        if( new_dist < min_dist)
            min_dist = new_dist;
    }
    
    if(max_dist > min_dist*multLimit)  return false;
    return true;
}

F average_distance(vP2f vp, P2f center){
    F dist=0;
    for(P2f p: vp){
        float d = euclidean_dist(p,center);
        dist += d;
    }
    dist /= (float)vp.size();

    return dist;
}

vP2f get_new_centers(vP2f centers, Mat &drawing){
    vP2f newCenters = centers;
    P2f oldC = get_center(centers), newC = oldC*2;
    F dist;
    int color = 150;
    int cont = 0;
    while(oldC != newC && cont++ < 10){
        centers = newCenters;
        newCenters.clear();
        oldC = newC;
        newC = get_center(centers);
        dist = average_distance(centers, newC);  
        dist*=1.9;

        //circle(drawing, newC, dist, color+=30, 2);
        //cout<<"NEW CENTERS\n";
        for(ST i = 0; i<centers.size(); i++)
            if(euclidean_dist(centers[i],newC) < dist)
                newCenters.push_back(centers[i]);
        
    }
    return newCenters;
}

F get_min_dist(vP2f &centers, ST& position){
    P2f x = centers[position], nearPoint;
    centers.erase(centers.begin() + position);      // erase the actual point son we can look for the nearest one
    F minDist=99999;
    
    size_t pos = 0, Pos;
    for(P2f c : centers){
        F dist = euclidean_dist(c,x);
        if(dist < minDist){
            minDist = dist;
            nearPoint = c;
            Pos = pos;
        }
        pos++;
    }
    x = nearPoint;
    position = Pos;
    return minDist;
}

F min_dist(vP2f points, P2f x){
    F minDist=99999;
    
    for(P2f p : points){
        F dist = euclidean_dist(x,p);
        if(dist < minDist && dist > 0)
            minDist = dist;
    }
    return minDist;
}

I n_near(vP2f points, P2f x, F minDist)
{
    I cont = 0;
    for(P2f p : points){
        F dist = euclidean_dist(x,p);
        if( dist < minDist )
            cont++;
    }
    return cont;   
}

void erase_far_centers(vP2f &centers, Mat &drawing)
{
    vP2f newC;
    for(P2f c : centers)
    {
        F dist = min_dist(centers,c);
        //circle(drawing, c, dist*1.5, 255);
        if(n_near(centers,c,dist*FAR_DIST) > 3)
            newC.push_back(c);
    }
    centers = newC;
}

struct sort_pred {
    bool operator()(const std::pair<P,F> &left, const std::pair<P,F> &right) {
        return left.second < right.second;
    }
};
//this shit also sorts them in distance to the middle point
void get_60_centers(vP2f &centers){
    vP2f newCenters;
    P2f c = get_center(centers);
    vector<pPF> vec;

    for(P2f p : centers){
        F dist = euclidean_dist(p,c);
        pPF ppf(p,dist);
        vec.push_back(ppf);
    }
    sort(vec.begin(), vec.end(), sort_pred());

    centers.clear();
    for(ST i=0; i<RINGS2;i++)
        centers.push_back(vec[i].first);
}

vP2f join_near_points(vP2f vp){
    vP2f newvp;
    for(P2f p : vp){
        if(min_dist(newvp, p) >  JOIN_DIST)
            newvp.push_back(p);
    }
    return newvp;
}

P2f nearest_point(vP2f centers, P2f x){
    F minDist=INF;
    P2f nearPoint;
    for(P2f c:centers){
        F dist = euclidean_dist(c,x);
        if(dist < minDist){
            minDist = dist;
            nearPoint = c;
        }
    }
    return nearPoint;
}

P2f nearest_point(P2f centers[4], P2f x){
    F minDist=INF;
    P2f nearPoint;
    for(ST i=0; i<4; i++){
        F dist = euclidean_dist(centers[i],x);
        if(dist < minDist){
            minDist = dist;
            nearPoint = centers[i];
        }
    }
    return nearPoint;
}

I get_corner_pos(P2f vertices[], P2f last_corner){
    F min_dist=INF;
    I pos = -1;
    for(I i=0;i<4; i++)
    {
        P2f vertex = vertices[i];
        F dist = euclidean_dist(vertex,last_corner);
        if( dist < min_dist)
        {
            pos = i;    min_dist = dist;
        }
    }
    return pos;
}

void read_coord_frames(string coords, vector<vP2f> &image_points, int size){
    ifstream file(coords.c_str());
    image_points.resize(size);

    for(int i=0;i <size; i++){
        int x = RINGS;
        while(x--){
            P2f p;
            file>>p.x>>p.y;
            image_points[i].push_back(p);
        }
    }
}

void adaptiveThreshold(cv::InputArray src,cv::OutputArray &dst)
{
    //asumiremos que ambos son Mat correctos.
    Mat inputt = src.getMat();
    Mat dstt(inputt.rows, inputt.cols, CV_8UC1);
    unsigned char* input = inputt.data;
    unsigned char* bin = dstt.data;
    int IMAGE_WIDTH = inputt.cols;
    int IMAGE_HEIGHT = inputt.rows;
    Mat intImg(IMAGE_WIDTH,IMAGE_HEIGHT,CV_32SC1);
    
    int S = (IMAGE_WIDTH) / 8;
    int T = 0.15f;
    unsigned long* integralImg = 0;
    int i, j;
    long sum = 0;
    int count = 0;
    int index;
    int x1, y1, x2, y2;
    int s2 = S / 2;

    // create the integral image
    integralImg = (unsigned long*)malloc(IMAGE_WIDTH*IMAGE_HEIGHT * sizeof(unsigned long*));

    for (i = 0; i<IMAGE_WIDTH; i++)
    {
        // reset this column sum
        sum = 0;

        for (j = 0; j<IMAGE_HEIGHT; j++)
        {
            index = j*IMAGE_WIDTH + i;

            sum += input[index];
            if (i == 0)
                integralImg[index] = sum;
            else
                integralImg[index] = integralImg[index - 1] + sum;
        }
    }

    // perform thresholding
    for (i = 0; i<IMAGE_WIDTH; i++)
    {
        for (j = 0; j<IMAGE_HEIGHT; j++)
        {
            index = j*IMAGE_WIDTH + i;

            // set the SxS region
            x1 = i - s2; x2 = i + s2;
            y1 = j - s2; y2 = j + s2;

            // check the border
            if (x1 < 0) x1 = 0;
            if (x2 >= IMAGE_WIDTH) x2 = IMAGE_WIDTH - 1;
            if (y1 < 0) y1 = 0;
            if (y2 >= IMAGE_HEIGHT) y2 = IMAGE_HEIGHT - 1;

            count = (x2 - x1)*(y2 - y1);

            // I(x,y)=s(x2,y2)-s(x1,y2)-s(x2,y1)+s(x1,x1)
            sum = integralImg[y2*IMAGE_WIDTH + x2] -
                integralImg[y1*IMAGE_WIDTH + x2] -
                integralImg[y2*IMAGE_WIDTH + x1] +
                integralImg[y1*IMAGE_WIDTH + x1];

            if ((long)(input[index] * count) < (long)(sum*(0.8 - T)))
                bin[index] = 0;
            else
                bin[index] = 255;
        }
    }
    dstt.copyTo(dst);
    free(integralImg);
}

vector<Point2d> FtoD(vP2f vec){
    vector<Point2d> vecD;
    for(P2f p : vec)
        vecD.push_back((Point2d)p);
    return vecD;
}