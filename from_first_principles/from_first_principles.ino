/*
  SDNN (HRV) from a pulse-like analog signal on A0
  - Beat detection: threshold crossing (rising edge) + re-arm when below threshold
  - Computes SDNN over a rolling window of last N NN intervals

  Wiring:
  - Pulse sensor output -> A0
  - GND -> GND
  - VCC -> 5V (or as required by your sensor)

  Notes:
  - This is a simple threshold detector. For best HRV accuracy, use a proper peak detector + filtering.
*/

const int SIGNAL_PIN = A0;

const int N = 60;                     // window size: last 60 NN intervals
unsigned int nn[N];                   // NN intervals in ms (0..65535)
int nnCount = 0;                      // how many valid intervals we have (<= N)
int nnIndex = 0;                      // circular buffer index

// Beat detection parameters
bool armed = true;                    // "flag": ready to detect next rising crossing
double signalavg = 512;                  // signal level average as a global variable
double signalold = 512;                  // old signal average;
int x = 0;                            // signal level as global variables;
int y = 0;
int z = 0;
int t = 0;
int w = 0;
int sample_count = 0;
unsigned long now = 0;

double cutoff = 800;                     // cutoff as global variable;
double signal_sd = 200;                  // signal standard deviation as global variable;

// Timing
unsigned long prevBeatTime = 0;

// Outlier / sanity limits (tune as needed)
const unsigned int MIN_IBI = 300;      // 300 ms = 200 bpm (reject faster)
const unsigned int MAX_IBI = 2000;     // 2000 ms = 30 bpm (reject slower)

// Recompute threshold occasionally
unsigned long lastThresholdUpdate = 0;
const unsigned long THRESHOLD_UPDATE_MS = 5000;  // every 5 seconds
const int RANGE_SAMPLES = 2000;       // samples used to estimate xmin/xmax

void computeThreshold() {
  int xmin = 32767;
  int xmax = 0;
  for (int i = 0; i < RANGE_SAMPLES; i++) {
    int x = analogRead(SIGNAL_PIN);
    if (x < xmin) xmin = x;
    if (x > xmax) xmax = x;
  }
  // 80% up from xmin toward xmax (like your code)
  cutoff = xmin + (int)((xmax - xmin) * 0.8);
}

void addNN(unsigned int ibi) {
  nn[nnIndex] = ibi;
  nnIndex = (nnIndex + 1) % N;
  if (nnCount < N) nnCount++;
}

// SDNN = sample standard deviation of NN intervals in the current window
// returns SDNN in ms (float). If not enough data, returns -1.
float sdnn_ms() {
  if (nnCount < 2) return -1.0;

  // Compute mean
  unsigned long sum = 0;
  for (int i = 0; i < nnCount; i++) sum += nn[i];
  float mean = (float)sum / (float)nnCount;

  // Compute sample variance
  double sse = 0.0;
  for (int i = 0; i < nnCount; i++) {
    double d = (double)nn[i] - (double)mean;
    sse += d * d;
  }
  double var = sse / (double)(nnCount - 1);

  // SD
  return (float)sqrt(var);
}

int median(int a, int b, int c) {
  if (a > b) {
    int temp = a;
    a = b;
    b = temp;
  }

  if (b > c) {
    int temp = b;
    b = c;
    c = temp;
  }

  if (a > b) {
    int temp = a;
    a = b;
    b = temp;
  }

  return b;
}

int median5(int a, int b, int c, int d, int e) {

  if (a > b) {
    int temp = a;
    a = b;
    b = temp;
  }

  if (b > c) {
    int temp = b;
    b = c;
    c = temp;
  }

  if (c > d) {
    int temp = c;
    c = d;
    d = temp;
  }

  if (d > e) {
    int temp = d;
    d = e;
    e = temp;
  }

  if (a > b) {
    int temp = a;
    a = b;
    b = temp;
  }

  if (b > c) {
    int temp = b;
    b = c;
    c = temp;
  }

  if (c > d) {
    int temp = c;
    c = d;
    d = temp;
  }

  if (a > b) {
    int temp = a;
    a = b;
    b = temp;
  }

  if (b > c) {
    int temp = b;
    b = c;
    c = temp;
  }

  if (a > b) {
    int temp = a;
    a = b;
    b = temp;
  }

  return c;
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);

  computeThreshold();
  lastThresholdUpdate = millis();
  prevBeatTime = millis();

  now = millis();
}

void loop() {
  // // Periodically refresh threshold so it adapts to drift
  // if (now - lastThresholdUpdate >= THRESHOLD_UPDATE_MS) {
  //   computeThreshold();
  //   lastThresholdUpdate = now;
  // }
  w = t;
  t = z;
  z = y;
  y = x;
  x = analogRead(SIGNAL_PIN);
  sample_count++;
  if (sample_count % 5000 == 0) {
    Serial.print(sample_count);
    Serial.print(" samples in ");
    Serial.print(millis()-now);
    Serial.println(" ms.");
  }

//   // Rising crossing detection (edge-triggered)
//   if (armed && x > cutoff) {
//     unsigned long t = millis();
//     unsigned long interval = t - prevBeatTime;
//     prevBeatTime = t;

//     // re-arm only after dropping below cutoff
//     armed = false;
//     digitalWrite(LED_BUILTIN, HIGH);

//     // Outlier rejection + protect against interval==0
//     if (interval >= MIN_IBI && interval <= MAX_IBI) {
//       addNN((unsigned int)interval);

//       // BPM from IBI (ms)
//       unsigned int bpm = (unsigned int)(60000UL / interval);

//       float sdnn = sdnn_ms();

//       Serial.print("IBI(ms)=");
//       Serial.print(interval);
//       Serial.print("  BPM=");
//       Serial.print(bpm);

//       Serial.print("  SDNN(ms)=");
//       if (sdnn < 0) Serial.println("NA");
//       else Serial.println(sdnn, 2);
//     } else {
//       // rejected interval (noise / missed beat)
//       Serial.print("Rejected IBI(ms)=");
//       Serial.println(interval);
//     }
//   }
//   else if (x <= cutoff) {
//     // Only re-arm when signal goes below threshold
//     armed = true;
//     digitalWrite(LED_BUILTIN, LOW);
//   }

//   // (Optional) small delay to reduce serial spam / CPU load
//   // delay(1);

//   // Records "old" signal value:
//   signalold = x;
}