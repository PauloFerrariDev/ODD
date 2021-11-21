/*
 * ssd1306.c
 *
 *  Created on: May 23, 2019
 *      Author: samper
 */

#include "ssd1306.h"
#include <msp430.h>
#include <stdint.h>
#include "font_5x7.h"
#include "i2c.h"
#include "bitmaps.h"

/* ====================================================================
 * Horizontal Centering Number Array
 * ==================================================================== */
const unsigned char HcenterUL[] = {                                     // Horizontally center number with separators on screen
                               0,                                       // 0 digits, not used but included to size array correctly
                               61,                                      // 1 digit
                               58,                                      // 2 digits
                               55,                                      // 3 digits
                               49,                                      // 4 digits and 1 separator
                               46,                                      // 5 digits and 1 separator
                               43,                                      // 6 digits and 1 separator
                               37,                                      // 7 digits and 2 separators
                               34,                                      // 8 digits and 2 separators
                               31,                                      // 9 digits and 2 separators
                               25                                       // 10 digits and 3 separators
};

void ssd1306_init(void) {
// SSD1306 INIT SEQUENCE
    ssd1306_command(SSD1306_DISPLAYOFF);                                // 0xAE
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);                        // 0xD5
    ssd1306_command(0x90);                                              // the suggested ratio 0x80
//  ssd1306_command(0xf0);                                              // the suggested ratio 0x80

    ssd1306_command(SSD1306_SETMULTIPLEX);                              // 0xA8
    ssd1306_command(SSD1306_LCDHEIGHT - 1);

    ssd1306_command(SSD1306_SETDISPLAYOFFSET);                          // 0xD3
    ssd1306_command(0x0);                                               // no offset
    ssd1306_command(SSD1306_SETSTARTLINE | 0x0);                        // line #0
    ssd1306_command(SSD1306_CHARGEPUMP);                                // 0x8D
    ssd1306_command(0x14);                                              // generate high voltage from 3.3v line internally
    ssd1306_command(SSD1306_MEMORYMODE);                                // 0x20
    ssd1306_command(0x00);                                         //0x00     // 0x0 act like ks0108
    ssd1306_command(SSD1306_SEGREMAP | 0x01);//?? 00 espelha 01 não espelha
    ssd1306_command(SSD1306_COMSCANDEC);

    ssd1306_command(SSD1306_SETCOMPINS);                                // 0xDA
    ssd1306_command(0x12);
    ssd1306_command(SSD1306_SETCONTRAST);                               // 0x81
    ssd1306_command(0xCF);

    ssd1306_command(SSD1306_SETPRECHARGE);                              // 0xd9
    ssd1306_command(0xF1);
    ssd1306_command(SSD1306_SETVCOMDETECT);                             // 0xDB
    ssd1306_command(0x20); // estava 40 --- bit7 não escreve!
    ssd1306_command(SSD1306_DISPLAYALLON_RESUME);                       // 0xA4
    ssd1306_command(SSD1306_NORMALDISPLAY);                             // 0xA6

    ssd1306_command(SSD1306_DEACTIVATE_SCROLL);

    ssd1306_command(SSD1306_DISPLAYON);                                 //--turn on oled panel
}


void ssd1306_command(unsigned char command) {
    buffer[0] = 0x80; // control bit --
    buffer[1] = command;

    i2c_write(buffer, 2);
}


void ssd1306_clearDisplay(void) {
    ssd1306_setFrame(0, 7, 0, 127);

    uint8_t i, x;

    for (i = 64; i > 0; i--) {   // count down for loops when possible for ULP
        for(x = 16; x > 0; x--) {
            if (x == 1) {
                buffer[x-1] = 0x40;
            } else {
                buffer[x-1] = 0x0;
            }
        }

        i2c_write(buffer, 17);
    }
}


void ssd1306_clearFrame(uint8_t page_start, uint8_t page_end, uint8_t col_start, uint8_t col_end) {
    uint8_t i, j, totalPages, totalColumns;

    totalPages = page_end - page_start + 1;
    totalColumns = col_end - col_start + 1;

    ssd1306_setFrame(page_start, page_end, col_start, col_end);

    buffer[0] = 0x40;
    buffer[1] = 0x0;

    for(i = totalPages; i > 0; i--) {
        for(j = totalColumns; j > 0; j--) {
            i2c_write(buffer, 2);
        }
    }
}


void ssd1306_setFrame(uint8_t page_start, uint8_t page_end, uint8_t col_start, uint8_t col_end) {
    ssd1306_command(SSD1306_PAGEADDR);  // Command to set page range (limits: 0 to 7)
    ssd1306_command(page_start);        // Page start address
    ssd1306_command(page_end);          // Page end address
    ssd1306_command(SSD1306_COLUMNADDR);// Command to set column range (limits: 0 to 127)
    ssd1306_command(col_start);         // Column start address
    ssd1306_command(col_end);           // Column end address
}


void ssd1306_setPosition(uint8_t page_start, uint8_t col_start) {
    if (page_start > SSD1306_PAGE_END) page_start = SSD1306_PAGE_START;  // constrain page to upper limit

    if (col_start > SSD1306_COL_END) col_start = SSD1306_COL_START;      // constrain column to upper limit

    ssd1306_setFrame(page_start, SSD1306_PAGE_END, col_start, SSD1306_COL_END);
}


void ssd1306_drawFullScreen(unsigned char *bitmap) {
    ssd1306_setPosition(0, 0);

   uint16_t index=0;
   uint8_t row, col, count;

   for(row=64; row>0; row--) {
       count=1;

       for(col=16; col>0; col--) {
           buffer[count++]=bitmap[index++];
       }

       buffer[0]=0x40;

       i2c_write(buffer, 17);
   }
}


void ssd1306_drawNumber(uint8_t number) {
   uint16_t index=0;
   uint8_t row, col, count;

   for(row=3; row>0; row--) {
       count=1;

       for(col=18; col>0; col--) {
           buffer[count++]=bitmap_numbers[number][index++];
       }

       buffer[0]=0x40;

       i2c_write(buffer, 19);
   }
}


void ssd1306_printText(uint8_t page_start, uint8_t col_start, char *ptString) {
    uint8_t page=page_start, col=col_start, i=0;

    if ((col + 6) > SSD1306_COL_END) {            // char will run off screen
        col = SSD1306_COL_START;                  // set column to 0
        page++;                                   // jump to next page
    }

    if(page > SSD1306_PAGE_END) page = SSD1306_PAGE_START;

    ssd1306_setPosition(page, col);               // send position change to oled

    while (*ptString != '\0') {
        if ((col + 6) > SSD1306_COL_END) {        // char will run off screen
            col = SSD1306_COL_START;              // set column to 0

            page++;                               // jump to next page

            if(page > SSD1306_PAGE_END) page = SSD1306_PAGE_START;

            ssd1306_setPosition(page, col);       // send position change to oled
        }

        for(i=0; i<5; i++) {
            buffer[i+1] = font_5x7[*ptString - ' '][i];
        }

        buffer[0] = 0x40;                        // command to write data
        buffer[6] = 0x0;                         // white space between characters

        i2c_write(buffer, 7);

        ptString++;
        col+=6;
    }
}


//void ssd1306_printTextBlock(uint8_t x, uint8_t y, char *ptString) {
//    char word[12];
//    uint8_t i;
//    uint8_t endX = x;
//    while (*ptString != '\0'){
//        i = 0;
//        while ((*ptString != ' ') && (*ptString != '\0')) {
//            word[i] = *ptString;
//            ptString++;
//            i++;
//            endX += 6;
//        }
//
//        word[i++] = '\0';
//
//        if (endX >= 127) {
//            x = 0;
//            y++;
//            ssd1306_printText(x, y, word);
//            endX = (i * 6);
//            x = endX;
//        } else {
//            ssd1306_printText(x, y, word);
//            endX += 6;
//            x = endX;
//        }
//        ptString++;
//    }
//}


void ssd1306_printUI32( uint8_t x, uint8_t y, uint32_t val, uint8_t Hcenter ) {
    char text[14];

    ultoa(val, text);
    if (Hcenter) {
        ssd1306_printText(HcenterUL[digits(val)], y, text);
    } else {
        ssd1306_printText(x, y, text);
    }
}


uint8_t digits(uint32_t n) {
    if (n < 10) {
        return 1;
    } else if (n < 100) {
        return 2;
    } else if (n < 1000) {
        return 3;
    } else if (n < 10000) {
        return 4;
    } else if (n < 100000) {
        return 5;
    } else if (n < 1000000) {
        return 6;
    } else if (n < 10000000) {
        return 7;
    } else if (n < 100000000) {
        return 8;
    } else if (n < 1000000000) {
        return 9;
    } else {
        return 10;
    }
}


void ultoa(uint32_t val, char *string) {
    uint8_t i = 0;
    uint8_t j = 0;
                                                                        // use do loop to convert val to string
    do {
        if (j==3) {                                                     // we have reached a separator position
            string[i++] = ',';                                          // add a separator to the number string
            j=0;                                                        // reset separator indexer thingy
        }
            string[i++] = val%10 + '0';                                 // add the ith digit to the number string
            j++;                                                        // increment counter to keep track of separator placement
    } while ((val/=10) > 0);

    string[i++] = '\0';                                                 // add termination to string
    reverse(string);                                                    // string was built in reverse, fix that
}


void reverse(char *s) {
    uint8_t i, j;
    uint8_t c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
