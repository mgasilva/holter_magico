#define NCELLS 512
#define WINDOW_MS 300000

unsigned int signal[NCELLS];
unsigned int counter = 0;
bool ledState = false;

float sum = 0, sumSq = 0;
float average = 0, variance = 0, sd = 0;

unsigned long lastBeatTime = 0;
unsigned long startTime = 0;

unsigned int rrCount = 0;
double rrSum = 0;
double rrSumSq = 0;

const unsigned int MIN_RR = 300;
const unsigned int MAX_RR_MS = 2000;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0, INPUT);
  Serial.begin(9600);

  unsigned int initial = analogRead(A0);

  for (int i = 0; i < NCELLS; i++) {
    signal[i] = initial;
    sum += initial;
    sumSq += (float)initial * initial;
  }

  startTime = millis();
}

float calculateSDRR() {
  if (rrCount < 2) return -1;

  double mean = rrSum / rrCount;
  double varianceRR = (rrSumSq - rrCount * mean * mean) / (rrCount - 1);

  if (varianceRR < 0) varianceRR = 0;

  return sqrt(varianceRR);
}

void loop() {
  unsigned int x = analogRead(A0);
  unsigned int oldSample = signal[counter];

  sum += (float)x - (float)oldSample;
  sumSq += (float)x * x - (float)oldSample * oldSample;

  signal[counter] = x;
  counter = (counter + 1) % NCELLS;

  average = sum / NCELLS;
  variance = sumSq / NCELLS - average * average;
  if (variance < 0) variance = 0;
  sd = sqrt(variance);

  unsigned long now = millis();

  if (!ledState && x > average + sd) {
    ledState = true;
    digitalWrite(LED_BUILTIN, HIGH);

    if (lastBeatTime > 0) {
      unsigned long interval = now - lastBeatTime;

      if (interval >= MIN_RR && interval <= MAX_RR_MS) {
        rrCount++;
        rrSum += interval;
        rrSumSq += (double)interval * interval;

        int bpm = 60000UL / interval;

        Serial.print("RR(ms): ");
        Serial.print(interval);
        Serial.print("  BPM: ");
        Serial.print(bpm);
        Serial.print("  SDRR(ms): ");
        Serial.println(calculateSDRR(), 2);
      }
    }

    lastBeatTime = now;
  }

  else if (ledState && x < average + 0.5f * sd) {
    ledState = false;
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (now - startTime >= WINDOW_MS) {
    Serial.println("----- 5 min result -----");
    Serial.print("RR intervals: ");
    Serial.println(rrCount);
    Serial.print("Final SDRR(ms): ");
    Serial.println(calculateSDRR(), 2);

    while (true);
  }
}