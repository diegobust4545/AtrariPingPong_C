/*
 * square4.c
 * this program lets you move the square, with vsync
 */

/* the width and height of the screen */
#define WIDTH 240
#define HEIGHT 160

/* these identifiers define different bit positions of the display control */
#define MODE4 0x0004
#define BG2 0x0400

/* this bit indicates whether to display the front or the back buffer
 * this allows us to refer to bit 4 of the display_control register */
#define SHOW_BACK 0x10;

/* the screen is simply a pointer into memory at a specific address this
 *  * pointer points to 16-bit colors of which there are 240x160 */
volatile unsigned short* screen = (volatile unsigned short*) 0x6000000;

/* the display control pointer points to the gba graphics register */
volatile unsigned long* display_control = (volatile unsigned long*) 0x4000000;

/* the address of the color palette used in graphics mode 4 */
volatile unsigned short* palette = (volatile unsigned short*) 0x5000000;

/* pointers to the front and back buffers - the front buffer is the start
 * of the screen array and the back buffer is a pointer to the second half */
volatile unsigned short* front_buffer = (volatile unsigned short*) 0x6000000;
volatile unsigned short* back_buffer = (volatile unsigned short*)  0x600A000;

/* the button register holds the bits which indicate whether each button has
 * been pressed - this has got to be volatile as well
 */
volatile unsigned short* buttons = (volatile unsigned short*) 0x04000130;

/* the bit positions indicate each button - the first bit is for A, second for
 * B, and so on, each constant below can be ANDED into the register to get the
 * status of any one button */
#define BUTTON_A (1 << 0)
#define BUTTON_B (1 << 1)
#define BUTTON_SELECT (1 << 2)
#define BUTTON_START (1 << 3)
#define BUTTON_RIGHT (1 << 4)
#define BUTTON_LEFT (1 << 5)
#define BUTTON_UP (1 << 6)
#define BUTTON_DOWN (1 << 7)
#define BUTTON_R (1 << 8)
#define BUTTON_L (1 << 9)
 // Determines Paddle Movement
 int bool = 1;
 // Determines if ball is preparing to launch
 int ballBool = 1;
 // Determines if ball goes left or right
 int dirBool;
 int lrBool = 1;

 int startClear = 1;


/* the scanline counter is a memory cell which is updated to indicate how
 * much of the screen has been drawn */
volatile unsigned short* scanline_counter = (volatile unsigned short*) 0x4000006;

/* wait for the screen to be fully drawn so we can do something during vblank */
void wait_vblank( ) {
    /* wait until all 160 lines have been updated */
    while (*scanline_counter < 160) { }
}



/* this function checks whether a particular button has been pressed */
unsigned char button_pressed(unsigned short button) {
    /* and the button register with the button constant we want */
    unsigned short pressed = *buttons & button;

    /* if this value is zero, then it's not pressed */
    if (pressed == 0) {
        return 1;
    } else {
        return 0;
    }
}


/* keep track of the next palette index */
int next_palette_index = 0;

/*
 * function which adds a color to the palette and returns the
 * index to it
 */
unsigned char add_color(unsigned char r, unsigned char g, unsigned char b) {
    unsigned short color = b << 10;
    color += g << 5;
    color += r;

    /* add the color to the palette */
    palette[next_palette_index] = color;

    /* increment the index */
    next_palette_index++;

    /* return index of color just added */
    return next_palette_index - 1;
}

/* a colored square */
struct square {
    unsigned short x, y, len, wid;;
    unsigned char color;
};


/* put a pixel on the screen in mode 4 */
void put_pixel(volatile unsigned short* buffer, int row, int col, unsigned char color) {
    /* find the offset which is the regular offset divided by two */
    unsigned short offset = (row * WIDTH + col) >> 1;

    /* read the existing pixel which is there */
    unsigned short pixel = buffer[offset];

    /* if it's an odd column */
    if (col & 1) {
        /* put it in the left half of the short */
        buffer[offset] = (color << 8) | (pixel & 0x00ff);
    } else {
        /* it's even, put it in the left half */
        buffer[offset] = (pixel & 0xff00) | color;
    }
}

/* draw a square onto the screen */
void draw_square(volatile unsigned short* buffer, struct square* s) {
    unsigned short row, col;
    /* for each row of the square */
    for (row = s->y; row < (s->y + s->len); row++) {
        /* loop through each column of the square */
        for (col = s->x; col < (s->x + s->wid); col++) {
            /* set the screen location to this color */
            put_pixel(buffer, row, col, s->color);
        }
    }
}

/* clear the screen to black */
void update_screen(volatile unsigned short* buffer, unsigned short color, struct square* s) {
    unsigned short row, col;
    /* set each pixel black */
    for (row = s->y - 3; row < (s->y + s->len + 3); row++) {
        for (col = s->x - 3; col < (s->x + s->wid + 3); col++) {
            put_pixel(buffer, row, col, color);
        }
    }
}

/* this function takes a video buffer and returns to you the other one */
volatile unsigned short* flip_buffers(volatile unsigned short* buffer) {
    /* if the back buffer is up, return that */
    if(buffer == front_buffer) {
        /* clear back buffer bit and return back buffer pointer */
        *display_control &= ~SHOW_BACK;
        return back_buffer;
    }
    else {
        /* set back buffer bit and return front buffer */
        *display_control |= SHOW_BACK;
        return front_buffer;
    }
}

/* handle the buttons which are pressed down */
void handle_buttons(struct square* s) {
    /* move the square with the arrow keys */
    if (button_pressed(BUTTON_DOWN)) {
    	if (s->y < 135)
    	{
    		s->y += 1;
    	}
    }
    if (button_pressed(BUTTON_UP)) {
        if (s->y > 2){
    		s->y -= 1;
    	}
    }
    if (button_pressed(BUTTON_RIGHT)) {
        s->x += 0;
    }
    if (button_pressed(BUTTON_LEFT)) {
        s->x -= 0;
    }
};

void ballMovement(struct square* s,struct square* ritPad,struct square *lefPad){
	//Determines ball launch
	if(ballBool == 1){
		s->x = 115;
		s->y = 75;
		if(button_pressed(BUTTON_RIGHT)){
			ballBool = 0;
            dirBool = 2;
			bool = 1;
		}
		if(button_pressed(BUTTON_LEFT)){
			ballBool = 0;
			dirBool = 3; 
			bool = 0;
		}
	}
	// when ball is in motion
	else{
		// when ball is heading towards the right 
		if (bool == 1)
			{
				// when ball goes out of bounds, ball bool gets sent to first if statement
				if(s->x == 235)
				{
					startClear = 0;
					ballBool = 1;
				}
                else if(dirBool == 2){
                    if (s->y == 3) {
                        dirBool = 1;
                    }
                    else{
                        s->x += 1;
                        s->y -= 1;
                        if (s->y > 1 & s->y < 138) {
                            ritPad->y =s->y;
                        }
                        
                    }
                }
                else if(dirBool == 1){
                    if (s->y == 156) {
                        dirBool = 2;
                    }
                    else{
                        s->x += 1;
                        s->y += 1;
                        if (s->y<138) {
                            ritPad->y =s->y;
                        }
                        
                    }
                }
				if ((s->y==ritPad->y)  && s->x == 230 && dirBool == 1){
					dirBool = 4;
					bool=0;
				}
				if ((s->y<=(ritPad->y)+20  &&  (s->y >= ritPad->y)) && s->x == 230 && dirBool == 1){
					dirBool = 4;
					bool=0;
				}
				if (s->y==ritPad->y && s->x == 230 && dirBool == 2){
					dirBool = 3;
					bool=0;
				}
				if (((s->y<=(ritPad->y)+20) &&  (s->y>=ritPad->y)) && s->x == 230 && dirBool == 2){
					dirBool = 3;
					bool=0;
				}
			}

		// when the ball is heading towards the left
	    else if(bool == 0)
		{
			// when the ball goes out of bounds
			if(s->x == 4){
				startClear = 0;
				ballBool = 1;
			}
			else if(dirBool == 3){
				if(s->y == 2){
					dirBool = 4;

				}
				else{
					s->x -=1;
					s->y -=1;
					if (s->y > 1 & s->y < 138) {
                        ritPad->y =s->y;
                    }
				}

			}
			else if(dirBool == 4){
				if(s->y == 156){
					dirBool = 3;

				}
				else{
					//ball goes left down
					s->x -=1;
					s->y +=1;
					//Tracks ball
					if (s->y<138 ) {
                        ritPad->y =s->y;
                    }
				}
			}


			if ((s->y==lefPad->y)  && s->x == 10 && dirBool == 3){
					dirBool = 2;
					bool=1;
			}
			if (((s->y<=(lefPad->y)+20) && (s->y > lefPad->y)) && s->x == 10 && dirBool == 3){
					dirBool = 2;
					bool=1;
			}
			if ((s->y==lefPad->y) && s->x == 10 && dirBool == 4){
					dirBool = 1;
					bool=1;
			}
			if (((s->y <=(lefPad->y)+20) && (s->y > lefPad->y)) && s->x == 10 && dirBool == 4){
					dirBool = 1;
					bool=1;
			}
		}
	}
}

void paddleMovement(struct square* s){
	if (s->y < 135 && bool == 1)
	{
		s->y += 1;
		if(s->y == 135)
		{
			bool = 0;
		}
	}
    else if(s->y > 2 && bool == 0)
	{
		s->y -= 1;
		if(s->y == 2)
		{
			bool = 1;
		}
	}

}


/* clear the screen to black */
void clear_screen(volatile unsigned short* buffer, unsigned short color) {
    unsigned short row, col;
    /* set each pixel black */
    for (row = 0; row < HEIGHT; row++) {
        for (col = 0; col < WIDTH; col++) {
            put_pixel(buffer, row, col, color);
        }
    }
}

/* the main function */
int main( ) {
	while(1){
    	/* we set the mode to mode 4 with bg2 on */
		*display_control = MODE4 | BG2;

		/* make a green square */
		struct square lineBord = {115, 0, 160,2, add_color(160, 160, 160)};
		struct square s = {10, 65, 20,2, add_color(255, 255, 255)};
		struct square s2 = {230, 65, 20,2, add_color(255, 255, 255)};
		struct square ball = {115, 65, 2,2, add_color(255, 255, 255)};

		/* add black to the palette */
		unsigned char black = add_color(0, 102, 0);

		/* the buffer we start with */
		volatile unsigned short* buffer = front_buffer;

		/* clear whole screen first */
		clear_screen(front_buffer, black);
		clear_screen(back_buffer, black);
		startClear = 1;

		/* loop forever */
		while (startClear == 1) {
		    /* clear the screen - only the areas around the square! */
		    update_screen(buffer, black, &s);
		    update_screen(buffer, black, &s2);
		    update_screen(buffer, black, &ball);
		    update_screen(buffer, black, &lineBord);

		    /* draw the square */
		    draw_square(buffer, &s);
		    draw_square(buffer, &s2);
		    draw_square(buffer, &ball);
		    draw_square(buffer, &lineBord);

		    ballMovement(&ball,&s2,&s);
		    //paddleMovement(&s2);


		    /* handle button input */
		    handle_buttons(&s);
		    //handle_buttons(&s2);

		    /* wiat for vblank before switching buffers */
		    wait_vblank();

		    /* swap the buffers */
		    buffer = flip_buffers(buffer);
		}
	}
}

/* the game boy advance uses "interrupts" to handle certain situations
 * for now we will ignore these */
void interrupt_ignore( ) {
    /* do nothing */
}

/* this table specifies which interrupts we handle which way
 * for now, we ignore all of them */
typedef void (*intrp)( );
const intrp IntrTable[13] = {
    interrupt_ignore,   /* V Blank interrupt */
    interrupt_ignore,   /* H Blank interrupt */
    interrupt_ignore,   /* V Counter interrupt */
    interrupt_ignore,   /* Timer 0 interrupt */
    interrupt_ignore,   /* Timer 1 interrupt */
    interrupt_ignore,   /* Timer 2 interrupt */
    interrupt_ignore,   /* Timer 3 interrupt */
    interrupt_ignore,   /* Serial communication interrupt */
    interrupt_ignore,   /* DMA 0 interrupt */
    interrupt_ignore,   /* DMA 1 interrupt */
    interrupt_ignore,   /* DMA 2 interrupt */
    interrupt_ignore,   /* DMA 3 interrupt */
    interrupt_ignore,   /* Key interrupt */
};

