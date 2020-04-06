#! /bin/bash
# run canny edge detection
# arguments
# REQUIRED -path to input image           (.raw file)
# REQUIRED -path to output image          (.pgm file)
# REQUIRED -sigma for Gaussian kernel     (double, default 1.0)
# REQUIRED -low threshold                 (double)
# REQUIRED -high threshold                (double)

./build/canny raw_images/cana.raw \
              output/cana_canny.pgm \
              1.0 \
              20.0 \
              80.0
