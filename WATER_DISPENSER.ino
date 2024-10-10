// Define pins for the JSN-SR04T sensor and relay
const int trigPin = 33;
const int echoPin = 32;
const int relayPin = 5;  // Relay control pin

// Variables to store duration, distance, and state
long duration;
int distance;
bool relayState = false;  // To track the current state of the relay (ON/OFF)
bool delayPassed = false; // To check if 1 second delay for turning ON has passed
bool resetCondition = false; // To track if the distance exceeded the OFF threshold (3 cm)
unsigned long relayTurnOnTime = 0;  // To store when the relay was turned on
unsigned long lastDistanceCheckTime = 0; // To store the last time distance was checked
const unsigned long turnOnDelay = 750;  // Delay before turning on the relay (1 second)
const unsigned long relayOnDuration = 6000;  // Relay ON duration (3 seconds)
const int distanceThresholdOn = 3;  // Distance threshold for turning ON the relay
const int distanceThresholdOff = 6;  // Distance threshold for immediate turn OFF

void setup() {
  // Start serial communication for debugging
  Serial.begin(9600);

  // Set up the trigger, echo, and relay pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relayPin, OUTPUT);

  // Ensure the relay starts in the OFF state
  digitalWrite(relayPin, HIGH);
}

void loop() {
  // Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a 10µs pulse to trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the echo pin pulse duration
  duration = pulseIn(echoPin, HIGH);

  // Calculate distance
  distance = duration * 0.0343 / 2;

  // Print the distance to the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(100);
  
  // Check if the distance has gone above the threshold to reset the chance to turn ON
  if (distance > distanceThresholdOff && !relayState) {
    resetCondition = true;  // The distance has exceeded the off threshold, ready for another cycle
    delayPassed = false;    // Reset the delayPassed flag
  }

  // Check if the distance is less than or equal to the threshold to turn ON the relay
  if (resetCondition && !relayState && distance <= distanceThresholdOn) {
    // Start a 1-second delay before turning on the relay
    if (!delayPassed) {
      // Start timing when the distance first drops below the threshold
      if (lastDistanceCheckTime == 0) {
        lastDistanceCheckTime = millis();
      }

      // Check if the delay time has passed
      if (millis() - lastDistanceCheckTime >= turnOnDelay) {
        delayPassed = true; // Mark that the delay has passed
        relayTurnOnTime = millis();  // Store the time the relay was turned on
        relayState = true;  // Update relay state to ON
        digitalWrite(relayPin, LOW);  // Turn on the relay
        Serial.println("Relay ON");
        resetCondition = false;  // Reset the condition to ensure relay doesn't turn on immediately again
        lastDistanceCheckTime = 0;  // Reset the timing for next cycle
      }
    }
  }

  // If the relay is ON, check if it should turn off
  if (relayState) {
    if (millis() - relayTurnOnTime >= relayOnDuration || (distance > distanceThresholdOff && distance<20)) {
      relayState = false;  // Update relay state to OFF
      delayPassed = false;  // Reset the delayPassed flag for the next ON cycle
      digitalWrite(relayPin, HIGH);  // Turn off the relay
      Serial.println("Relay OFF");
    }
  }
}
