#define NCELLS 128

#define MAX_RR 600        // enough for 5 min if HR > 30 bpm
#define WINDOW_MS 300000  // 5 minutes

unsigned int signal[NCELLS];
unsigned int counter = 0;
bool bufferFilled = false;
bool ledState = false;

float sum = 0.0f;
float sumSq = 0.0f;
float average = 0.0f;
float variance = 0.0f;
float sd = 0.0f;

// RR / SDRR variables
unsigned long lastBeatTime = 0;
unsigned int rr[MAX_RR];
unsigned int rrCount = 0;

const unsigned int MIN_RR = 300;   // 200 bpm
const unsigned int MAX_RR_MS = 2000; // 30 bpm

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0, INPUT);
  Serial.begin(9600);

  // initialize buffer with first reading
  unsigned int initial = analogRead(A0);
  for (int i = 0; i < NCELLS; i++) {
    signal[i] = initial;
    sum += initial;
    sumSq += (float)initial * initial;
  }
  bufferFilled = true;
}

float calculateSDRR() {
  if (rrCount < 2) return -1.0;

  float mean = 0.0;
  for (int i = 0; i < rrCount; i++) {
    mean += rr[i];
  }
  mean /= rrCount;

  float sse = 0.0;
  for (int i = 0; i < rrCount; i++) {
    float d = rr[i] - mean;
    sse += d * d;
  }

  return sqrt(sse / (rrCount - 1));  // sample SD
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

  // Beat detected: rising threshold crossing
  if (!ledState && x > average + sd) {
    ledState = true;
    digitalWrite(LED_BUILTIN, HIGH);

    if (lastBeatTime > 0) {
      unsigned long interval = now - lastBeatTime;

      if (interval >= MIN_RR && interval <= MAX_RR_MS) {
        if (rrCount < MAX_RR) {
          rr[rrCount] = interval;
          rrCount++;
        }

        int bpm = 60000UL / interval;

        Serial.print("RR(ms): ");
        Serial.print(interval);
        Serial.print("  BPM: ");
        Serial.print(bpm);
        Serial.print("  Beats: ");
        Serial.println(rrCount);
      }
    }

    lastBeatTime = now;
  }

  // Re-arm detector
  else if (ledState && x < average + 0.5f * sd) {
    ledState = false;
    digitalWrite(LED_BUILTIN, LOW);
  }

  // After 5 minutes, calculate SDRR
  if (now >= WINDOW_MS) {
    float sdrr = calculateSDRR();

    Serial.println("----- 5 min result -----");
    Serial.print("Number of RR intervals: ");
    Serial.println(rrCount);

    Serial.print("SDRR(ms): ");
    Serial.println(sdrr, 2);

    while (true); // stop after 5 minutes
  }
}