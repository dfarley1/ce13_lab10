//Daniel Farley, dfarley@ucsc.edu

// **** Include libraries here ****
// Standard libraries
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries - Hardware
#include "HardwareDefs.h"
#include "Adc.h"
#include "Ascii.h"
#include "Buttons.h"
#include "Common.h"
#include "Leds.h"
#include "Oled.h"
#include "OledDriver.h"

// User libraries - Game
#include "Game.h"
#include "Player.h"

// **** Set any macros or preprocessor directives here ****

// **** Declare any data types here ****
typedef struct {
    bool event;
    uint16_t value;
}AdcResult;

// **** Define any global or external variables here ****
static ButtonEventFlags buttons = BUTTON_EVENT_NONE;
static AdcResult ADC;

static char Oledstr[OLED_CHARS_PER_LINE + GAME_MAX_ROOM_DESC_LENGTH + 2];


// **** Declare any function prototypes here ****
void UpdateOledString(void);

// Configuration Bit settings
// SYSCLK = 80 MHz (8MHz Crystal/ FPLLIDIV * FPLLMUL / FPLLODIV)
// PBCLK = 20 MHz
// Primary Osc w/PLL (XT+,HS+,EC+PLL)
#pragma config FPLLIDIV   = DIV_2     // Set the PLL input divider to 2
#pragma config FPLLMUL    = MUL_20    // Set the PLL multiplier to 20
#pragma config FPLLODIV   = DIV_1     // Don't modify the PLL output.
#pragma config FNOSC      = PRIPLL    // Set the primary oscillator to internal RC w/ PLL
#pragma config FSOSCEN    = OFF       // Disable the secondary oscillator
#pragma config IESO       = OFF       // Internal/External Switch O
#pragma config POSCMOD    = XT        // Primary Oscillator Configuration
#pragma config OSCIOFNC   = OFF       // Disable clock signal output
#pragma config FPBDIV     = DIV_4     // Set the peripheral clock to 1/4 system clock
#pragma config FCKSM      = CSECMD    // Clock Switching and Monitor Selection
#pragma config WDTPS      = PS1       // Specify the watchdog timer interval (unused)
#pragma config FWDTEN     = OFF       // Disable the watchdog timer
#pragma config ICESEL     = ICS_PGx2  // Allow for debugging with the Uno32
#pragma config PWP        = OFF       // Keep the program flash writeable
#pragma config BWP        = OFF       // Keep the boot flash writeable
#pragma config CP         = OFF       // Disable code protect

int main()
{
    // Configure the device for maximum performance but do not change the PBDIV
    // Given the options, this function will change the flash wait states, RAM
    // wait state and enable prefetch cache but will not change the PBDIV.
    // The PBDIV value is already set via the pragma FPBDIV option above..
    SYSTEMConfig(F_SYS, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    // Auto-configure the PIC32 for optimum performance at the specified operating frequency.
    SYSTEMConfigPerformance(F_SYS);

    // osc source, PLL multipler value, PLL postscaler , RC divisor
    OSCConfig(OSC_POSC_PLL, OSC_PLL_MULT_20, OSC_PLL_POST_1, OSC_FRC_POST_1);

    // Configure the PB bus to run at 1/4th the CPU frequency, so 20MHz.
    OSCSetPBDIV(OSC_PB_DIV_4);

    // Enable multi-vector interrupts
    INTEnableSystemMultiVectoredInt();
    INTEnableInterrupts();

    // Configure Timer 2 using PBCLK as input. We configure it using a 1:16 prescalar, so each timer
    // tick is actually at F_PB / 16 Hz, so setting PR2 to F_PB / 16 / 100 yields a .01s timer.
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_16, F_PB / 16 / 100);

    // Set up the timer interrupt with a medium priority of 4.
    INTClearFlag(INT_T2);
    INTSetVectorPriority(INT_TIMER_2_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_TIMER_2_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
    INTEnable(INT_T2, INT_ENABLED);


    UARTConfigure(UART_USED, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetFifoMode(UART_USED, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART_USED, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART_USED, F_PB, UART_BAUD_RATE);
    UARTEnable(UART_USED, UART_ENABLE | UART_TX | UART_RX);

    // And configure printf/scanf to use UART_USED if it's different from the default of UART2.
	if (UART_USED == UART1) {
		__XC_UART = 1;
	}

    /****************************** Your custom code goes below here ******************************/
    ButtonsInit();
    AdcInit();
    OledInit();
    LEDS_INIT();
    if (GameInit() != SUCCESS) {
        FATAL_ERROR();
    }

    bool roomChanged = true; //Signifies a room change and an Oled update is needed
    int screenShowing = 0; //Which screen of text is being shown

    UpdateOledString();

    while(1) {
        //check for buttons, move if pressed: GameGo*, update Oledstr, update LEDs
        if (buttons & BUTTON_EVENT_4UP) {
            roomChanged = true;
            buttons = BUTTON_EVENT_NONE;
            if (GameGoNorth() != SUCCESS) FATAL_ERROR();
            UpdateOledString();
        }

        if (buttons & BUTTON_EVENT_3UP) {
            roomChanged = true;
            buttons = BUTTON_EVENT_NONE;
            if (GameGoEast() != SUCCESS) FATAL_ERROR();
            UpdateOledString();
        }

        if (buttons & BUTTON_EVENT_2UP) {
            roomChanged = true;
            buttons = BUTTON_EVENT_NONE;
            if (GameGoSouth() != SUCCESS) FATAL_ERROR();
            UpdateOledString();
        }

        if (buttons & BUTTON_EVENT_1UP) {
            roomChanged = true;
            buttons = BUTTON_EVENT_NONE;
            if (GameGoWest() != SUCCESS) FATAL_ERROR();
            UpdateOledString();
        }

        //check for Adc change and change Oled if needed
        if (ADC.event || roomChanged) {
            
            //Calculate number of lines in Oledstr
            int numLines = 0;
            char *lineCounter = strchr(Oledstr, '\n');
            while (lineCounter != '\0') {
                numLines++;
                lineCounter = strchr(lineCounter + 1, '\n');
            }


            int numScreens = (numLines / OLED_NUM_LINES) + 1;
            if (((numScreens - 1) * OLED_NUM_LINES) == numLines) numScreens--;

            //Calculate which screen should be shown based on Adc value
            double topPart = ((double)ADC.value / ((double)ADC_MAX_VALUE + 1.0));
            double bottomPart = (1.0 / (double)numScreens);
            int screenToShow = (int)(floor(topPart / bottomPart) + 1.0);
            

            //if the screen needs to be updated
            if ((screenToShow != screenShowing) || roomChanged) {
                //           [((21 visible chars + '\n') *   4 lines  ) + '\0']
                char Printstr[((OLED_CHARS_PER_LINE + 1) * OLED_NUM_LINES) + 1];

                //Find start point
                char *iter = Oledstr;
                int i;
                for (i = 0; i < OLED_NUM_LINES * (screenToShow - 1); i++) {
                    iter = strstr(iter, "\n");
                    iter++;
                }

                //Find end point
                char *cpyUntil = iter;
                for (i = 0; i < OLED_NUM_LINES; i++) {
                    if (strstr(cpyUntil, "\n") == NULL) break;
                    cpyUntil = strstr(cpyUntil, "\n");
                    cpyUntil++;
                }

                //Copy the string and print
                strncpy(Printstr, iter, (cpyUntil - iter + 1));
                Printstr[(cpyUntil - iter)] = '\0';
                //printf("Screen Update: numLines=%d, numScreens=%d\n{%s}\n\n\n",
                //        numLines, numScreens, Printstr);
                OledClear(OLED_COLOR_BLACK);
                OledDrawString(Printstr);
                OledUpdate();
                LEDS_SET(GameGetCurrentRoomExits());
                screenShowing = screenToShow;
            }

            roomChanged = false;
            ADC.event = false;
        }
    }


    /**********************************************************************************************/
    while (1);
}

/**
 * Timer2 interrupt. Checks for button events.
 */
void __ISR(_TIMER_2_VECTOR, ipl4auto) TimerInterrupt100Hz(void)
{
    // Clear the interrupt flag.
    IFS0CLR = 1 << 8;

    buttons = ButtonsCheckEvents();
    uint16_t val = ((ADC1BUF0 + ADC1BUF1 + ADC1BUF2 + ADC1BUF3 +
                     ADC1BUF4 + ADC1BUF5 + ADC1BUF6 + ADC1BUF7) / 8);
    //Getting jitters when using !=, might as well have some more buffer, precision isn't important
    if ((val < ADC.value - 3) || (val > ADC.value + 3)) {
        ADC.value = val;
        ADC.event = true;
    }
}

void UpdateOledString(void)
{
    GameGetCurrentRoomTitle(Oledstr);
    Oledstr[strlen(Oledstr) + 1] = '\0';
    Oledstr[strlen(Oledstr)] = '\n';
    GameGetCurrentRoomDescription((Oledstr + strlen(Oledstr)));

    //Need an extra plug for the while-loop's check when all done
    // (the final iteration replaces the plug with a \n)
    Oledstr[strlen(Oledstr) + 1] = '\0';

    char *Olediter = strstr(Oledstr, "\n"), *space;
    while (*Olediter != '\0') {
        int i;
        for (i = 0; i <= OLED_CHARS_PER_LINE; i++) {
            if (*Olediter == '\0') {
                space = Olediter;
                break;
            }
            if (*Olediter == ' ') space = Olediter;
            Olediter++;
        }
        if (space != '\0') {
            *space = '\n';
        } else {
            break;
        }
        Olediter = space + 1;
    }
    //printf("Moved:\n{%s}\n\n\n", Oledstr);
}
