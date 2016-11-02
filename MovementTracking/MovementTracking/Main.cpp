#include "MainHelper.h"

// Base code provided by tutorial at http://docs.opencv.org/3.1.0/d1/dc5/tutorial_background_subtraction.html

#define MOG 0
#define KNN 1
#define SUBTRACT_METHOD KNN
#define SHOULD_BLUR 1
#define BLUR_STRUCT_SIZE 3
#define MORPH_STRUCT_SIZE 3
#define MORPH_OPERATION CV_MOP_CLOSE
#define THRESHOLD_MIN 128


int main(int argc, char* argv[]) {
	// Check for appropriate # input values
	if (argc < 3 || argc > 8) {
		std::cout << "ERROR: Invalid # of inputs." << std::endl;
		std::cout << "Usage: OpenCVSandbox -vid relative/path/to/video [0 = MOG | 1 = KNN] [0 = no blur | 1 = blur] blurAmt morphAmt shadowThreshold" << std::endl;
		return EXIT_FAILURE;
	}

	method = SUBTRACT_METHOD;
	blur = SHOULD_BLUR;
	blurAmt = BLUR_STRUCT_SIZE;
	morphAmt = MORPH_STRUCT_SIZE;
	shadowThr = THRESHOLD_MIN;

	switch (argc) {
	case 8:
		shadowThr = atoi(argv[7]);
	case 7:
		morphAmt = atoi(argv[6]);
	case 6:
		blurAmt = atoi(argv[5]);
	case 5:
		blur = atoi(argv[4]);
	case 4:
		method = atoi(argv[3]);
		break;
	case 3:
	default:
		break;
	}

	// Create GUI windows
	cv::namedWindow("Frame");

	if (strcmp(argv[1], "-vid") == 0) {
		switch (method) {
		case MOG:
			pMOG2 = cv::createBackgroundSubtractorMOG2();
			cv::namedWindow("FG Mask MOG 2");
			processVideoMOG(argv[2]);
			break;
		case KNN:
			pKNN = cv::createBackgroundSubtractorKNN();
			cv::namedWindow("FG Mask KNN");
			processVideoKNN(argv[2]);
			break;
		default:
			break;
		}
	}
	else if (strcmp(argv[1], "-img") == 0) {
		//processImages(argv[2]);
	}
	else {
		std::cout << "ERROR: Invalid input. Please select -vid or -img." << std::endl;
		return EXIT_FAILURE;
	}
	// Clean up GUI windows
	cv::destroyAllWindows();

	// Exit
	return EXIT_SUCCESS;
}

void processVideoMOG(char* videoFilename) {
	cv::Mat tmpMog;
	// Create video capture object
	cv::VideoCapture capture(videoFilename);

	// Check for successful video opening
	if (!capture.isOpened()) {
		std::cout << "ERROR: Failed to open video file: " << videoFilename << std::endl;
		exit(EXIT_FAILURE);
	}

	// Read input as long as the user doesn't enter 'q' (for quit) or ESC
	while (((char) keyboard != 'q') && ((char) keyboard != 27)) {
		// Attempt to read current frame
		if (!capture.read(currFrame)) {
			std::cout << "ERROR: Failed to read next frame." << std::endl;
			exit(EXIT_FAILURE);
		}

		if (blur) {
			cv::blur(currFrame, currFrame, cv::Size(blurAmt, blurAmt));
		}

		// Update background model
		pMOG2->apply(currFrame, fgMaskMOG2);

		cv::morphologyEx(fgMaskMOG2, tmpMog, MORPH_OPERATION, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(morphAmt, morphAmt), cv::Point(-1, -1)));
		cv::threshold(tmpMog, tmpMog, shadowThr, 255, CV_THRESH_BINARY);

		// Show current frame and foreground masks
		cv::imshow("Frame", currFrame);
		cv::imshow("FG Mask MOG 2", tmpMog);

		// Get keyboard input
		keyboard = cv::waitKey(30);
	}

	capture.release();
}

void processVideoKNN(char* videoFilename) {
	cv::Mat tmpKNN;
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

		// Blur image (removes some noise)
		if (blur) {
			cv::blur(currFrame, currFrame, cv::Size(blurAmt, blurAmt));
		}

		// Update background model
		pKNN->apply(currFrame, fgMaskKNN);

		// Morph pixel groups
		cv::morphologyEx(fgMaskKNN, tmpKNN, MORPH_OPERATION, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(morphAmt, morphAmt), cv::Point(0, 0)));

		// Remove shadows
		cv::threshold(tmpKNN, tmpKNN, shadowThr, 255, CV_THRESH_BINARY);

		// Show current frame and foreground masks
		cv::imshow("Frame", currFrame);
		cv::imshow("FG Mask KNN", tmpKNN);

		// Get keyboard input
		keyboard = cv::waitKey(30);
	}

	capture.release();
}