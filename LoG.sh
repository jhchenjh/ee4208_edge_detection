#! /bin/bash
# run canny edge detection
# arguments
# REQUIRED -path to input image           (.raw file)
# REQUIRED -path to output image          (.pgm file)
# REQUIRED -sigma for LoG kernel          (double, default 1.5)
# REQUIRED -kernel_size for LoG mask      (int, default 9)

./build/log raw_images/cana.raw \
            output/cana_LoG.pgm \
            1.5 \
            9
