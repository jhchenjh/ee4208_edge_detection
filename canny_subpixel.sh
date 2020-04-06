#! /bin/bash
# run canny edge detection at subpixel accuracy
# arguments
# REQUIRED -path to input image           (.raw file)
# REQUIRED -path to output image          (.pgm file)
# OPTIONAL -output scale                  (double, between 1.0 to 2.0, default 1.0)
# OPTIONAL -sigma for Gaussian kernel     (double, default 1.0)
# OPTIONAL -high threshold                (double, default 15.0)
# OPTIONAL -low threshold                 (double, default 5.0)
# OPTIONAL -txt file to save edge points coordinates

./build/canny_devernay raw_images/cana.raw \
                       output/cana_subpixel_x1.pgm \
                       1.0 \
                       1.0 \
                       15.0 \
                       5.0 \
                       output/cana_edge_points.txt
