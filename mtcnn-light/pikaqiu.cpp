#include "network.h"
#include "mtcnn.h"
#include <time.h>
#pragma comment(lib, "libopenblas.dll.a")

#ifdef NDEBUG
//#pragma comment(lib, "libopenblas.lib")
#else
//#pragma comment(lib, "libopenblasd.lib")
#endif

int main()
{
	VideoCapture cap;
	cap.open("video_day_night.avi");

	namedWindow("result");
	waitKey();

	Mat frame;
	cap >> frame;
	mtcnn find(frame.cols, frame.rows);

	while (!frame.empty()){
		find.findFace(frame);
		imshow("result", frame);
		waitKey(1);
		cap >> frame;
	}
    return 0;
}