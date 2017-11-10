#include "functions.h"

#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library


int FRAME_WIDTH = 1280;
int FRAME_HEIGHT = 720;

class Calibrator{
	VideoCapture cap;
    string vid_name;
	Mat frame;
	Mat processed;

	P2f FIRST_CORNER;
	RotatedRect rr;

	vP2f centers;
	vP2f pattern;
    vP2f lastPattern;
	P2f CENTER;

    Mat intrinsic;
    Mat distCoeffs;
    F rms;

	I iter;
    I th;

    I MAX;
    I MIN;
    F MULT;

	S src;
	S pro;

    I error;
    I totalFrames;

public:
    vector<Point3d> boardPoints;
    vector<Point3d> framePoints;
    vector<Point2d> imagePoints;
    I horizontalCircles;
    I verticalCircles;
    I RINGS;
	
public:
	Calibrator();
    void init(string dir, I thr, I max, I min, F mult);
    void patternSize(I h, I v);

    void preprocessing2(I thrshold, Mat fr );
	void get_pattern(ST pos);
	void draw(Scalar color);

    void read(Mat &fr) { cap >> fr; frame = fr.clone(); }
    bool findCirclesGrid(Mat fr, vector<Point2d> &imagePoints);

    void read_intrinsic_data(string data);

    Mat get_glMatrix(Mat tempimage, Mat &rvec, Mat &tvec);
};

Calibrator::Calibrator():iter(-1), src("src"), pro("pro"){
}

bool Calibrator::findCirclesGrid(Mat fr, vector<Point2d> &imagePoints){
    cvtColor(fr,fr,COLOR_BGR2GRAY);
    processed = Mat::zeros( frame.size(), CV_8UC1 );
    //preprocessing(th);
    preprocessing2(th, fr);
    get_pattern(1);
    imagePoints = FtoD(pattern);
    this->imagePoints = imagePoints;

    return pattern.size()==RINGS;
}

void Calibrator::init(string dir, I thr, I max, I min, F mult){
    th = thr;
    MAX = max;
    MIN = min;
    MULT = mult;
    /*vid_name = dir;
    VideoCapture vcap(dir.c_str());
    if(!vcap.isOpened())
        printf("Error : Video does not exist.\n");
    int frameNumbers = (int) vcap.get(CV_CAP_PROP_FRAME_COUNT);
    cout << frameNumbers <<endl;
    cap = vcap;*/
}

void Calibrator::preprocessing2(I thrshold, Mat fr){
    Mat proc;
    Mat thresh;

    thresh = fr;
    /*rectangle( thresh,
           Point( 1200, 600 ),
           Point( 1280, 720), 250, -1, 8 );
    */
    
    adaptiveThreshold(thresh, proc);
    //adaptiveThreshold(thresh, proc, max_BINARY_value, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 3,0);
    //proc = thresh.clone();
    //threshold( thresh, proc, thrshold, max_BINARY_value,0 );

    //imshow("adfasdf"    , proc);    waitKey(1);

    vector<vP> contours, newContours;
    vector<Vec4i> hierarchy;
    findContours( proc, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    ST contoursSize, newCenters, farCenters, nearPoints;

    centers.clear();
    
    int id=0;
    for(vP vp: contours)
    {  
        size_t size=vp.size();
        if( size < MAX && size > MIN )
        {
            if(is_circle(vp, MULT))
            {
                //drawContours( processed, contours, id, 255, 1, 8, hierarchy, 0, Point() );
                P2f p = get_center_contours(vp);
                newContours.push_back(vp);
                centers.push_back(p);
            }
        }
        id++;
    }
    contoursSize = centers.size();
    if(centers.size() < RINGS)
        return;

    centers = join_near_points(centers);
    nearPoints = centers.size();

    centers = get_new_centers(centers,processed);
    newCenters = centers.size();

    if(centers.size() > RINGS)
        erase_far_centers(centers, processed);
    farCenters = centers.size();

    //for(P2f p :centers)        
      //  circle(processed, p, 3, 255, -1);
    
    //cout<<"contours : "<<contoursSize<<"\t join_near : "<<nearPoints<<"\t get_new : "<<newCenters<<"\t farCenters : "<<farCenters<<endl;    
}

void Calibrator::get_pattern(ST pos){
    P2f LAST, last;
    
    rr = minAreaRect(Mat(centers));         // finds the minimun rectangular area
    P2f vertices[4];                                // the vertices of the rectangle
    rr.points(vertices);

    pattern.clear();
    
    if (centers.size()>RINGS)
    {
        vP2f temp;
        for( P2f c : centers ) {       
            if( min_dist(lastPattern, c) < WARP_DIST )
                temp.push_back(c);
        }
        centers = temp;
    }
    
    if (centers.size()==RINGS) {
        P2f new_center = get_center(centers);
        P2f corner_movement = new_center - CENTER;
        CENTER = new_center;
        //for( int j = 0; j < 4; j++ )  line( processed, vertices[j], vertices[(j+1)%4], 240, 1, 8 );

        P2f corner,actual;
        if(iter ==  1) { FIRST_CORNER = vertices[pos];   iter++; }    // if its the first frame
        //else { FIRST_CORNER += corner_movement; }
        if(iter == -1) { P2f origin(0,0); FIRST_CORNER = nearest_point(vertices, origin); }

        //circle(processed, FIRST_CORNER, 10, 255, -1);
        
        pos = get_corner_pos(vertices,FIRST_CORNER);        // we look for the nearest center to FIRST_CORNER      
        corner = vertices[pos];
        corner = nearest_point(centers, corner);
        actual = corner;                // save first corner in actual
        FIRST_CORNER = corner;          // refresh FIRST_CORNER              
        
        Point2f vecX, vertX, vecY, vertY;
        
        vertX = nearest_point(centers, vertices[(pos+1)%4]);
        vecX = vertX - corner;
            vecX.x /= horizontalCircles - 1;
            vecX.y /= horizontalCircles - 1;

        int ant_pos = pos-1; 
            if(ant_pos==-1)     ant_pos = 3;
        
        vecY = vertices[ant_pos] - vertices[pos]; 
            vecY.x /=verticalCircles - 1;
            vecY.y /=verticalCircles - 1;

        vertY = nearest_point(centers, vertices[ant_pos] - vecY);
        vecY = vertY - corner; 
        
        LAST = last = corner;
        
        pattern.push_back(last);

        //circle(drawing, vertX, 6, 255, -1);circle(drawing, vertY, 9, 255, -1);circle(drawing, corner, 4, 255, -1); 
        arrowedLine(processed, last, last + vecY, 230, 2); arrowedLine(processed, last, last + vecX, 230, 2);
        
        for(int j = 0; j<horizontalCircles; j++){    
            if(j!=0){
                corner += vecX;
                               
                corner = nearest_point(centers, corner);
                actual = corner;
                
                pattern.push_back(actual);
                last = corner;
            }
            for(int i = 0; i<verticalCircles-1; i++){ 
                actual += vecY;

                P2f p = nearest_point(centers, actual);
                pattern.push_back(p);
                last = p;
                actual = p;                                             
            }
        }
    }

    if (pattern.size()==RINGS)
        lastPattern = pattern;
}

void Calibrator::draw(Scalar color){
    if(pattern.size() == RINGS){
        circle(frame, pattern[0], 5, color,-1);
        
        for(I i = 0; i<pattern.size()-1; i++){
            circle(frame, pattern[i], 4, color);
            line(frame, pattern[i], pattern[i+1], color);
            circle(processed, pattern[i], 5, 200);
            line(processed, pattern[i], pattern[i+1], 200);
        }
    }
    imshow(src, frame);
    imshow(pro, processed);
}

void Calibrator::read_intrinsic_data(string data){
    ifstream dat(data.c_str());

    dat>>rms;
    intrinsic = Mat(3, 3, CV_32FC1);
    distCoeffs = Mat(1, 5, CV_32FC1);
    dat>>intrinsic.ptr<float>(0)[0]>>intrinsic.ptr<float>(0)[1]>>intrinsic.ptr<float>(0)[2]>>intrinsic.ptr<float>(1)[0]>>intrinsic.ptr<float>(1)[1]>>intrinsic.ptr<float>(1)[2]>>intrinsic.ptr<float>(2)[0]>>intrinsic.ptr<float>(2)[1]>>intrinsic.ptr<float>(2)[2];
    dat>>distCoeffs.ptr<float>(0)[0]>>distCoeffs.ptr<float>(0)[1]>>distCoeffs.ptr<float>(0)[2]>>distCoeffs.ptr<float>(0)[3]>>distCoeffs.ptr<float>(0)[4];
    cout<<rms<<endl; cout<<"intrinsic\n"<<intrinsic<<endl;    cout<<distCoeffs<<endl;
}

void Calibrator::patternSize(I h, I v) { 
    horizontalCircles = h;  verticalCircles = v;
    RINGS = h * v ;
 
    for (int i=0; i<horizontalCircles; i++)
       for (int j=0; j<verticalCircles; j++)
            boardPoints.push_back( Point3d( double(67.5 * i), double(67.5 * j), 0.0f) );

    framePoints.push_back( Point3d( 0.0, 0.0, 0.0 ) );
    framePoints.push_back( Point3d( 5.0, 0.0, 0.0 ) );
    framePoints.push_back( Point3d( 0.0, 5.0, 0.0 ) );
    framePoints.push_back( Point3d( 0.0, 0.0, -5.0 ) );
}

void printMat(const cv::Mat& cvmat, int n) {
    for (int i=0; i<n; i++){
        for (int j=0; j<n; j++)
            cout<<cvmat.at<double>(i,j)<<" ";
        cout<<endl;
    }
    cout<<endl;
}

Mat Calibrator::get_glMatrix(Mat tempimage, Mat &rvec, Mat &tvec){
    Mat rotation;
    vector<Point2d> imageFramePoints;

    solvePnP( Mat(boardPoints), Mat(imagePoints), intrinsic, distCoeffs, rvec, tvec, false ); //find the camera extrinsic parameters      
    projectPoints(framePoints, rvec, tvec, intrinsic, distCoeffs, imageFramePoints );      //project the reference frame onto the image
    Rodrigues(rvec, rotation);

    ////////////////////////////////////////////////////////////
    Mat projMat = Mat::zeros(4, 4, CV_64FC1);
    Mat projectionMat = Mat::zeros(4, 4, CV_64FC1);
    float zfar = 10000.f, znear = 0.1f;
    //cout << cameraMatrix.at<double>(0, 0) << " " << cameraMatrix.at<double>(0, 1) << " " << cameraMatrix.at<double>(0, 2) << " " << cameraMatrix.at<double>(1, 1) << " " << cameraMatrix.at<double>(1, 2) << endl;

    projMat.at<double>(0, 0) = 2 * intrinsic.at<double>(0, 0) / tempimage.size().width;
    projMat.at<double>(0, 1) = 2 * intrinsic.at<double>(0, 1) / tempimage.size().width;
    projMat.at<double>(0, 2) = -1 + (2 * intrinsic.at<double>(0, 2) / tempimage.size().width); // en la diapo del profe es su negativo se equivoco
    projMat.at<double>(1, 1) = 2 * intrinsic.at<double>(1, 1) / tempimage.size().height;
    projMat.at<double>(1, 2) = -1 + (2 * intrinsic.at<double>(1, 2) / tempimage.size().height);// en la diapo del profe es su negativo se equivoco
    projMat.at<double>(2, 2) = -(zfar + znear) / (zfar - znear);
    projMat.at<double>(2, 3) = -2 * zfar*znear / (zfar - znear);
    projMat.at<double>(3, 2) = -1;

    //printMat(projMat,4);
    transpose(projMat, projectionMat);
    //printMat(projectionMat,4);
    ////////////////////////////////////////////////////////////

        
    gluPerspective(39.0f, float(FRAME_WIDTH) / float(FRAME_HEIGHT), 0.01f, 1000.0f);
    Mat viewMatrix = cv::Mat::zeros(4, 4, CV_64FC1);

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col){
            viewMatrix.at<double>(row, col) = rotation.at<double>(row, col);
        }
        viewMatrix.at<double>(row, 3) = tvec.at<double>(row, 0);        
    }
    viewMatrix.at<double>(3, 3) = 1.0f;     
    //printMat(viewMatrix,4);

    Mat cvToGl = cv::Mat::zeros(4, 4, CV_64F);
    cvToGl.at<double>(0, 0) = 1.0f;
    cvToGl.at<double>(1, 1) = -1.0f;
    cvToGl.at<double>(2, 2) = -1.0f;
    cvToGl.at<double>(3, 3) = 1.0f;
    viewMatrix = cvToGl * viewMatrix;       
    
    Mat glViewMatrix;   
    transpose(viewMatrix, glViewMatrix); 
    
    //printMat(glViewMatrix,4);

    return glViewMatrix;
}