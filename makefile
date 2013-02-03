#LDFLAGS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video -lopencv_ml -lopencv_features2d 
LDFLAGS = -I/usr/include/opencv  -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann
#TARGET = background
TARGET = detect
ARGS = data/output/*
#ARGS=data/S01_GENERAL_LOITERING_1/3/thirdView00*
#ARGS=data/S3-T7-A/video/pets2006/S3-T7-A/3/S3-T7-A.0*.jpeg

all: $(TARGET)

run::
	./$(TARGET) $(ARGS)

%: %.cpp
	g++ $< -o $@ -Wall -O2 $(LDFLAGS)
