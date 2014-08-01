#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "common.h"
#include "viz.h"

using namespace cv;
using namespace std;

static Mat s_img;
static Mat s_img_gs;
static Mat s_img_gs_trans;
static Mat s_thresh1, s_thresh2, s_thresh;
static int s_size;
static float s_scale_high = 0.2;
static float s_scale_low = -0.2;
static int s_thresh_high = 160;
static int s_thresh_low = 70;
static string s_winname = "Image";

void viz_init(int row, int col) {
  s_img = Mat::zeros(row, col, CV_32FC1);
  s_size = row * col;
}


void viz_viz(float *data) {

  // Copy data to matrix
  assert(s_img.isContinuous());
  memcpy(s_img.data, data, s_size*sizeof(float));

  imshow("raw image", s_img);
  printf("Image: %d x %d\n", s_img.rows, s_img.cols);
  
  // Convert image to grayscale
  s_img.convertTo(s_img_gs, CV_8UC1, 255.0/(s_scale_high-s_scale_low), -255.0/(s_scale_high-s_scale_low)*s_scale_low);
  imshow("grayscale", s_img_gs);

  // Threshold image to get edges
  // Set high and low values to 255, others to 0
  threshold(s_img_gs, s_thresh1, s_thresh_high, 255, THRESH_BINARY);
  threshold(s_img_gs, s_thresh2, s_thresh_low, 255, THRESH_BINARY_INV);
  s_thresh = s_thresh1 + s_thresh2;
  imshow("thresh", s_thresh);
  
  // Get contours
  vector<vector<Point> > contours;
  findContours(s_thresh, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
  applyColorMap(s_img_gs, s_img_gs, COLORMAP_JET);
  drawContours(s_img_gs, contours, -1, Scalar(255, 255, 255), 2);

  transpose(s_img_gs, s_img_gs_trans);
  imshow(s_winname, s_img_gs_trans);

  waitKey(0);
}
  
  

  
