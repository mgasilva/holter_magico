#define NCELLS 512

unsigned int signal[NCELLS];
unsigned int counter = 0;
bool bufferFilled = false;
bool ledState = false;

float sum = 0.0f;
float sumSq = 0.0f;
float average = 0.0f;
float variance = 0.0f;
float sd = 0.0f;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0, INPUT);
  Serial.begin(9600);
}

void loop() {
  unsigned int x = analogRead(A0);
  unsigned int oldSample = signal[counter];

  // Remove old sample and add new sample
  sum += (float)x - (float)oldSample;
  sumSq += (float)x * (float)x - (float)oldSample * (float)oldSample;

  // Store new sample in buffer
  signal[counter] = x;
  counter = (counter + 1) % NCELLS;

  if (counter == 0) {
    bufferFilled = true;
  }

  // Only compute meaningful stats after buffer is full
  if (bufferFilled) {
    average = sum / NCELLS;
    variance = sumSq / NCELLS - average * average;

    if (variance < 0) variance = 0;   // protect against rounding
    sd = sqrt(variance);

    // Simple hysteresis
    if (!ledState && x > average + sd) {
      ledState = true;
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if (ledState && x < average + 0.5f * sd) {
      ledState = false;
      digitalWrite(LED_BUILTIN, LOW);
    }

    Serial.println(x);
  }
}