#define NCELLS 512

unsigned int signal[NCELLS];
unsigned int counter = 0;
double average = 0.;
double varce = 0.;
double sd = 0.;
double aux = 0.;
double olderr = 0.;
double newerr = 0.;
unsigned int old = 0;
unsigned int x = 0;
unsigned int index = 0;
bool flag = true;

void setup() {
  // put your setup code here, to run once:
pinMode(LED_BUILTIN, OUTPUT);
pinMode(A0, INPUT);
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
x = analogRead(A0);
old = signal[counter];
average += ((double)x - (double)old) / NCELLS;
olderr = (old-average);
newerr = (x - average);
varce = varce - olderr * olderr / NCELLS + newerr * newerr / NCELLS;
sd = sqrt(varce);
signal[counter] = x;
counter = (counter + 1) % NCELLS;

if (x > average +  sd && flag)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    flag = false;
  }
  else if (x < average + sd)
  {
    digitalWrite(LED_BUILTIN, LOW);
    flag = true;
  }
  Serial.println(average);
}
