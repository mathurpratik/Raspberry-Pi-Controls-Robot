#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <termios.h> /* POSIX terminal control definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

Mat image;

bool backprojMode = false;
bool selectObject = false;
int trackObject = 0;
bool showHist = true;
Point origin;
Rect selection;
int vmin = 10, vmax = 256, smin = 30;

void sendTurnRight(int fd){
    int n = write(fd, "R", 1);
    if (n < 0)
        fputs("write() of 1 byte (R) failed!\n", stderr);
}

void sendTurnLeft(int fd){
    int n = write(fd, "L", 1);
    if (n < 0)
        fputs("write() of 1 bytes (L) failed!\n", stderr);
}

/*
 * 'open_port()' - Open serial port 1.
 *
 * Returns the file descriptor on success or -1 on error.
 */

int open_port(void) {
    int fd; /* File descriptor for the port */
    
    
    fd = open("/dev/tty.usbmodem1a21", O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        /*
         * Could not open the port.
         */
        
        perror("open_port: Unable to open /dev/cu.usbmodem1d11 - ");
    } else
        fcntl(fd, F_SETFL, 0);
    
    return (fd);
}

void onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )
    {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);
        
        selection &= Rect(0, 0, image.cols, image.rows);
    }
    
    switch( event )
    {
        case CV_EVENT_LBUTTONDOWN:
            origin = Point(x,y);
            selection = Rect(x,y,0,0);
            selectObject = true;
            break;
        case CV_EVENT_LBUTTONUP:
            selectObject = false;
            if( selection.width > 0 && selection.height > 0 )
                trackObject = -1;
            break;
    }
}

void help()
{
    cout << "\nThis is a demo that shows mean-shift based tracking\n"
    "You select a color objects such as your face and it tracks it.\n"
    "This reads from video camera (0 by default, or the camera number the user enters\n"
    "Usage: \n"
    "	./camshiftdemo [camera number]\n";
    
    cout << "\n\nHot keys: \n"
    "\tESC - quit the program\n"
    "\tc - stop the tracking\n"
    "\tb - switch to/from backprojection view\n"
    "\th - show/hide object histogram\n"
    "\tp - pause video\n"
    "To initialize tracking, select the object with mouse\n";
}

const char* keys = 
{
	"{1|  | 0 | camera number}"
};

int main( int argc, const char** argv )
{
    
    
    
	help();
    cout << "after help()" << endl;
    int fd = open_port();
    cout << "after open port()" << endl;
    struct termios options;
    
    /*
     * Get the current options for the port...
     */
    
    tcgetattr(fd, &options);
    
    /*
     * Set the baud rates to 19200...
     */
    
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);
    
    /*
     * Enable the receiver and set local mode...
     */
    
    options.c_cflag |= (CLOCAL | CREAD);
    
    /*
     * Set the new options for the port...
     */
    
    tcsetattr(fd, TCSANOW, &options);
    
    //sendTurnRight(fd);
    //sendTurnLeft(fd);
    
    
    
    //return 0;
    
    VideoCapture cap;

    Rect trackWindow;
    RotatedRect trackBox;
    int hsize = 16;
    float hranges[] = {0,180};
    const float* phranges = hranges;
	CommandLineParser parser(argc, argv, keys);
	int camNum = parser.get<int>("1");    
    bool pointsLeft = true;
	
	cap.open(camNum);
    
    if( !cap.isOpened() )
    {
    	help();
        cout << "***Could not initialize capturing...***\n";
        cout << "Current parameter's value: \n";
		parser.printParams();
        return -1;
    }
    cout << "width = " << cap.get(CV_CAP_PROP_FRAME_WIDTH) << endl;
    cout << "height = " << cap.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;
    
    double half_width = cap.get(CV_CAP_PROP_FRAME_WIDTH)/2;
    double half_height = cap.get(CV_CAP_PROP_FRAME_HEIGHT)/2;
    
    cout <<"1/2 width = " << half_width << endl;
    cout <<"1/2 height = " << half_height << endl;
    namedWindow( "Histogram", 0 );
    namedWindow( "CamShift Demo", 0 );
    //cvResizeWindow( "CamShift Demo", 250,250 );
    setMouseCallback( "CamShift Demo", onMouse, 0 );
    createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
    createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
    createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );
    
    Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
    bool paused = false;
    
    for(;;)
    {
        if( !paused )
        {
            cap >> frame;
            if( frame.empty() )
                break;
        }

        frame.copyTo(image);
        
        if( !paused )
        {
            cvtColor(image, hsv, CV_BGR2HSV);
            
            if( trackObject )
            {
                int _vmin = vmin, _vmax = vmax;
                
                inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
                        Scalar(180, 256, MAX(_vmin, _vmax)), mask);
                int ch[] = {0, 0};
                hue.create(hsv.size(), hsv.depth());
                mixChannels(&hsv, 1, &hue, 1, ch, 1);
                
                if( trackObject < 0 )
                {
                    Mat roi(hue, selection), maskroi(mask, selection);
                    calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
                    normalize(hist, hist, 0, 255, CV_MINMAX);
                    
                    trackWindow = selection;
                    trackObject = 1;
                    
                    histimg = Scalar::all(0);
                    int binW = histimg.cols / hsize;
                    Mat buf(1, hsize, CV_8UC3);
                    for( int i = 0; i < hsize; i++ )
                        buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
                    cvtColor(buf, buf, CV_HSV2BGR);
                    
                    for( int i = 0; i < hsize; i++ )
                    {
                        int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
                        rectangle( histimg, Point(i*binW,histimg.rows),
                                  Point((i+1)*binW,histimg.rows - val),
                                  Scalar(buf.at<Vec3b>(i)), -1, 8 );
                    }
                }
                
                calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
                backproj &= mask;
                RotatedRect trackBox = CamShift(backproj, trackWindow,
                                                TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
                Point center= trackBox.center;
                cout << "("<<center.x<<","<<center.y<<")"<<endl;
                
                // SHOULD POINT LEFT
                if (center.x < half_width){
                    // front wheel should be turned LEFT
                    if (pointsLeft == true){
                        // no op
                        
                    }
                    else{
                        // front wheels should be turned RIGHT
                        
                        cout << "LEFT" << endl;
                        sendTurnLeft(fd);
                        pointsLeft = true;
                        
                    }
                }
                
                // SHOULD POINT RIGHT
                if (center.x > half_width){
                    if (pointsLeft == false){
                        // no op
                    }
                    else {
                        cout << "RIGHT" << endl;
                        sendTurnRight(fd);
                        pointsLeft = false;
                    }
                }
                    
                if( trackWindow.area() <= 1 )
                {
                    int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
                    trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                                       trackWindow.x + r, trackWindow.y + r) &
                    Rect(0, 0, cols, rows);
                }   
                
                if( backprojMode )
                    cvtColor( backproj, image, CV_GRAY2BGR );
                
                ellipse( image, trackBox, Scalar(0,0,255), 3, CV_AA );
            }
        }
        else if( trackObject < 0 )
            paused = false;
        
        if( selectObject && selection.width > 0 && selection.height > 0 )
        {
            Mat roi(image, selection);
            bitwise_not(roi, roi);
        }
        
        imshow( "CamShift Demo", image );
        imshow( "Histogram", histimg );
        
        char c = (char)waitKey(10);
        if( c == 27 )
            break;
        switch(c)
        {
            case 'b':
                backprojMode = !backprojMode;
                break;
            case 'c':
                trackObject = 0;
                histimg = Scalar::all(0);
                break;
            case 'h':
                showHist = !showHist;
                if( !showHist )
                    destroyWindow( "Histogram" );
                else
                    namedWindow( "Histogram", 1 );
                break;
            case 'p':
                paused = !paused;
                break;
            default:
                ;
        }
    }
    
    return 0;
}
