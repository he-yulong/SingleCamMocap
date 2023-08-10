//
// Copyright 2022 Adam Horvath - POSE.UPLUGINS.COM - info@uplugins.com - All Rights Reserved.
//

#include <vector>

#if defined _WIN32 || defined _WIN64
#define MEDIAPIPEDLL_API __declspec(dllimport)
#elif defined __linux__
#define MEDIAPIPEDLL_API __attribute__((visibility("default")))
#else
#define MEDIAPIPEDLL_API
#endif


MEDIAPIPEDLL_API bool processFrame(int rows, int cols, uint8* raw_frame, std::vector<std::vector<double>>& pose_landmarks_vector);
MEDIAPIPEDLL_API bool initGraph(LPCWSTR base_dir);
MEDIAPIPEDLL_API bool shutdownGraph();