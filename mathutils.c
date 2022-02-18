#include <stdint.h>
#include <stdlib.h>

#include "mathutils.h"

float *covariance3(float arr[][3], uint32_t num_elems) {
  float mean[3] = {0.0f};
  float cov = 0.0f;
  for (uint32_t i = 0; i < num_elems; i++) {
    mean[0] += arr[i][0] / num_elems;
    mean[1] += arr[i][1] / num_elems;
    mean[2] += arr[i][2] / num_elems;
  }
  for (uint32_t i = 0; i < num_elems; i++){
    
  }
}