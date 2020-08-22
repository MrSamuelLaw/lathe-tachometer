
/***********************************************************************************************************
*
*	created by: MPZinke & MrSamuelLaw
*	on 2020.08.21
*
*	DESCRIPTION: Uses hallex sensor to read magnet on flywheel. Data is kept in a circular
*		linked list to determine speed, deviation & directionality.  RPM is displayed on line 1
*		of 16x2 of LCD screen.  A warning is displayed on line 2 of LCD if bidirectional 
*		movement is detected.
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
constexpr int VIN{8};  // pin to read sensor from
constexpr uint8_t PERIOD_LOWER_LIMIT{40};  // minimum time between high-low to do calculations
constexpr unsigned long TIMEOUT = 3.5e6;  // time input pin listens before timeouts
constexpr uint8_t LIST_SIZE{10};  // number of nodes in LL
constexpr uint8_t STD_LIMIT{60};  // highest deviation (acceleration) before directionality should be checked
constexpr uint8_t ARDUINO_DELAY{50};  // pause between measurements


// —————————————————————— GLOBAL ———————————————————————

Node* Head = NULL;  // holds current position in LL; NULL prevent possible run aways
int Sum{0};  // sliding window sum of nodes
LiquidCrystal Lcd(6, 7, 2, 3, 4, 5);  // actual number literals must be used (feel free to try for yourself)


// —————————————————————— UTILITY ——————————————————————

// determines if deviations are bidirectional.
// finds all the differences.
// compares the absolute value of summation of differences with summation of absolutes values of differences.
// returns whether two values are not equal (equal mean unidirectional.)
bool is_bidirectional()
{
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


// calculate the standard deviation of the nodes.
// iterates over the LL & summing the variances.
// returns int of standard deviation
int standard_deviation()
{
	int mean = Sum / LIST_SIZE;
	int variance{0};
	for(uint8_t x{0}; x < LIST_SIZE; x++, Head = Head->next) variance += pow(Head->value - mean, 2);
	return (int)sqrt(variance);
}


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
}



// Note, the limit for the timeout varies by board. On the uno rpm's under 40 probably won't be detected,
// thus the timeout variable was chosen specifically with that in mind. If you want to detect lower rpm's you
// must increase the value of the timeout variable.
void loop()
{
	// time between magnet passes
	unsigned long period = pulseIn(VIN, LOW, TIMEOUT) + pulseIn(VIN, HIGH, TIMEOUT);

	uint16_t rpm = 0;  // default RPM to zero

	// write RPM to LCD
	Lcd.setCursor(0, 0);
	// if on only one of the outputs bounced or rpm is below limit
	// trailing whitespace to override possible chars (MAX considered len:11 chars)
	if(period && period < PERIOD_LOWER_LIMIT) Lcd.print("RPM: < 40  ");
	else
	{
		if(period) rpm = 6e7 / period;  // prevent 0 denominator division

		// print to LCD
		Lcd.print("RPM: ");
		Lcd.print(rpm);
		Lcd.print(TRAILING_WHITESPACE);
	
		Sum -= Head->value + rpm;  // update the sum

		// update the linked list
		Head->value = rpm;
		Head = Head->next;
	}

	// write/clear possible error of dangerous machine torque circumstances
	Lcd.setCursor(0, 1);
	if(STD_LIMIT < standard_deviation() && is_bidirectional()) Lcd.print("shit's broke!");
	else Lcd.print("                ");  // clear warning

	delay(ARDUINO_DELAY);  // slow down arduino to read magnet better
}
