#include <gd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char* argv[])
{
    FILE *fp, *fpp;
    gdImagePtr in, out, image_p;
    int ow, oh, nw, nh, width, height, file_type, jpg_quality;
    int reply = 1;
    int sfw = 1;

    const int MAX_W = 250;
    const int MAX_H = 250;
    const int MAXR_W = 125;
    const int MAXR_H = 125;

    /* Help */
    if (!(argc == 3 || argc == 4 || argc == 5)) {
        printf("Generate thumbnails from 4chan\n");
        printf("%s  <input-file>  <output-file>  [reply?1:0]  [sfw-board?1:0]\n", argv[0]);
        return 1;
    }

    /* Reply Check */
    if (argc > 3) {
        reply = atoi(argv[3]);
        if (argc > 4) {
            sfw = atoi(argv[4]);
        }
    }
    if (reply <= 0) {
        width = MAX_W; // Max output width
        height = MAX_H; //Max output height
        jpg_quality = 50;
    }
    else {
        width = MAXR_W; // Max output width (imgreply)
        height = MAXR_H; // Max output height (imgreply)
        jpg_quality = 40;
    }

    /* Input */
    fp = fopen(argv[1], "rb");
    fpp = fopen(argv[1], "rb"); // A clone so fgetc doesn't consume the original
    if (!fp || !fpp) {
        fprintf(stderr, "An error occurred trying to open the file %s\n", argv[1]);
        return 3;
    }

    int i1 = fgetc(fpp);
    int i2 = fgetc(fpp);
    int i3 = fgetc(fpp);
    int i4 = fgetc(fpp);
    int i5 = fgetc(fpp);
    int i6 = fgetc(fpp);

    // https://en.wikipedia.org/wiki/Magic_number_%28programming%29
    if (i1 == 0x89 && i2 == 0x50) {
        // PNG: 89 50 4E 47 0D 0A 1A 0A
        // Starts with 0x89 0x50
        // printf("PNG\n");
        in = gdImageCreateFromPng(fp);
        file_type = 3;
    }
    else if (i1 == 0x47 && i2 == 0x49 && i3 == 0x46 && i4 == 0x38 && (i5 == 0x39 || i5 == 0x37) && i6 == 0x61) {
        // GIF: 47 49 46 38 39 61
        // GIF: 47 49 46 38 37 61
        // printf("GIF\n");
        in = gdImageCreateFromGif(fp);
        file_type = 1;
    }
    else if (i1 == 0xFF && i2 == 0xD8) {
        // JPEG: FF D8
        // printf("JPG\n");
        in = gdImageCreateFromJpeg(fp);
        file_type = 2;
    }
    else {
        fclose(fpp);
        fclose(fp);
        fprintf(stderr, "Unsupported file.\n");
        return 1;
    }

    if (!in) {
        fprintf(stderr, "An error occurred trying to access the file %s\n", argv[1]);
        return 1;
    }

    ow = gdImageSX(in);
    oh = gdImageSY(in);

    /* Resize */
    if (ow > width || oh > height || file_type == 1) {
        double key_w = (double)width / ow;
        double key_h = (double)height / oh;
        double keys;
        if (key_w < key_h) {
            keys = key_w;
        }
        else {
            keys = key_h;
        }
        nw = floor(ow * keys);
        nh = floor(oh * keys);
    }
    else {
        nw = ow;
        nh = nh;
    }

    image_p = gdImageCreateTrueColor(nw, nh);

    /* Transparency */
    if (file_type == 3 || file_type == 1) { /* PNG or GIF */
        if (reply <= 0) {  /* OP */
            if (sfw >= 1) {
                gdImageFill(image_p, 0, 0, 0xEEF2FF); // Blue board OP
            } else {
                gdImageFill(image_p, 0, 0, 0xFFFFEE); // Orange board OP
            }
        } else { /* Reply */
            if (sfw >= 1) {
                gdImageFill(image_p, 0, 0, 0xD6DAF0); // Blue board reply
            } else {
                gdImageFill(image_p, 0, 0, 0xF0E0D6); // Orange board Reply
            }
        }
    }

    gdImageCopyResampled(image_p, in, 0, 0, 0, 0, nw, nh, ow, oh);

    /* Debug aspect ratio */
    /*printf("%d x %d\n",ow,oh);
      printf("%d x %d\n",nw,nh);
      printf("%d x %d\n",gdImageSX(image_p),gdImageSY(image_p));*/

    fclose(fpp);
    fclose(fp);
    if (nw <= 0 || nh <= 0 ) {
        fprintf(stderr, "Input:\t%d x %d\n",ow,oh);
        fprintf(stderr, "Output:\t%d x %d\n",nw,nh);
        fprintf(stderr, "An error has occurred  .\n");
        return 1;
    }


    /* Output */
    fp = fopen(argv[2], "wb");
    if (!fp) {
        fprintf(stderr, "Error saving image %s\n", argv[2]);
        return 1;
    }
    gdImageJpeg(image_p, fp, jpg_quality);
    fclose(fp);

    /* Cleanups */
    gdImageDestroy(in);
    gdImageDestroy(image_p);

    return 0;
}
