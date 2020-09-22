
/***********************************************************************************************************
*
*	created by: MPZinke & MrSamuelLaw
*	on 2020.08.21
*
*	DESCRIPTION: Uses hall effect sensor to read magnet on flywheel. Data is kept in a circular
*		linked list to determine speed, deviation & directionality.  RPM is displayed on line 1
*		of 16x2 of LCD screen.  A warning is displayed on line 2 of LCD if bidirectional
*		movement is detected along with a high standard deviation
*	BUGS:
*	FUTURE:
*
***********************************************************************************************************/

#include <Arduino.h>
#include <LiquidCrystal.h>


// ——————————————————————— TYPES ———————————————————————

typedef struct Node{int value; struct Node* next;} Node;  // node of Linked-List


// ————————————————————— CONSTANTS ——————————————————————

#define TRAILING_WHITESPACE "     "  // pace to override possible chars (MAX considered len: 11 chars)
constexpr int Vin{8};  // pin to read sensor from
constexpr double PERIOD_UPPER_LIMIT{1.5e6}; // maximum time between high-low (other wise motor too slow)
constexpr double TIMEOUT{3.5e6};  // time input pin listens before timeouts
constexpr uint8_t LIST_SIZE{5};  // number of nodes in LL
constexpr uint8_t STD_LIMIT{60};  // highest deviation (acceleration) before directionality should be checked
constexpr uint8_t ARDUINO_DELAY{70};  // pause between measurements
constexpr uint8_t COLUMNS{16};  // columns on the LCD
constexpr uint8_t ROWS{2};  // number of rows on the LCD

// —————————————————————— GLOBAL ———————————————————————

Node* Head = NULL;  // holds current position in LL; NULL prevent possible run aways
int Sum{0};  // sliding window sum of nodes
LiquidCrystal Lcd(6, 7, 2, 3, 4, 5);  // actual number literals must be used (feel free to try for yourself)
           // Lcd(rs, enable, lcd_d4, lcd_d5, lcd_d6, lcd_d7)
unsigned long high_time{0};
unsigned long low_time{0};
unsigned long period{0};
uint16_t rpm{0};


// —————————————————————— UTILITY ——————————————————————

bool is_bidirectional()
{
  // determines if deviations are bidirectional.
  // finds all the differences.
  // compares the absolute value of summation of differences with summation of absolutes values of differences.
  // returns whether two values are not equal (equal mean unidirectional.)

    int raw_sums{0};
    int sum_of_absolutes{0};

    for (uint8_t x{0}; x < LIST_SIZE; x++, Head = Head->next)
    {
        register int delta = Head->next->value - Head->value;
        sum_of_absolutes += abs(delta);
        raw_sums += delta;
    }

    Head = Head->next;  // return the head to its origianl position

    return sum_of_absolutes != abs(raw_sums);  // return true if they are not equal to eachother.
}


int standard_deviation()
{
  // calculate the standard deviation of the nodes.
  // iterates over the LL & summing the variances.
  // returns int of standard deviation
    int mean{Sum / LIST_SIZE};
    int variance{0};
    for(uint8_t x{0}; x < LIST_SIZE; x++){
      Head = Head->next;
      variance += (Head->value - mean) * (Head->value - mean);
    }
    return sqrt(variance);
}


void clear_row(uint8_t row){
  // clear each cell
  for (uint8_t i{0}; i < COLUMNS; ++i){
    Lcd.setCursor(i, row);
    Lcd.print(" ");
  }
  // return cursor zero
  Lcd.setCursor(0, 0);
}


void clear_warning(){
  clear_row(1);
}


void clear_rpm(){
  clear_row(0);
}


// —————————————————————— SETUP ——————————————————————

void setup()
{
    pinMode(Vin, INPUT);  // define voltage sensor pin

    // start the LCD and clear the screen
    Lcd.begin(16, 2);
    Lcd.clear();

    // create circular linked list
    Node node5;
    Head = &node5;

    Node node4{0, &node5};
    Node node3{0, &node4};
    Node node2{0, &node3};
    Node node1{0, &node2};
    node5 = (Node){0, &node1};
}


// —————————————————————— LOOP ——————————————————————

void loop()
{
  // Note, the limit for the timeout varies by board. On the uno rpm's under 40 probably won't be detected,
  // thus the timeout variable was chosen specifically with that in mind. If you want to detect lower rpm's you
  // must increase the value of the timeout variable.

    // time between magnet passes
    high_time = pulseIn(Vin, HIGH, TIMEOUT);
    low_time = pulseIn(Vin, LOW, TIMEOUT);
    // period = pulseIn(Vin, LOW, TIMEOUT) + pulseIn(Vin, HIGH, TIMEOUT);

    // uint16_t rpm = 0;  // default RPM to zero

    // both values returned non zero
    if (high_time && low_time) {
        period = high_time + low_time;
        rpm = 6e7/period;
    }
    // one value returned non zero
    else if (!high_time != !low_time) {
        period = (high_time + low_time) + TIMEOUT;
        rpm = 6e7/period;
    }
    // both values returned zero
    else {
        rpm = 0;
    }

    // update the rpm value from Lcd
    clear_rpm();
    Lcd.print("RPM: ");
    Lcd.print(rpm);

    // update the circular linked list
    Sum += rpm - Head->value;
    Head->value = rpm;
    Head = Head->next;

    // display warning if necessary
    if ((standard_deviation() > STD_LIMIT) && is_bidirectional()) {
        clear_warning();
        Lcd.setCursor(0, 1);
        Lcd.print("Shit's broke!");
    }
    else {
        clear_warning();
    }

    // pause to ensure the message is written to the Lcd
    delay(ARDUINO_DELAY);
}
