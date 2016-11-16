#include "MainHelper.h"

// Base code provided by tutorial at http://docs.opencv.org/3.1.0/d1/dc5/tutorial_background_subtraction.html

#define MOG 0
#define KNN 1
#define OPT_FLOW 2
#define PROCESS_METHOD KNN
#define SHOULD_BLUR 1
#define BLUR_STRUCT_SIZE 3
#define MORPH_STRUCT_SIZE 3
#define MORPH_OPERATION CV_MOP_OPEN
#define THRESHOLD_MIN 128
#define DETECT 0


int main(int argc, char* argv[]) {
	// Check for appropriate # input values
	if (argc < 3 || argc > 9) {
		std::cout << "ERROR: Invalid # of inputs." << std::endl;
		std::cout << "Usage: OpenCVSandbox -vid relative/path/to/video [0 = MOG | 1 = KNN] [0 = no blur | 1 = blur] blurAmt morphAmt shadowThreshold" << std::endl;
		return EXIT_FAILURE;
	}

	processInput(argc, argv);
	
	// Clean up GUI windows
	cv::destroyAllWindows();

	// Exit
	return EXIT_SUCCESS;
}

void processInput(int argc, char* argv[]) {
	method = PROCESS_METHOD;
	blur = SHOULD_BLUR;
	blurAmt = BLUR_STRUCT_SIZE;
	morphAmt = MORPH_STRUCT_SIZE;
	shadowThr = THRESHOLD_MIN;
	detect = DETECT;

	// Process input values
	switch (argc) {
	case 9:
		detect = atoi(argv[8]);
		// Correct input
		if (detect >= 0) {
			detect = 1;
		}
		else {
			detect = 0;
		}
	case 8:
		shadowThr = atoi(argv[7]);
		// Correct input
		if (shadowThr > 255) {
			shadowThr = 255;
		}
		else if (shadowThr < 0) {
			shadowThr = 0;
		}
	case 7:
		morphAmt = atoi(argv[6]);
	case 6:
		blurAmt = atoi(argv[5]);
	case 5:
		blur = atoi(argv[4]);
		// Correct input
		if (blur >= 0) {
			blur = 1;
		}
		else {
			blur = 0;
		}
	case 4:
		method = atoi(argv[3]);
		break;
	case 3:
	default:
		break;
	}

	// Process command
	if (strcmp(argv[1], "-vid") == 0) {
		processVideo(argv);
	}
	else if (strcmp(argv[1], "-img") == 0) {
		//processImages(argv[2]);
	}
	else {
		std::cout << "ERROR: Invalid input. Please select -vid or -img." << std::endl;
		exit(1);
	}
}

void processVideo(char* argv[]) {
	// Create GUI windows
	cv::namedWindow("Frame");
	if (detect) {
		cv::namedWindow("Frame Delta");
	}

	// Process video based on input method
	switch (method) {
	case MOG:
		pMOG2 = cv::createBackgroundSubtractorMOG2();
		cv::namedWindow("FG Mask MOG 2");
		processVideoSubtr(argv[2]);
		break;
	case KNN:
		pKNN = cv::createBackgroundSubtractorKNN();
		cv::namedWindow("FG Mask KNN");
		processVideoSubtr(argv[2]);
		break;
	case OPT_FLOW:
		cv::namedWindow("Optical Flow");
		processVideoOF(argv[2]);
		break;
	default:
		std::cout << "ERROR: Invalid processing method." << std::endl;
		break;
	}
}

void processVideoSubtr(char* videoFilename) {
	cv::Mat prevFrame, tmpMask, frameDelta, thresh, fgMask;
	std::vector<std::vector<cv::Point>> contours;
	bool firstFrameSet = false;
	float thr = 20.0f;
	float dist;

	// Create video capture object
	cv::VideoCapture capture(videoFilename);

	// Check for successful video opening
	if (!capture.isOpened()) {
		std::cout << "ERROR: Failed to open video file: " << videoFilename << std::endl;
		exit(EXIT_FAILURE);
	}
	

	// Read input as long as the user doesn't enter 'q' (for quit) or ESC
	while (((char)keyboard != 'q') && ((char)keyboard != 27)) {
		// Attempt to read current frame
		if (!capture.read(currFrame)) {
			std::cout << "ERROR: Failed to read next frame." << std::endl;
			exit(EXIT_FAILURE);
		}

		// Convert to GS
		cv::cvtColor(currFrame, currFrame, cv::COLOR_BGR2GRAY);

		// Blur
		if (blur) {
			cv::GaussianBlur(currFrame, currFrame, cv::Size(blurAmt, blurAmt), 0);
			//cv::blur(grayFrame, grayFrame, cv::Size(blurAmt, blurAmt));
		}

		// Update background model (FG mask)
		if (method == MOG) {
			pMOG2->apply(currFrame, fgMask);
		}
		else if (method == KNN) {
			pKNN->apply(currFrame, fgMask);
		}

		// Morph mask
		cv::morphologyEx(fgMask, tmpMask, MORPH_OPERATION, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(morphAmt, morphAmt)));

		// Remove shadows
		cv::threshold(tmpMask, tmpMask, shadowThr, 255, CV_THRESH_BINARY);

		if (detect) {
			// Get frame difference
			if (!firstFrameSet) {
				firstFrameSet = true;
				cv::absdiff(tmpMask, tmpMask, frameDelta);
			}
			else {
				cv::absdiff(prevFrame, tmpMask, frameDelta);
			}

			cv::morphologyEx(frameDelta, frameDelta, CV_MOP_DILATE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(morphAmt, morphAmt)), cv::Point(-1, -1), 1);
			cv::morphologyEx(frameDelta, frameDelta, CV_MOP_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(morphAmt, morphAmt)));

			fgMask = cv::Mat::zeros(frameDelta.rows, frameDelta.cols, tmpMask.type());

			for (int i = 0; i < frameDelta.rows; i++) {
				for (int j = 0; j < frameDelta.cols; j++) {
					unsigned char pix = frameDelta.at<unsigned char>(i, j);

					if (pix > thr) {
						fgMask.at<unsigned char>(i, j) = 255;
					}
				}
			}

			tmpMask.copyTo(prevFrame);

			// Dilate
			cv::dilate(fgMask, fgMask, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(morphAmt, morphAmt)), cv::Point(0, 0), 1);

			// Find contours
			cv::findContours(fgMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

			// Loop over all contours
			for (int i = 0; i < contours.size(); i++) {
				// Check if size qualifies
				if (cv::contourArea(contours.at(i)) < 175.0) {
					continue;
				}
				else { // If so, display rectangle
					cv::Rect objectRect = cv::boundingRect(contours.at(i));
					cv::rectangle(currFrame, cv::Rect(objectRect.x, objectRect.y, objectRect.width, objectRect.height), cv::Scalar(0, 255, 0), 2);
				}
			}
		}

		// Show current frame and foreground masks
		cv::imshow("Frame", currFrame);
		if (detect) {
			cv::imshow("Frame Delta", frameDelta);
		}
		
		if (method == MOG) {
			cv::imshow("FG Mask MOG 2", tmpMask);
		}
		else if (method == KNN) {
			cv::imshow("FG Mask KNN", tmpMask);
		}

		// Get keyboard input
		keyboard = cv::waitKey(30);
	}

	capture.release();
}

void processVideoOF(char* videoFilename) {
	cv::Mat prevFrame, nextFrame, flow, mag, angle, xy[2], hsv1[3], hsv2, bgr;
	double magMax;

	// Create video capture object
	cv::VideoCapture capture(videoFilename);

	// Check for successful video opening
	if (!capture.isOpened()) {
		std::cout << "ERROR: Failed to open video file: " << videoFilename << std::endl;
		exit(EXIT_FAILURE);
	}

	// Set 1st frame
	if (!capture.read(currFrame)) {
		std::cout << "ERROR: Failed to read next frame." << std::endl;
		exit(EXIT_FAILURE);
	}

	// Blur
	if (blur) {
		cv::GaussianBlur(currFrame, currFrame, cv::Size(blurAmt, blurAmt), 0);
		//cv::blur(grayFrame, grayFrame, cv::Size(blurAmt, blurAmt));
	}

	// Convert to grayscale
	cv::cvtColor(currFrame, prevFrame, cv::COLOR_BGR2GRAY);

	// Read input as long as the user doesn't enter 'q' (for quit) or ESC
	while (((char)keyboard != 'q') && ((char)keyboard != 27)) {
		// Attempt to read current frame
		if (!capture.read(currFrame)) {
			std::cout << "ERROR: Failed to read next frame." << std::endl;
			exit(EXIT_FAILURE);
		}

		// Blur
		if (blur) {
			cv::GaussianBlur(currFrame, currFrame, cv::Size(blurAmt, blurAmt), 0);
			//cv::blur(grayFrame, grayFrame, cv::Size(blurAmt, blurAmt));
		}

		// Convert next frame to grayscale
		cv::cvtColor(currFrame, nextFrame, cv::COLOR_BGR2GRAY);

		// Calculate optical flow
		cv::calcOpticalFlowFarneback(prevFrame, nextFrame, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

		// Copy next frame to prev frame (for next calculation)
		nextFrame.copyTo(prevFrame);

		// Split optical flow into two channels
		cv::split(flow, xy);

		// Calculate magnitude and angle of flow
		cv::cartToPolar(xy[0], xy[1], mag, angle, true);

		// Normalize magnitude
		cv::minMaxLoc(mag, 0, &magMax);
		mag.convertTo(mag, -1, 1.0 / magMax);

		// Set HSV channel values
		hsv1[0] = angle;
		hsv1[1] = cv::Mat::ones(angle.size(), CV_32F);
		hsv1[2] = mag;

		// Merge into single HSV
		cv::merge(hsv1, 3, hsv2);

		// Convert to BGR
		cv::cvtColor(hsv2, bgr, cv::COLOR_HSV2BGR);

		cv::imshow("Frame", currFrame);
		cv::imshow("Optical Flow", bgr);

		// Get keyboard input
		keyboard = cv::waitKey(30);
	}

	capture.release();
}