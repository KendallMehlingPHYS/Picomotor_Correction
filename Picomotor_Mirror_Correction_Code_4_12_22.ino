
// This Code is designed to sense misalignments in optics mirrors and apply appropriate corrective steps to Picomotor Mirror Motors. 
// Each Arduino board will control a mirror pair. We will have 4 inputs (Mirror 1/2 X/Y differential voltage from Position Sensors).
// Each Arduion board will also have be able to control each of the 4 Axes (Mirror 1/2 Horizontal/Vertical) for correction. 
// Dana Z. Anderson Laboratories
// Kendall James Mehling
// 2/17/22

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("<Teensy is ready to supply Voltage>");
    
    // Set the digital pins which will supply the Gate bias Voltages.
    pinMode(18, OUTPUT);        //High Side Backward
    pinMode(19, OUTPUT);        //Low Side Backward 
    pinMode(20, OUTPUT);        //High Side Forward
    pinMode(21, OUTPUT);        //Low Side Forward

    // Set the digital pins which will close the relays to select the appropriate motor axis

    pinMode(13, OUTPUT);        //Mirror1 X
    pinMode(14, OUTPUT);        //Mirror1 Y
    pinMode(15, OUTPUT);        //Mirror2 X
    pinMode(16, OUTPUT);        //Mirror2 Y

    pinMode(31, OUTPUT);


    // Set the digital pins which will power the LED's Motor state information
    
    /*pinMode(38, OUTPUT);
    pinMode(39, OUTPUT);
    pinMode(40, OUTPUT);

    */
    // Set all the analog pins to receive data from the mirrors. 
        //Mirror 1  x-diff, y-diff, sum respectively
    pinMode(24, INPUT);
    pinMode(25, INPUT);
    pinMode(26, INPUT);

        //Mirror 2  x-diff, y-diff, sum respectively
    pinMode(38, INPUT);
    pinMode(39, INPUT);
    pinMode(40, INPUT);

    //Gate Signal (tells the microcontroller it is okay to enact motion)
    pinMode(32, INPUT);


    //Lock Signal (tells the microcontroller to read the Photodiode signal pins and lock these as your referenc)
    pinMode(10, INPUT);
    pinMode(11, INPUT);
 
}


int x1 = 24;
int Y1 = 25;
int sum1 = 26;
int x2 = 38;
int y2 = 39;
int sum2 = 40;

int relay_list [4] = {13, 14, 15, 16};
//int pin_list [6] = { x1, y1, sum1, x2, y2, sum2 };

int gate = 1;
int lock = 1;
int lock_reset = 1;
int lock_pin = 10;
int lock_reset_pin = 11;
int pause = 0;


int no_effect = 0;
int step_guess = 40;
int average_length = 100;

// What is the acceptable ADC error for each detector and each dimension (X/Y). This will be compared to the lock vs measured values.
float x1_margin = 5;
float y1_margin = 5;

float x2_margin = 5;
float y2_margin = 5;

float x1_lock = 0;
float y1_lock = 0;
float x2_lock = 0;
float y2_lock = 0;


int gate_pin = 32;
  //This should check the analog pins. If the analog voltage is outside the acceptable error bound, then apply corrective steps to the Mirror Axes. 
  //Note that this should have safeguards so that if the motor turns the wrong way it will not do so indefinitely. This would cause major
  //Mirror Misalignment and could lead to heating of the components which are not designed for continous operation ( > a few seconds).
void loop(){
  
    // We update our reference for both mirrors. lock_reset must be high. This is to prevent changing reference every update sequence
  if(lock >= 1 && lock_reset > 0){
    delay(1000);
    float* avg = get_average(x1, Y1, sum1, x2, y2, sum2, average_length);
    x1_lock = avg[0];
    y1_lock = avg[1];

    x2_lock = avg[3];
    y2_lock = avg[4];
    
    Serial.println("The target positions have been acquired");
    Serial.print("Lock Values: ");
    Serial.print(x1_lock);  
    Serial.print(" , ");
    Serial.print(y1_lock);
    Serial.print(" , ");
    Serial.print(x2_lock);
    Serial.print(" , ");    
    Serial.print(y2_lock);
    lock_reset = 0;
  }
    
    //If the gate and lock are both high then we want to servo to reference. 
    if(gate && lock && !pause){
      Read_Analog_Pins();
    }



  //the lock_reset pin and lock pin will be complementary to allow a "sample and hold" like performance
  if(digitalRead(lock_reset_pin)){
    lock_reset = 1;
  }
  
  //Set the gate to high or low depending on voltage level
//  if(digitalRead(gate_pin)){
//    gate = 1;
//  }
//  else{
//    gate = 0;
//  }
}


int Read_Analog_Pins(){
    /*Purpose: Check mirrors in order to determine if corrective action is needed
    Parameters: None. Will simply step through number of mirrors
    Returns: None. Simple completion C++ integer 0. 
                */
  float* avg = get_average(x1, Y1, sum1, x2, y2, sum2, average_length);
  Serial.print(avg[0]);
  Serial.print(" , ");
  Serial.print(avg[1]);
  Serial.print(" , ");
  Serial.print(avg[2]);
  Serial.println("");
  Serial.print(avg[3]);
  Serial.print(" , ");
  Serial.print(avg[4]);
  Serial.print(" , ");
  Serial.print(avg[5]);
  Serial.println("");
  check_errors(avg);
    return 0;
  }

float* get_average(int x1_pin, int y1_pin1, int sum1_pin, int x2_pin, int y2_pin, int sum2_pin, int average_length){
  /*Purpose: Return the average ADC value from a given detector for a given averaging length
    Parameters: x_pin: The Teensy pin to analogRead for x-information
                y_pin: the Teensy pin to analogRead for y-information
                sum_pin: the Teensy pin to analogRead for z-infomration
    Returns: Averages: an array of the average value for x, y, and sum. 
                */
  static float averages [6];
  int x1_val = 0;
  int y1_val = 0;
  int sum1_val = 0;
  
  int x1_sum = 0;
  int y1_sum = 0;
  int sum1_sum = 0;

  int x2_val = 0;
  int y2_val = 0;
  int sum2_val = 0;
  
  int x2_sum = 0;
  int y2_sum = 0;
  int sum2_sum = 0;

  float x1_avg = 0;
  float y1_avg = 0;
  float sum1_avg = 0;

  float x2_avg = 0;
  float y2_avg = 0;
  float sum2_avg = 0;
   
  for(int j = 1; j <= average_length; j++){
        x1_val = analogRead(x1);
        y1_val = analogRead(Y1);
        sum1_val = analogRead(sum1);
  
        x1_sum += x1_val;
        y1_sum += y1_val;
        sum1_sum += sum1_val;

        x2_val = analogRead(x2);
        y2_val = analogRead(y2);
        sum2_val = analogRead(sum2);
  
        x2_sum += x2_val;
        y2_sum += y2_val;
        sum2_sum += sum2_val;
        
      }
      x1_avg = x1_sum/average_length;
      y1_avg = y1_sum/average_length;
      sum1_avg = sum1_sum/average_length; 
      
      x2_avg = x2_sum/average_length;
      y2_avg = y2_sum/average_length;
      sum2_avg = sum2_sum/average_length; 

      averages[0] = x1_avg;
      averages[1] = y1_avg;
      averages[2] = sum1_avg;

      averages[3] = x2_avg;
      averages[4] = y2_avg;
      averages[5] = sum2_avg;    
      
      return averages;
      
}

int check_errors(float avg[]){
  /*Purpose: Compare the averaged value of the x and y pins to the lock values. If the 
             difference between the average and lock value are greater than accepted margin
             Apply corrective steps to the motors. 
    Parameters: mirror: Which mirror in the optical setup needs to be corrected.
                avg[]:  The average values to be compared to the lock values.
    Returns: None. 
                */
  Serial.println("checking errors");
  digitalWrite(31, LOW);
  float x1_error = avg[0] - x1_lock;
  float y1_error = avg[1] - y1_lock;

  float x2_error = avg[3] - x2_lock;
  float y2_error = avg[4] - y2_lock;
  
  Serial.print("X1_Error  ");
  Serial.println(x1_error);
  Serial.print("Y1_Error  ");
  Serial.println(y1_error);
  Serial.print("X2_Error  ");
  Serial.println(x2_error);
  Serial.print("Y2_Error  ");
  Serial.println(y2_error);  
  
  if(abs(x1_error) > x1_margin){
    Serial.println("X1 must be fixed");
    calculate_step_number(x1_error, x1_margin, 1, 0);
  } 
  else if(abs(y1_error) > y1_margin){
    Serial.println("Y1 must be fixed");
   calculate_step_number(y1_error, y1_margin, 1, 1);
  }

  else if(abs(x2_error) > x2_margin){
    Serial.println("X2 must be fixed");    
    calculate_step_number(x2_error, x2_margin, 2, 0);
  }
  else if(abs(y2_error) > y2_margin){
    Serial.println("Y2 must be fixed");    
    calculate_step_number(y2_error, y2_margin, 2, 1);  
  }
  else{
    Serial.println("All Errors are within margin");
    digitalWrite(31, HIGH);
  }

  
  delay(1000);
  return 0;

}

int move_motor_forward(float error, int steps_needed, int mirror_number, int axis_number){
  int motor_number;

  float lock;
  int error_index;
  float margin;
  float* avg;
  if(mirror_number == 1){
    if(axis_number == 0){
      motor_number = 13;
      error_index = 0;
      lock = x1_lock;
      margin = x1_margin;
    }   
    else if(axis_number ==1){
      motor_number = 14;
      error_index = 1;
      lock = y1_lock;
      margin = y1_margin;
    }
  }
  else if(mirror_number ==2){
    if(axis_number == 0){
      motor_number = 15;
      error_index = 3;
      lock = x2_lock;
      margin = x2_margin;
    }
    else if(axis_number == 1){
      motor_number = 16;     
      error_index = 4;
      lock = y2_lock;
      margin = y2_margin;
    }
  }
  int relay_logic = check_relays();
  if(relay_logic){
    digitalWrite(motor_number, HIGH);
    delay(1);
    for(int step_num = 0; step_num <= steps_needed; step_num++){
        digitalWrite(18, HIGH);
        delayMicroseconds(500);
        digitalWrite(18,LOW);
        delayMicroseconds(10);
        digitalWrite(19, HIGH);
        delayMicroseconds(500);
        digitalWrite(19,LOW);
        digitalWrite(18,LOW);  
      }
    digitalWrite(motor_number, LOW);
    avg = get_average(x1,y1,sum1, x2, y2, sum2, average_length);
    double error_update = avg[error_index] - lock;  
    if(abs(error_update) - abs(error) >= -1){
      no_effect += 1;
      if(no_effect > 5){
        Serial.println("The error increased. Please stop operation");
      }
    }
    else{
      Serial.println(error_update);
      no_effect = 0;
      calculate_step_number(error_update, margin, mirror_number, axis_number);
    
    }
  }
  return 0;
}


int move_motor_backward(float error, int steps_needed, int mirror_number, int axis_number){
  int motor_number;
  float margin;
  float lock;
  int error_index;
  float* avg;
  if(mirror_number == 1){
    if(axis_number == 0){
      motor_number = 13;
      error_index = 0;
      lock = x1_lock;
      margin = x1_margin;
    }
    else if(axis_number == 1){
      motor_number = 14;
      error_index = 1;
      lock = y1_lock;
      margin = y1_margin;
    }
  }
  else if(mirror_number ==2){
    if(axis_number == 0){
      motor_number = 15;
      error_index = 3;
      lock = x2_lock;
      margin = x2_margin;
    }
    else if(axis_number ==1){
      motor_number = 16;     
      error_index = 4;
      lock = y2_lock;
      margin = y2_margin;
    }
  }
  int relay_logic = check_relays();
  if(relay_logic){
    digitalWrite(motor_number, HIGH);
    delay(1);
    for(int step_num = 0; step_num <= steps_needed; step_num++){
        digitalWrite(20, HIGH);
        delayMicroseconds(500);
        digitalWrite(20,LOW);
        delayMicroseconds(10);
        digitalWrite(21, HIGH);
        delayMicroseconds(500);
        digitalWrite(21,LOW);
        digitalWrite(20,LOW);  
      }
    digitalWrite(motor_number, LOW);
    avg = get_average(x1,y1,sum1, x2, y2, sum2, average_length);
    double error_update = avg[error_index] - lock;  
    if(abs(error_update) - abs(error) > 0){
      Serial.println("The error increased. Please stop operation");
    }
    else{
      Serial.println(error_update);
      calculate_step_number(error_update, margin, mirror_number, axis_number);
    }
  }
  return 0;
}

int check_relays(){
  int relay_closed = 0;
  for(int relay_index = 0; relay_index <= 3; relay_index++){
    relay_closed += digitalRead(relay_list[relay_index]);
    
  }
  if(relay_closed >0){
    Serial.println("ERROR, More than one Relay is open");
    for(int j = 0; j<=3; j++){
      digitalWrite(relay_list[j], LOW);
    }
    Serial.println("All relays have been set LOW");
    return 0;
  }
  else{
    return 1;
  }
  
}

int calculate_step_number(double error, double error_margin, int mirror, int axis){
    if(error > error_margin){
      move_motor_forward(error, step_guess, mirror, axis);
    }
    else if(error < -1*error_margin){
      move_motor_backward(error, step_guess, mirror, axis);
    }
    return 1;
     
}
