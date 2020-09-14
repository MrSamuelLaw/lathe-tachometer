
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
constexpr uint8_t LIST_SIZE{10};  // number of nodes in LL
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
    Node node10;
    Head = &node10;

    Node node9{0, Head};
    Node node8{0, &node9};
    Node node7{0, &node8};
    Node node6{0, &node7};
    Node node5{0, &node6};
    Node node4{0, &node5};
    Node node3{0, &node4};
    Node node2{0, &node3};
    Node node1{0, &node2};
    node10 = (Node){0, &node1};

    Serial.begin(9600);
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
        Serial.println(standard_deviation());
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

// #include <Arduino.h>
// #include <LiquidCrystal.h>

// //    _____                _              _
// //   / ____|              | |            | |
// //  | |     ___  _ __  ___| |_ __ _ _ __ | |_ ___
// //  | |    / _ \| '_ \/ __| __/ _` | '_ \| __/ __|
// //  | |___| (_) | | | \__ \ || (_| | | | | |_\__ \
// //   \_____\___/|_| |_|___/\__\__,_|_| |_|\__|___/
// constexpr int Vin{8};
// constexpr int rpm_low_lim{40};
// constexpr unsigned long timeout = 3.5e6;
// constexpr uint8_t rows{2};
// constexpr uint8_t cols{16};
// constexpr uint8_t LIST_SIZE{10};
// constexpr uint16_t STD_LIMIT{60};

// //  __      __        _       _     _
// //  \ \    / /       (_)     | |   | |
// //   \ \  / /_ _ _ __ _  __ _| |__ | | ___  ___
// //    \ \/ / _` | '__| |/ _` | '_ \| |/ _ \/ __|
// //     \  / (_| | |  | | (_| | |_) | |  __/\__ \
// //      \/ \__,_|_|  |_|\__,_|_.__/|_|\___||___/
// unsigned long t_high{};
// unsigned long t_low{};
// unsigned long period{};
// int rpm{};
// int Sum{0};

// //    ____  _     _           _
// //   / __ \| |   (_)         | |
// //  | |  | | |__  _  ___  ___| |_ ___
// //  | |  | | '_ \| |/ _ \/ __| __/ __|
// //  | |__| | |_) | |  __/ (__| |_\__ \
// //   \____/|_.__/| |\___|\___|\__|___/
// //              _/ |
// //             |__/
// LiquidCrystal lcd(6, 7, 2, 3, 4, 5);
// // NOTE* This will not work except when literal numbers are used to instantate
// // the object. For example. Assigning 6 to a variable will cause the lcd to
// // not work.
// typedef struct Node{int value; struct Node* next;} Node;
// // give it some head safety
// Node* Head = NULL;

// //   ______ _    _ _   _      _   _
// //  |  ____| |  | | \ | |    | | (_)
// //  | |__  | |  | |  \| | ___| |_ _  ___  _ __  ___
// //  |  __| | |  | | . ` |/ __| __| |/ _ \| '_ \/ __|
// //  | |    | |__| | |\  | (__| |_| | (_) | | | \__ \
// //  |_|     \____/|_| \_|\___|\__|_|\___/|_| |_|___/
// void clear_row(uint8_t row){
//   // clear each cell
//   for (uint8_t i{0}; i < cols; ++i){
//     lcd.setCursor(i, row);
//     lcd.print(" ");
//   }
//   // return cursor zero
//   lcd.setCursor(0, 0);
// }

// void print_rpm(int &rpm){
//   /* outputs rpm to lcd based on condition
//   set rpm -1 if less than 40,
//   set rpm to 0 if equal to zero*/

//   switch (rpm)
//   {
//     case 0: {
//       clear_row(0);
//       lcd.print("RPM: 0");
//       break;
//     }
//     case -1: {
//       clear_row(0);
//       lcd.print("RPM: < 40");
//       break;
//     }
//     default: {
//       clear_row(0);
//       lcd.print("RPM: ");
//       lcd.print(rpm);
//       break;
//     }
//   }
// }

// void clear_warning(){
//   // clears the second row of the lcd
//   clear_row(1);
// }

// void print_warning(char message[]){
//   // clear old text
//   clear_row(1);
//   // set the curser to the second row
//   lcd.setCursor(0, 1);
//   // send to lcd
//   lcd.print(message);
// }

// bool is_bidirectional(){
//   // create the storage variables
//   int raw_sums{0};
//   int sum_of_absolutes{0};

//   // determine if uni-directional
//   for (uint8_t x{0}; x < LIST_SIZE; x++, Head = Head->next){
//     register int delta = Head->next->value - Head->value;
//     sum_of_absolutes += abs(delta);
//     raw_sums += delta;
//   }

//   // return the head to its origianl position
//   Head = Head->next;

//   // return true if they are not equal to eachother.
//   return sum_of_absolutes != abs(raw_sums);
// }

// int standard_deviation(){
//   int mean = Sum/LIST_SIZE;
//   int variance{0};
//     for(uint8_t x{0}; x < LIST_SIZE; x++, Head = Head->next){
//       variance += pow(Head->value - mean, 2);
//     }
//   return (int)sqrt(variance);
// }

// void update_sum(int old_value, int new_value){
//   // updates the sum
//   Sum -= old_value;
//   Sum += new_value;
// }

// void update_linked_list(int new_value){
//   /* updates the value of the current linked
//   list index and then points to the next index*/
//   Head->value = new_value;
//   Head = Head->next;
// }

// void calc_rpm(unsigned long &t_low, unsigned long &t_high){
//   /* calcualtes the rpm using the low time and the high time,
//   then automatically sends the text to the lcd*/

//   // calculate the total time period
//   period = t_low + t_high;

//   // if the output is valid
//   if (t_high && t_low && (period >= rpm_low_lim)) {
//     rpm = 60e6/period;
//     // update the sum
//     update_sum(Head->value, rpm);

//     // update the linked list
//     update_linked_list(rpm);
//   }

//   // if on only one of the outputs bounced or rpm is below limit
//   else if (t_high || t_low || (rpm < rpm_low_lim && rpm > 0)) {
//     rpm = -1;
//   }

//   // rpm is approximately zero
//   else {
//     rpm = 0;
//   }

//   // if high std deviation, get angry!
//   if(STD_LIMIT < standard_deviation() && is_bidirectional()){
//     print_warning("shit's broke!");
//   }
//   else {
//     clear_warning();
//   }

//   // send output to lcd
//   print_rpm(rpm);
// }

// //    _____      _
// //   / ____|    | |
// //  | (___   ___| |_ _   _ _ __
// //   \___ \ / _ \ __| | | | '_ \
// //   ____) |  __/ |_| |_| | |_) |
// //  |_____/ \___|\__|\__,_| .__/
// //                        | |
// //                        |_|
// void setup() {
//   // define voltage sensor pin
//   pinMode(Vin, INPUT);

//   // start the lcd and clear the screen
//   lcd.begin(16, 2);
//   lcd.clear();

//   // create circular linked list
//   Node node10;
//   Head = &node10;

//   Node node9{0, &node10};
//   Node node8{0, &node9};
//   Node node7{0, &node8};
//   Node node6{0, &node7};
//   Node node5{0, &node6};
//   Node node4{0, &node5};
//   Node node3{0, &node4};
//   Node node2{0, &node3};
//   Node node1{0, &node2};
//   node10 = (Node){0, &node1};
// }

// //   _
// //  | |
// //  | |     ___   ___  _ __
// //  | |    / _ \ / _ \| '_ \
// //  | |___| (_) | (_) | |_) |
// //  |______\___/ \___/| .__/
// //                    | |
// //                    |_|
// void loop() {
//   /* Note, the limit for the timeout varies by board.
//   On the uno rpm's under 40 probably won't be detected,
//   thus the timeout variable was chosen specifically with
//   that in mind. If you want to detect lower rpm's you
//   must increase the value of the timeout variable.
//   */

//   // calculated the period
//   t_high = pulseIn(Vin, HIGH, timeout);
//   t_low = pulseIn(Vin, LOW, timeout);
//   calc_rpm(t_low, t_high);
//   delay(50);
// }