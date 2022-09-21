all:
	g++ camshiftdemo.cpp -o camshiftdemo -lopencv_imgproc -lopencv_highgui -lopencv_video -lopencv_core
clean:
	rm a.out;rm *.o
