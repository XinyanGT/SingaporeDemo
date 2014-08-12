#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "common.h"
#include "viz.h"

using namespace cv;
using namespace std;

class VIZCPP {
private:
  Mat img;
  Mat img_gs;
  Mat img_gs_trans;
  Mat thresh1, thresh2, thresh;
  int size;
  float scale_high;
  float scale_low;
  int thresh_high;
  int thresh_low;
  string winname;

public:
  VIZCPP(int row, int col, float sh, float sl, int th, int tl);
  void viz(float *data);
  
};


VIZCPP::VIZCPP(int row, int col, float sh, float sl, int th, int tl) {
  img = Mat::zeros(row, col, CV_32FC1);
  size = row * col;
  scale_high = sh;
  scale_low = sl;
  thresh_high = th;
  thresh_low = tl;
  winname = "Image";
}

void VIZCPP::viz(float *data) {

  // Copy data to matrix
  assert(img.isContinuous());
  memcpy(img.data, data, size*sizeof(float));

  // imshow("raw image", img);
  printf("Image: %d x %d\n", img.rows, img.cols);
  
  // Convert image to grayscale
  img.convertTo(img_gs, CV_8UC1, 255.0/(scale_high-scale_low), -255.0/(scale_high-scale_low)*scale_low);
  // imshow("grayscale", img_gs);

  // Threshold image to get edges
  // Set high and low values to 255, others to 0
  threshold(img_gs, thresh1, thresh_high, 255, THRESH_BINARY);
  threshold(img_gs, thresh2, thresh_low, 255, THRESH_BINARY_INV);
  thresh = thresh1 + thresh2;
  // imshow("thresh", thresh);
  
  // Get contours
  vector<vector<Point> > contours;
  findContours(thresh, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
  applyColorMap(img_gs, img_gs, COLORMAP_JET);
  drawContours(img_gs, contours, -1, Scalar(255, 255, 255), 2);

  transpose(img_gs, img_gs_trans);
  imshow(winname, img_gs_trans);

  waitKey(0);
}


/************************************************************
 * C interface
 ************************************************************/
VIZ *viz_new(int row, int col, float scale_high, float scale_low, int thresh_high, int thresh_low) {
  VIZCPP *vizcpp_p = new VIZCPP(row, col, scale_high, scale_low, thresh_high, thresh_low);
  VIZ *viz_p = (VIZ *) malloc(sizeof(VIZ));
  viz_p->cpp_p = (void *)vizcpp_p;
  return viz_p;
}

void viz_viz(VIZ *viz_p, float *data) {
  VIZCPP *vizcpp_p = (VIZCPP *)(viz_p->cpp_p);
  vizcpp_p->viz(data);
}

void viz_free(VIZ *viz_p) {
  VIZCPP *vizcpp_p = (VIZCPP *) (viz_p->cpp_p);
  delete vizcpp_p;
  free(viz_p);
}
  
  

