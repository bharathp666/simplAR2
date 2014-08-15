//______________________________________________________________________________________
// Program : SimplAR 2 - OpenCV Simple Augmented Reality Program with Chessboard
// Author  : Bharath Prabhuswamy
//______________________________________________________________________________________

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define CHESSBOARD_WIDTH 6
#define CHESSBOARD_HEIGHT 5
//The pattern actually has 6 x 5 squares, but has 5 x 4 = 20 'ENCLOSED' corners

int main ( int argc, char **argv )
{

	Mat img;
	Mat display = imread("shingani.jpg");
	VideoCapture capture(0);

	Size board_size(CHESSBOARD_WIDTH-1, CHESSBOARD_HEIGHT-1);
    
	vector<Point2f> corners;

	if(display.empty())
	{
		cerr << "ERR: Unable to find overlay image.\n" << endl;
		return -1;
	}
	
	if ( !capture.isOpened() )
	{
		cerr << "ERR: Unable to capture frames from device 0" << endl;
		return -1;
	}
    
    int key = 0;
	
	while(key!='q')
	{
		// Query for a frame from Capture device
		capture >> img;

		Mat cpy_img(img.rows, img.cols, img.type());
		Mat neg_img(img.rows, img.cols, img.type());
		Mat gray;
		Mat blank(display.rows, display.cols, display.type());

        cvtColor(img, gray, CV_BGR2GRAY);
        
		bool flag = findChessboardCorners(img, board_size, corners);

		if(flag == 1)
		{            
			// This function identifies the chessboard pattern from the gray image, saves the valid group of corners
			cornerSubPix(gray, corners, Size(11,11), Size(-1,-1), TermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));
				
			vector<Point2f> src;			// Source Points basically the 4 end co-ordinates of the overlay image
			vector<Point2f> dst;			// Destination Points to transform overlay image	
			
			src.push_back(Point2f(0,0));
			src.push_back(Point2f(display.cols,0));
			src.push_back(Point2f(display.cols, display.rows));
			src.push_back(Point2f(0, display.rows));
	
			dst.push_back(corners[0]);
			dst.push_back(corners[CHESSBOARD_WIDTH-2]);
			dst.push_back(corners[(CHESSBOARD_WIDTH-1)*(CHESSBOARD_HEIGHT-1)-1]);
			dst.push_back(corners[(CHESSBOARD_WIDTH-1)*(CHESSBOARD_HEIGHT-2)]);
	
			// Compute the transformation matrix, 
			// i.e., transformation required to overlay the display image from 'src' points to 'dst' points on the image
			Mat warp_matrix = getPerspectiveTransform(src, dst);

			blank = Scalar(0);
			neg_img = Scalar(0);								// Image is white when pixel values are zero
			cpy_img = Scalar(0);								// Image is white when pixel values are zero

			bitwise_not(blank,blank);

			// Note the jugglery to augment due to OpenCV's limitation passing two images of DIFFERENT sizes while using "cvWarpPerspective"

			warpPerspective(display, neg_img, warp_matrix, Size(neg_img.cols, neg_img.rows));	// Transform overlay Image to the position	- [ITEM1]
			warpPerspective(blank, cpy_img, warp_matrix, Size(cpy_img.cols, neg_img.rows));		// Transform a blank overlay image to position	
			bitwise_not(cpy_img, cpy_img);							// Invert the copy paper image from white to black
			bitwise_and(cpy_img, img, cpy_img);						// Create a "hole" in the Image to create a "clipping" mask - [ITEM2]						
			bitwise_or(cpy_img, neg_img, img);						// Finally merge both items [ITEM1 & ITEM2]
	
		}

		imshow("Camera", img);
		key = cvWaitKey(1);	
	}
    
	destroyAllWindows();
	return 0;
}
