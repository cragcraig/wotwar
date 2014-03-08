#include <stddef.h>
#include <stdio.h>
#include <allegro.h>

// functions
void init(void);
void gen_filename(char * base, char * form, int count, char * buf);

// Main
int main(int argc, char * argv[])
{	
	init(); // init text-mode allegro
	set_color_depth(32); // so any bitmaps we create are 32 bit
	
	// check that we got a filename
	if (argc != 2) {
		allegro_message("usage error! Must be run from command line with the following format.\n\nusage: imgm BASEFILENAME");
		return -1;
	}
	
	// get file format
	char form[512] = "%s%d.bmp";
	FILE * fp = fopen("imgm.conf", "r");
	if (fp) {
		fgets(form, 512, fp);
		if (strchr(form, '\n')) form[strlen(form)-1] = '\0';
		printf("filename format: '%s'\n", form);
		fclose(fp);
	} else {
		printf("imgm.conf not found, using default format '%s'\n", form);
	}
	
	// variables
	int width = 0;
	int height = 0;
	int count = 0;
	char buf[512];
	BITMAP *obuffer;
	BITMAP *tbuffer;
	BITMAP *tmp = NULL;
	
	while (1) {
		gen_filename(argv[1], form, count+1, buf);
		tmp = load_bitmap(buf, NULL);
		
		if (!tmp && !width && !height) { // no images
			allegro_message("Error: no images found.");
			return -1;
		} else if (!tmp) { // last image
			break;
		} else if (tmp && !width && !height) { // first image, set dimensions
			width = tmp->w;
			height = tmp->h;
			if (!width || !height) return -1;
			obuffer = create_bitmap_ex(32, width, height);
		}
		
		if (tmp && tmp->w == width && tmp->h == height) { // add image to output
			count++;
			puts(buf);
			
			// add image
			tbuffer = obuffer;
			obuffer = create_bitmap_ex(32, width * count, height);
			blit(tbuffer, obuffer, 0, 0, 0, 0, tbuffer->w, tbuffer->h);
			blit(tmp, obuffer, 0, 0, width * (count - 1), 0, tmp->w, tmp->h);
			
			// release tmp data
			destroy_bitmap(tbuffer);
			tbuffer = NULL;
			destroy_bitmap(tmp);
			tmp = NULL;
		} else { // dimension error
			allegro_message("Error: the images are not all of the same dimensions!");
			return -1;
		}
	}
	
	// save final image
	gen_filename(argv[1], form, 0, buf);
	if (!save_bmp(buf, obuffer, NULL)) printf("final image saved as '%s'\n", buf);
	
	
	// release data
	destroy_bitmap(obuffer);

	return 0;
}
END_OF_MAIN();

void gen_filename(char * base, char * form, int count, char * buf)
{
	if (!count) sprintf(buf, "%s_final.bmp", base); 
	else sprintf(buf, form, base, count);
}

void init(void)
{
	allegro_init();
	set_window_title("generating bitmap..."); // set window's title

	set_color_depth(32);
	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
}
