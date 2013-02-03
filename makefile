LDFLAGS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_video -lopencv_ml -lopencv_features2d
TARGET = background
#TARGET = frame
#ARGS=data/S01_GENERAL_LOITERING_1/3/thirdView00*
ARGS=data/S3-T7-A/video/pets2006/S3-T7-A/3/S3-T7-A.0*.jpeg

all: $(TARGET)

run::
	./$(TARGET) $(ARGS)

%: %.cpp
	g++ $< -o $@ -Wall -O2 $(LDFLAGS)
