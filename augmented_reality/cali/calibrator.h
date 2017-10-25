#include "functions.h"

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
	Calibrator();
    void init(string dir, I thr, I max, I min, F mult);
	void loop();
	void preprocessing(I thrshold );
    void preprocessing2(I thrshold, Mat fr );
	void get_pattern(ST pos);
	void draw(Scalar color);

    void read(Mat &fr) { cap >> fr; frame = fr.clone(); }
    bool findCirclesGrid(Mat fr, Size cbSize, vector<Point2d> &imagePoints);

    void gen_intrinsic_data(string data);
    void read_intrinsic_data(string data);
    void draw_axis();
    void get_intrinsic(Mat &intr, Mat &distC) {
        intr = intrinsic.clone();
        distC = distCoeffs.clone();
    }
};

Calibrator::Calibrator():iter(-1), src("src"), pro("pro"){
}

bool Calibrator::findCirclesGrid(Mat fr, Size cbSize, vector<Point2d> &imagePoints){
    processed = Mat::zeros( frame.size(), CV_8UC1 );
    //preprocessing(th);
    preprocessing2(th, fr);
    get_pattern(1);
    imagePoints = FtoD(pattern);

    return pattern.size()==RINGS;
}

void Calibrator::init(string dir, I thr, I max, I min, F mult){
    th = thr;
    MAX = max;
    MIN = min;
    MULT = mult;
    vid_name = dir;
    VideoCapture vcap(dir.c_str());
    if(!vcap.isOpened())
        printf("Error : Video does not exist.\n");
    int frameNumbers = (int) vcap.get(CV_CAP_PROP_FRAME_COUNT);
    cout << frameNumbers <<endl;
    cap = vcap;
}

void Calibrator::loop(){
	namedWindow(src,1);
	namedWindow(pro,1);
    int frameNumbers = (int) cap.get(CV_CAP_PROP_FRAME_COUNT);

    while(frameNumbers--){
    	cap >> frame;
        cout<<totalFrames<<" ---> ";
    	processed = Mat::zeros( frame.size(), CV_8UC1 );
    	preprocessing(th);
    	get_pattern(1);
        if(pattern.size() != RINGS) error++;
    	draw(Scalar(0,255,255));
    	if( waitKey(TIME) == ESC ) break;
        totalFrames++;
        //cout<<totalFrames<<" "<<error<<endl;
    }
}

void Calibrator::preprocessing(I thrshold){
	Mat proc;
    Mat thresh;
    cvtColor(frame, thresh, COLOR_BGR2GRAY);            
    rectangle( thresh,
           Point( 1200, 600 ),
           Point( 1280, 720), 250, -1, 8 );

    GaussianBlur(thresh, thresh, Size(7,7), 1.5, 1.5);
    
    adaptiveThreshold(thresh, proc);
    //adaptiveThreshold(thresh, proc, max_BINARY_value, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 3,0);
    //proc = thresh.clone();
    //threshold( proc, proc, thrshold, max_BINARY_value,0 );

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

    for(P2f p :centers)        
        circle(processed, p, 3, 255, -1);
    
    //cout<<"contours : "<<contoursSize<<"\t join_near : "<<nearPoints<<"\t get_new : "<<newCenters<<"\t farCenters : "<<farCenters<<endl;    
}

void Calibrator::preprocessing2(I thrshold, Mat fr){
    Mat proc;
    Mat thresh;

    thresh = fr;
    /*rectangle( thresh,
           Point( 1200, 600 ),
           Point( 1280, 720), 250, -1, 8 );
*/
    //GaussianBlur(thresh, thresh, Size(7,7), 1.5, 1.5);
    
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
    
    //temporal .. we might need to look for the corners
    /*if (centers.size()<RINGS){
        cout<<" IN ";
        centers.insert(centers.end(),lastPattern.begin(),lastPattern.end());
        centers = join_near_points(centers);
        cout<<centers.size()<<"  OUT\n";
        if(centers.size() > RINGS)
            centers = get_new_centers(centers,processed);
        if(centers.size() > RINGS)
            erase_far_centers(centers, processed);
        
    }*/

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
            vecX.x /= numX;
            vecX.y /= numX;

        int ant_pos = pos-1; 
            if(ant_pos==-1)     ant_pos = 3;
        
        vecY = vertices[ant_pos] - vertices[pos]; 
            vecY.x /=numY;
            vecY.y /=numY;

        vertY = nearest_point(centers, vertices[ant_pos] - vecY);
        vecY = vertY - corner; 
        
        LAST = last = corner;
        
        pattern.push_back(last);

        //circle(drawing, vertX, 6, 255, -1);circle(drawing, vertY, 9, 255, -1);circle(drawing, corner, 4, 255, -1); 
        arrowedLine(processed, last, last + vecY, 230, 2); arrowedLine(processed, last, last + vecX, 230, 2);
        
        for(int j = 0; j<numCornersHor; j++){    
            if(j!=0){
                corner += vecX;
                               
                corner = nearest_point(centers, corner);
                actual = corner;
                
                pattern.push_back(actual);
                last = corner;
            }
            for(int i = 0; i<numCornersVer-1; i++){ 
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
    cout<<rms<<endl; cout<<intrinsic<<endl;    cout<<distCoeffs<<endl;
}

void Calibrator::gen_intrinsic_data(string data){
    namedWindow(src,1);
    namedWindow(pro,1);
    int frameNumbers = (int) cap.get(CV_CAP_PROP_FRAME_COUNT);
    ofstream coords("coords");
    I frame_count = 0, frames = 0;

    vector<vP2f> image_points;
    while(frameNumbers--){
        cap >> frame;
        processed = Mat::zeros( frame.size(), CV_8UC1 );
        preprocessing(th);
        get_pattern(1);
        int key = waitKey(TIME);
        bool b = false;
        if(key=='s'){
            b = true;
            while(b){
                int key = waitKey(1);
                if(key=='g')
                {
                    if (pattern.size() == RINGS){
                        image_points.push_back(pattern);
                        frames++;
                        for(auto p : pattern){
                            coords<<p.x<<" "<<p.y<<" ";
                        }
                        //fr_nums<<frame_count<<endl;
                        coords<<endl;
                    }
                    b=false;
                }

                if(key=='s'){
                    b = false;                
                    break;
                }
            }
        }
        draw(Scalar(0,255,255));
        //if( waitKey(TIME) == ESC ) break;
        frame_count++;
        if(frames==FRAMES_NUMBER)
            break;

    }

    if(frames==FRAMES_NUMBER){
        Size board_sz = Size(numCornersHor, numCornersVer);

        VideoCapture capture = VideoCapture(vid_name);

        vector<vP3f> object_points;

        Mat image;
        capture >> image;
        vector<Point3f> obj;
            
        for(int j=0;j<numSquares;j++){
            obj.push_back(Point3f(j/numCornersVer, j%numCornersVer, 0.0f));
            cout<<j/numCornersVer<<" "<<j%numCornersVer<<endl;
        }
        
        object_points.resize(FRAMES_NUMBER,obj);

        //read_coord_frames("coords", image_points,FRAMES_NUMBER);

        intrinsic = Mat(3, 3, CV_32FC1);
        vector<Mat> rvecs;
        vector<Mat> tvecs;

        intrinsic.ptr<float>(0)[0] = 1;
        intrinsic.ptr<float>(1)[1] = 1;
        cout<<image.size()<<endl;

        rms = calibrateCamera(object_points, image_points, image.size(), intrinsic, distCoeffs, rvecs, tvecs);
    }
    ofstream dat(data.c_str());
    dat<<rms<<endl;
    dat<<intrinsic.ptr<double>(0)[0]<<" "<<intrinsic.ptr<double>(0)[1]<<" "<<intrinsic.ptr<double>(0)[2]<<" ";
    dat<<intrinsic.ptr<double>(1)[0]<<" "<<intrinsic.ptr<double>(1)[1]<<" "<<intrinsic.ptr<double>(1)[2]<<" ";
    dat<<intrinsic.ptr<double>(2)[0]<<" "<<intrinsic.ptr<double>(2)[1]<<" "<<intrinsic.ptr<double>(2)[2]<<endl;
    dat<<distCoeffs.ptr<double>(0)[0]<<" "<<distCoeffs.ptr<double>(0)[1]<<" "<<distCoeffs.ptr<double>(0)[2]<<" "<<distCoeffs.ptr<double>(0)[3]<<" "<<distCoeffs.ptr<double>(0)[4]<<endl;
}

void Calibrator::draw_axis(){
    namedWindow(src,1);
    namedWindow(pro,1);
    int frameNumbers = (int) cap.get(CV_CAP_PROP_FRAME_COUNT);

    Mat rvec = Mat(Size(3,1), CV_64F);
    Mat tvec = Mat(Size(3,1), CV_64F);

    vector<Point2d> imageFramePoints, imageOrigin;
    vector<Point3d> obj, framePoints;
    for(int j=0;j<numSquares;j++) 
        obj.push_back(Point3f(j/numCornersHor, j%numCornersHor, 0.0));

    framePoints.push_back( Point3d( 0.0, 0.0, 0.0 ) );    
    framePoints.push_back( Point3d( 5.0, 0.0, 0.0 ) );
    framePoints.push_back( Point3d( 0.0, 5.0, 0.0 ) );    
    framePoints.push_back( Point3d( 0.0, 0.0, -5.0 ) );

    while(frameNumbers--){
        cap >> frame;
        processed = Mat::zeros( frame.size(), CV_8UC1 );
        preprocessing(105);
        get_pattern(1);
        if(pattern.size() == RINGS){
            solvePnP( Mat(obj), Mat(pattern), intrinsic, distCoeffs, rvec, tvec, false );
            projectPoints(framePoints, rvec, tvec, intrinsic, distCoeffs, imageFramePoints );

            circle(frame, (Point)pattern[0], 4 ,CV_RGB(255,0,0) );
             
            Point one, two, three;
            one.x=10; one.y=10;
            two.x = 60; two.y = 10;
            three.x = 10; three.y = 60;

            line(frame, one, two, CV_RGB(255,0,0) );
            line(frame, one, three, CV_RGB(0,255,0) );

            line(frame, imageFramePoints[0], imageFramePoints[1], CV_RGB(255,0,0), 2 );
            line(frame, imageFramePoints[0], imageFramePoints[2], CV_RGB(0,255,0), 2 );
            line(frame, imageFramePoints[0], imageFramePoints[3], CV_RGB(0,0,255), 2 );
        }
        draw(Scalar(0,255,255));
        if( waitKey(TIME) == ESC ) break;
    }
}

