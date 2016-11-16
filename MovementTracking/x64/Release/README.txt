MovementTracking:

Reads a video file and subtracts the background, creating a mask of moving objects.
Both the video and mask are displayed in windows.

Usage:
1. Navigate to folder with MovementTracking files in cmd
2. Run as follows:
	[1] [2] [3] [4] [5] [6] [7] [8]
	Required:
		[1] Exe 		= 	MovementTracking
		[2] Type of input 	= 	-vid
		[3] Relative path to video =	/Rel/Path/To/Vid
	Optional:
		[4] Subtraction method to use
			0 - MOG
				Faster
				Noisier
				Faster background subtraction
			1 - KNN
				Slower
				Less noise
				Better separation
		[5] Whether or not to blur the source video (reduces noise)
			0 - No Blur
			1 - Blur
		[6] Amount of blur
			1 - Very little
			8+ - A lot
		[7] Amount of pixel morph (grouping)
			1 - Very little
			6+ - A lot
		[8] Shadow threshold (point at which shadows are not added to mask)
			0 - Min
			128 - Average
			255 - Max

Note: [4] - [8] are not required to run (uses defaults otherwise), but if used, 
must be in order (i.e. to use [6], you must first specify [4] and [5], but [5] 
can be specified without [6]).


Examples:
	C:\Development\Workspace\MovementTracking\x64\Release>MovementTracking -vid oneway.wmv 0 1 5 5 128
	C:\Development\Workspace\MovementTracking\x64\Release>MovementTracking -vid oneway.wmv 1 0 1 6 64