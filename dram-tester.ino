/* 
 *  Noel Llopis 2021
 *  
 *  Based on initial code by Chris Osborn <fozztexx@fozztexx.com>
 *  http://insentricity.com/a.cl/252
 */

#define DIN             8
#define DOUT            9
#define CAS             10
#define RAS             11
#define WE              12

#define STATUS    17
#define FAILED    18
#define SUCCESS   19

#define ADDR_BITS 8
#define COLROW_COUNT (1 << ADDR_BITS)

void setup()
{
  pinMode(DIN, OUTPUT);
  pinMode(DOUT, INPUT);

  pinMode(CAS, OUTPUT);
  pinMode(RAS, OUTPUT);
  pinMode(WE, OUTPUT);

  DDRD = 0xff;
  
  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);
  digitalWrite(WE, HIGH);

  pinMode(STATUS, OUTPUT);
  pinMode(FAILED, OUTPUT);
  pinMode(SUCCESS, OUTPUT);

  digitalWrite(STATUS, HIGH);
  digitalWrite(FAILED, LOW);
  digitalWrite(SUCCESS, LOW);
}

void loop()
{
  checkSame(0);
  digitalWrite(STATUS, LOW);
  delay(250);
  digitalWrite(STATUS, HIGH);

  checkSame(1);
  digitalWrite(STATUS, LOW);
  delay(250);
  digitalWrite(STATUS, HIGH);

  checkAlternating(0);
  digitalWrite(STATUS, LOW);
  delay(250);
  digitalWrite(STATUS, HIGH);

  checkAlternating(1);
  digitalWrite(STATUS, LOW);
  delay(250);
  digitalWrite(STATUS, HIGH);

  checkRandom();
  
  digitalWrite(STATUS, LOW);
  digitalWrite(SUCCESS, HIGH);

  while (1) {};
}

static inline void writeToRowCol(int row, int col)
{
  PORTD = row;
  digitalWrite(RAS, LOW);
  PORTD = col;
  digitalWrite(CAS, LOW);

  digitalWrite(WE, LOW);

  digitalWrite(WE, HIGH);
 
  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);
}


static inline int readFromRowCol(int row, int col)
{
  PORTD = row;
  digitalWrite(RAS, LOW);
  PORTD = col;
  digitalWrite(CAS, LOW);

  int val = digitalRead(DOUT);
 
  digitalWrite(CAS, HIGH);
  digitalWrite(RAS, HIGH);
  return val;
}

void fail()
{
  digitalWrite(STATUS, LOW);
  digitalWrite(FAILED, HIGH);

  while (1) {};
}

void checkSame(int val)
{
  digitalWrite(DIN, val);
 
  for (int col=0; col<COLROW_COUNT; col++)
    for (int row=0; row<COLROW_COUNT; row++)
      writeToRowCol(row, col);

  /* Reverse DIN in case DOUT is floating */
  digitalWrite(DIN, !val);
 
  for (int col=0; col<COLROW_COUNT; col++)
    for (int row=0; row<COLROW_COUNT; row++)
      if (readFromRowCol(row, col) != val)
        fail();

  return;
}

void checkAlternating(int start)
{
  int i = start;
  for (int col=0; col<COLROW_COUNT; col++) 
  {
    for (int row=0; row<COLROW_COUNT; row++) 
    {
      digitalWrite(DIN, i);
      i = !i;
      writeToRowCol(row, col);
    }
  }
  
  for (int col=0; col<COLROW_COUNT; col++) 
  {
    for (int row=0; row<COLROW_COUNT; row++) 
    { 
      if (readFromRowCol(row, col) != i)
        fail();
  
      i = !i;
    }
  }
  
  return;
}


#define randomSize (1 << ADDR_BITS*2) / 8

void checkRandom()
{
  // Generate a somewhat random seed
  const int seed = analogRead(16);

  randomSeed(seed);
  for (int col=0; col<COLROW_COUNT; col++) 
  {
    for (int row=0; row<COLROW_COUNT; row++) 
    {
      const int value = (int)random(2);    
      digitalWrite(DIN, value);
      writeToRowCol(row, col);
    }
  }

  // Set the same seed as for the write to know what value to get
  randomSeed(seed);
  for (int col=0; col<COLROW_COUNT; col++) 
  {
    for (int row=0; row<COLROW_COUNT; row++) 
    {
      const int value = (int)random(2);    
      if (readFromRowCol(row, col) != value)
        fail();
    }
  }
  
  return;
}
