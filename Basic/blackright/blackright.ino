#include <QTRSensors.h>
#include<SoftwareSerial.h>

#define Kp 0.33 // experiment to determine this, start by something small that just makes your bot follow the line at a slow speed
#define Kd 2.47// experiment to determine this, slowly increase the speeds and adjust this value. ( Note: Kp0.06 < Kd1.97) 
#define Ki 0
#define d 150
#define rightMaxSpeed 250   // max speed of the robot
#define leftMaxSpeed 250    // max speed of the robot
#define rightBaseSpeed 175  // this is the speed at which the motors should spin when the robot is perfectly on the line
#define leftBaseSpeed 175   // this is the speed at which the motors should spin when the robot is perfectly on the line

#define NUM_SENSORS  8      // number of sensors used
#define TIMEOUT       4   // waits for 2500 us for sensor outputs to go low
#define EMITTER_PIN   QTR_NO_EMITTER_PIN    // emitter is not controlled
#define turnslowspeed  150
#define turnspeed     175

#define A1 7
#define A2 8
#define AE 10

#define B1 5
#define B2 6
#define BE 9

#define STBY 4

SoftwareSerial mySerial(12, 11); // RX, TX
QTRSensorsAnalog qtra((unsigned char[]) {  0, 1, 2, 3, 4, 5, 6, 7} ,NUM_SENSORS, TIMEOUT, EMITTER_PIN);

int f2=0;
int lastError;
int t=0;
int f=0;
int x=0;
//int t1=0,t2=0;
unsigned int sensors[8], lsavg[8]={100, 100, 100, 100, 100, 100, 100, 100},hsavg[8]={700, 700, 700, 700, 700, 700, 700, 700};
int sum=0;
int n=0;
char path[100] , path_index = 0, ind;
int position, error;
bool isDryRunComplete = false;

long int t1=0, t2;
void setup() {
  mySerial.begin(38400);  
  //motor driver pins
  pinMode(A1,OUTPUT);
  pinMode(A2,OUTPUT);
  pinMode(B1,OUTPUT);
  pinMode(B2,OUTPUT);
  pinMode(AE,OUTPUT);
  pinMode(BE,OUTPUT);
  pinMode(STBY,OUTPUT);

  //LED and Switch pins
  pinMode(13,OUTPUT);
  pinMode(3,INPUT);
  
  //Making the motor to move forward
  digitalWrite(STBY,HIGH);
  digitalWrite(A1,HIGH);
  digitalWrite(A2,LOW);
  digitalWrite(B1,HIGH);
  digitalWrite(B2,LOW);
  
  // callibrating the sensors
  for (int i = 0; i < 300; i++){
    qtra.calibrate();
    delay(20);
  }
  
  // Notifying the end of callibration through led
  digitalWrite(13,HIGH);

  //Printing the callibrated values in serial monitor
  for (int i = 0; i < NUM_SENSORS; i++){
    mySerial.print(qtra.calibratedMinimumOn[i]);
    mySerial.print(' ');
    //savg[i]=(qtra.calibratedMinimumOn[i]+qtra.calibratedMaximumOn[i])/2;
  }
  mySerial.println();
  for (int i = 0; i < NUM_SENSORS; i++){
    mySerial.print(qtra.calibratedMaximumOn[i]);
    mySerial.print(' ');
  }
  mySerial.println();
}

void loop(){
  
  //Detecting switch press
  if(f == 0){  
    analogWrite(AE,0);
    analogWrite(BE,0);
    sum=0; 
  }
  if(digitalRead(3)==0 && f==0){ 
    f=1;
    delay(500);
  }
  else if(digitalRead(3)==0 && f==1){
    f=0;
    delay(500);
  }
  
  // Start line following after the switch is pressed
  if(f==1){
    if(isDryRunComplete == false){
      follow();
      isDryRunComplete = true;
      f=0;
      mySerial.println(path);
      mySerial.println(simplify_path(path, 'R'));
    }
    else if(isDryRunComplete == true){
      digitalWrite(STBY, HIGH);
      solve();
    }
  }
}

void dry_180(){
  //called when all sensors are out of track
  //robot moves forward for 200ms 

  qtra.readLine(sensors);
  //and takes a right 90 turn
  while(!(sensors[0]>=hsavg[0]&&sensors[1]>=hsavg[1] )){ 
    //checking rightmost sensors reach black again
    qtra.readLine(sensors);
    analogWrite(AE, 160);
    analogWrite(BE, 160);
    digitalWrite(A1,LOW);
    digitalWrite(A2,HIGH);
    digitalWrite(B1,HIGH);
    digitalWrite(B2,LOW);
  }
  qtra.readLine(sensors);
  
  //and takes a right 90 turn
  while(!(sensors[5]>=hsavg[5] && sensors[4]>=hsavg[4])){ 
    //checking rightmost sensors reach black again
    qtra.readLine(sensors);
    analogWrite(AE, 130);
    analogWrite(BE, 130);
    digitalWrite(A1,LOW);
    digitalWrite(A2,HIGH);
    digitalWrite(B1,HIGH);
    digitalWrite(B2,LOW);
  }
  
  //stop the robot after that and change the pins to forward condition
  analogWrite(AE, 0);
  analogWrite(BE, 0);
  digitalWrite(A1,HIGH);
  digitalWrite(A2,LOW);
  digitalWrite(B1,HIGH);
  digitalWrite(B2,LOW);
  delay(200);              
}

void dry_left_90(){
  //called when rightmost sensor is in the track and leftmost is out of track
  //robot moves forward for 200ms 

  qtra.readLine(sensors);
  //and takes a right 90 turn
  while(!(sensors[7]>=hsavg[7])){ 
    //checking rightmost sensors reach black again
    qtra.readLine(sensors);
    analogWrite(AE, turnspeed-10);
    analogWrite(BE, turnspeed-10);
    digitalWrite(A1,HIGH);
    digitalWrite(A2,LOW);
    digitalWrite(B1,LOW);
    digitalWrite(B2,HIGH);
  }
  qtra.readLine(sensors);
  while(!((sensors[5]>=hsavg[5])&&(sensors[4]>=hsavg[4])))
  { 
    //checking rightmost sensors reach black again
    qtra.readLine(sensors);
    analogWrite(AE, 140);
    analogWrite(BE, 140);
    digitalWrite(A1,HIGH);
    digitalWrite(A2,LOW);
    digitalWrite(B1,LOW);
    digitalWrite(B2,HIGH);
  }
  //stop the robot after that and change the pins to forward condition
  analogWrite(AE, 0);
  analogWrite(BE, 0);
  digitalWrite(A1,HIGH);
  digitalWrite(A2,LOW);
  digitalWrite(B1,HIGH);
  digitalWrite(B2,LOW);
  delay(200);              
}

void dry_right_90(){
  //called when rightmost sensor is in the track and leftmost is out of track
  //robot moves forward for 200ms 

  qtra.readLine(sensors);
  //and takes a right 90 turn
  while(!(sensors[0]>=hsavg[0]&&sensors[1]>=hsavg[1] )){ 
    //checking rightmost sensors reach black again
    qtra.readLine(sensors);
    analogWrite(AE, turnspeed);
    analogWrite(BE, turnspeed);
    digitalWrite(A1,LOW);
    digitalWrite(A2,HIGH);
    digitalWrite(B1,HIGH);
    digitalWrite(B2,LOW);
  }
  qtra.readLine(sensors);
  //and takes a right 90 turn
  while(!(sensors[3]>=hsavg[3] && sensors[4]>=hsavg[4])){ 
    //checking rightmost sensors reach black again
    qtra.readLine(sensors);
    analogWrite(AE, turnslowspeed);
    analogWrite(BE, turnslowspeed);
    digitalWrite(A1,LOW);
    digitalWrite(A2,HIGH);
    digitalWrite(B1,HIGH);
    digitalWrite(B2,LOW);
  }
  
  //stop the robot after that and change the pins to forward condition
  analogWrite(AE, 0);
  analogWrite(BE, 0);
  digitalWrite(A1,HIGH);
  digitalWrite(A2,LOW);
  digitalWrite(B1,HIGH);
  digitalWrite(B2,LOW);
  delay(200);              
}

void dry_pid(){
  //PID condition
  lastError=0;
  while(1){

    position = qtra.readLine(sensors); // get calibrated readings along with the line position, refer to the QTR Sensors Arduino Library for more details on line position.
    error = position-3500;
    // mySerial.println(er);
    sum+=error;
    if(error<250&&error>-250){
      sum=0;
      f2=0;
    }
    int motorSpeed = Kp * error + Kd * (error - lastError)+Ki*sum;
    lastError = error;
    int rightMotorSpeed = rightBaseSpeed + motorSpeed;
    int leftMotorSpeed = leftBaseSpeed - motorSpeed;
    if(rightMotorSpeed>250)
      rightMotorSpeed=250;
  
    else if(rightMotorSpeed<=0)
      rightMotorSpeed=0; 
  
    if(leftMotorSpeed>250)
      leftMotorSpeed=250;
  
    else if(leftMotorSpeed<=0)
      leftMotorSpeed=0; 

    analogWrite(AE,rightMotorSpeed);
    analogWrite(BE,leftMotorSpeed); 
    if(sensors[0]>hsavg[0]||sensors[7]>lsavg[7]){
      return;   
    }
    if (sensors[0]<lsavg[0]&&sensors[1]<lsavg[1]&&sensors[2]<lsavg[2]&&sensors[3]<lsavg[3]&&sensors[4]<lsavg[4]&&sensors[5]<lsavg[5]&&sensors[6]<lsavg[6]&&sensors[7]<lsavg[7]) {        
      return; 
    }
  }
}

void actual_180(){
  //called when all sensors are out of track
  //robot moves forward for 200ms 
  analogWrite(AE, 150);
  analogWrite(BE, 150);
  digitalWrite(A1,HIGH);
  digitalWrite(A2,LOW);
  digitalWrite(B1,HIGH);
  digitalWrite(B2,LOW);

  delay(50);
  qtra.readLine(sensors);
 
  analogWrite(AE, 0);
  analogWrite(BE, 0);
  delay(80);
  //and takes a right 180 turn
  qtra.readLine(sensors);
  //and takes a right 90 turn
  
  while(!(sensors[0]>=hsavg[0] ))
  { 
    //checking rightmost sensors reach black again
    qtra.readLine(sensors);
    analogWrite(AE, 160);
    analogWrite(BE, 160);
    digitalWrite(B1,HIGH);
    digitalWrite(B2,LOW);
    digitalWrite(A1,LOW);
    digitalWrite(A2,HIGH);
  }
qtra.readLine(sensors);

  while(!(sensors[2]>=hsavg[2] && sensors[1]>=hsavg[1]))
  { 
    //checking rightmost sensors reach black again
    qtra.readLine(sensors);
    analogWrite(AE, 130);
    analogWrite(BE, 130);
    digitalWrite(B1,HIGH);
    digitalWrite(B2,LOW);
    digitalWrite(A1,LOW);
    digitalWrite(A2,HIGH);
  }

  //stop the robot after that and change the pins to forward condition
  analogWrite(AE, 0);
  analogWrite(BE, 0);
  digitalWrite(A1,HIGH);
  digitalWrite(A2,LOW);
  digitalWrite(B1,HIGH);
  digitalWrite(B2,LOW);
  delay(200);      
}

void actual_left_90(){
  //called when rightmost sensor is in the track and leftmost is out of track
  
  //robot moves forward for 200ms 

  qtra.readLine(sensors);
  //and takes a right 90 turn
  while(!(sensors[7]>=hsavg[7])){ 
    //checking rightmost sensors reach black again
    qtra.readLine(sensors);
    analogWrite(AE, turnspeed);
    analogWrite(BE, turnspeed);
    digitalWrite(A1,HIGH);
    digitalWrite(A2,LOW);
    digitalWrite(B1,LOW);
    digitalWrite(B2,HIGH);
  }
  qtra.readLine(sensors);
  while(!((sensors[3]>=hsavg[3])&&(sensors[4]>=hsavg[4])))
  { 
    //checking rightmost sensors reach black again
    qtra.readLine(sensors);
    analogWrite(AE, 130);
    analogWrite(BE, 130);
    digitalWrite(A1,HIGH);
    digitalWrite(A2,LOW);
    digitalWrite(B1,LOW);
    digitalWrite(B2,HIGH);
  }
 
  //stop the robot after that and change the pins to forward condition
  analogWrite(AE, 0);
  analogWrite(BE, 0);
  digitalWrite(A1,HIGH);
  digitalWrite(A2,LOW);
  digitalWrite(B1,HIGH);
  digitalWrite(B2,LOW);
  delay(200);              
}

void actual_right_90(){
  //called when rightmost sensor is in the track and leftmost is out of track
  //robot moves forward for 200ms 

  qtra.readLine(sensors);
  //and takes a right 90 turn
  while(!(sensors[0]>=hsavg[0]&&sensors[1]>=hsavg[1] )){ 
    //checking rightmost sensors reach black again
    qtra.readLine(sensors);
    analogWrite(AE, turnspeed);
    analogWrite(BE, turnspeed);
    digitalWrite(A1,LOW);
    digitalWrite(A2,HIGH);
    digitalWrite(B1,HIGH);
    digitalWrite(B2,LOW);
  }
  qtra.readLine(sensors);
  //and takes a right 90 turn
  while(!(sensors[3]>=hsavg[3] && sensors[4]>=hsavg[4])){ 
    //checking rightmost sensors reach black again
    qtra.readLine(sensors);
    analogWrite(AE, turnslowspeed);
    analogWrite(BE, turnslowspeed);
    digitalWrite(A1,LOW);
    digitalWrite(A2,HIGH);
    digitalWrite(B1,HIGH);
    digitalWrite(B2,LOW);
  }
  
  //stop the robot after that and change the pins to forward condition
  analogWrite(AE, 0);
  analogWrite(BE, 0);
  digitalWrite(A1,HIGH);
  digitalWrite(A2,LOW);
  digitalWrite(B1,HIGH);
  digitalWrite(B2,LOW);
  delay(200);              
}

void actual_pid(){
   //PID condition
    /*mySerial.print(++t);
    mySerial.print(" ");*/
    while(1)
    {  
    position = qtra.readLine(sensors); // get calibrated readings along with the line position, refer to the QTR Sensors Arduino Library for more details on line position.
    error = position-3500;
   // mySerial.println(er);
    sum+=error;
    if(error<250&&error>-250)
    {sum=0;
    f2=0;}
    int motorSpeed = Kp * error + Kd * (error - lastError)+Ki*sum;
    lastError = error;
    int rightMotorSpeed = rightBaseSpeed + motorSpeed;
    int leftMotorSpeed = leftBaseSpeed - motorSpeed;
    if(rightMotorSpeed>250)
      rightMotorSpeed=250;
    
    else if(rightMotorSpeed<=0)
      rightMotorSpeed=0; 
    
    if(leftMotorSpeed>250)
      leftMotorSpeed=250;
    
    else if(leftMotorSpeed<=0)
      leftMotorSpeed=0; 
  
    analogWrite(AE,rightMotorSpeed);
    analogWrite(BE,leftMotorSpeed); 
    if(sensors[0]>hsavg[0]||sensors[7]>lsavg[7])
    {
      break;   
    }
   }
}

char select_turn(unsigned char found_left, unsigned char found_straight, unsigned char found_right){
  if(found_right)
    return 'R';
  else if(found_straight)
    return 'S';
  else if(found_left)
    return 'L';
  else
    return 'B';
}

void follow(){
  while(1){
    dry_pid();
    analogWrite(AE, 0);
    analogWrite(BE, 0);
    delay(50);
    unsigned char found_left=0;
    unsigned char found_straight=0;
    unsigned char found_right=0;
    
    // Now read the sensors and check the intersection type.
    position = qtra.readLine(sensors);

    // Check for left and right exits.
    if(sensors[0] > 600)
      found_right = 1;
    if(sensors[7] > 600)
      found_left = 1;

    // Drive straight a bit more - this is enough to line up our
    // wheels with the inte
    analogWrite(AE, 150);
    analogWrite(BE, 150);
    digitalWrite(A1,HIGH);
    digitalWrite(A2,LOW);
    digitalWrite(B1,HIGH);
    digitalWrite(B2,LOW);
    delay(75);
    analogWrite(AE, 0);
    analogWrite(BE, 0);
    delay(50);
    position = qtra.readLine(sensors);
    if(sensors[1] > 600 ||sensors[2] > 600 || sensors[3] >600 || sensors[4] > 600 || sensors[5] > 600 || sensors[6] > 600)
      found_straight = 1;
    unsigned char dir = select_turn(found_left, found_straight, found_right);
    if(dir=='R'){
      mySerial.println("R90");
      path[path_index++] = 'R';
      dry_right_90();
    }
    else if(dir=='B'){
      mySerial.println("180");
      path[path_index++] = 'B';
      dry_180();
    } 
    else if(dir=='L'){
      mySerial.println("L90");
      path[path_index++] = 'L';
      dry_left_90();
    } 
    else if(dir=='S'){
      mySerial.println("S");
      path[path_index++] = 'S';
    }
    if (sensors[0]>hsavg[0]&&sensors[1]>hsavg[1]&&sensors[2]>hsavg[2]&&sensors[3]>hsavg[3]&&sensors[4]>hsavg[4]&&sensors[5]>hsavg[5]&&sensors[6]>hsavg[6]&&sensors[7]>hsavg[7]) {
      pinMode(2, OUTPUT);
      digitalWrite(2,HIGH);
      digitalWrite(STBY,LOW);
      break;
    }
  }
}

int deleteElement(char* arr, int n, int pos){ 
  for (int i = pos; i < n - 1; i++)
    arr[i] = arr[i + 1];
  arr[n-1] = '\0';
  return n - 1;
}
 
int insertElement(char* arr, int n, int pos, int val){
  for (int i = n; i >= pos; i--)
    arr[i] = arr[i - 1];
  arr[pos] = val;
  return n + 1; 
}

char RightHand(char a[]){
  
  if(a[0]=='R' && a[1]=='B' && a[2]=='L'){
    return 'B';
  }
  else if(a[0]=='R' && a[1]=='B' && a[2]=='S'){
    return 'L';
  }
  else if(a[0]=='R' && a[1]=='B' && a[2]=='R'){
    return 'S';
  }
  else if(a[0]=='S' && a[1]=='B' && a[2]=='R'){
    return 'L';
  }
  else if(a[0]=='S' && a[1]=='B' && a[2]=='S'){
    return 'B';
  }
  else if(a[0]=='L' && a[1]=='B' && a[2]=='R'){
    return 'B';
  }
  else{
    return 0; 
  }
}

char LeftHand(char a[]){
  
  if(a[0]=='L' && a[1]=='B' && a[2]=='R'){
    return 'B';
  }
  else if(a[0]=='L' && a[1]=='B' && a[2]=='S'){
    return 'R';
  }
  else if(a[0]=='L' && a[1]=='B' && a[2]=='L'){
    return 'S';
  }
  else if(a[0]=='S' && a[1]=='B' && a[2]=='L'){
    return 'R';
  }
  else if(a[0]=='S' && a[1]=='B' && a[2]=='S'){
    return 'B';
  }
  else if(a[0]=='R' && a[1]=='B' && a[2]=='L'){
    return 'B';
  }
  else{
    return '\0'; 
  }
}

int reduce_path(char *path, int n, int index, char choice){
  //n = strlen(path);
  if(index >=0 && index <=n){
    char splice[] = {path[index], path[index+1], path[index+2]}, ans;
    
    if(choice == 'R')
      ans = RightHand(splice);
    else if(choice == 'L')
      ans = LeftHand(splice);
      
    if(ans!='\0'){
      for(int c =0; c<3; c++)
      n = deleteElement(path, n, index);
      n = insertElement(path, n, index, ans);
    }
  }
  ind++;
}

char* simplify_path(char path[], char choice){

  //SSLLSRRRBRRBRRRBSRRLLR
  //SSLLSRRSRBRRRBSRRLLR  
  //SSLLSRRSSRRBSRRLLR  
  //SSLLSRRSSRLRRLLR  
  int n = strlen(path);
  for(int i = 0; i<n; i++){
    reduce_path(path, n, i, choice);
  } 
  return path;
}

void solve(){
   position = qtra.readLine(sensors); // get calibrated readings along with the line position, refer to the QTR Sensors Arduino Library for more details on line position.
   error = position-3500;
  for(int i=0;i<strlen(path);i++){
    // SECOND MAIN LOOP BODY  
    actual_pid();
    analogWrite(AE, 150);
    analogWrite(BE, 150);
    digitalWrite(A1,HIGH);
    digitalWrite(A2,LOW);
    digitalWrite(B1,HIGH);
    digitalWrite(B2,LOW);
    delay(50);
    analogWrite(AE, 0);
    analogWrite(BE, 0);
    delay(50);
     
    if(path[i]=='R'){
      actual_right_90();
      mySerial.println("R90");
    }
    else  if(path[i]=='L'){
      actual_left_90();
      mySerial.println("L90");
    }
    else  if(path[i]=='S'){
    }
  }
}

