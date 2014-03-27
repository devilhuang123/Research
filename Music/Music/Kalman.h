#pragma once
class Kalman
{

public:


	float Gyro_y;       
	float Angle_gy;      
	float Accel_x;	 
	float Angle_ax;   
	float Angle1;        
	float angle2;
	
	float Angle;         
	char value;	

	float  Q_angle;  
	float  Q_gyro;
	float  R_angle;
	float  dt;	                  
	char   C_0;
	float  Q_bias, Angle_err;
	float  PCt_0, PCt_1, E;
	float  K_0, K_1, t_0, t_1;
	float  Pdot[4];
	float  PP[2][2];
	void Kalman_Filter(float Accel,float Gyro);

	Kalman(void);
	~Kalman(void);
};

