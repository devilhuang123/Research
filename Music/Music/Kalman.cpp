#include "StdAfx.h"
#include "Kalman.h"


Kalman::Kalman(void)
{
	Gyro_y=0;       
	Angle_gy=0;      
	Accel_x=0;	 
	Angle_ax=0;   
	Angle1=0;        
	angle2=0;
	
	Angle=0;         
	value=' ';	

	
	
	Q_bias=0;
	Angle_err=0;
	PCt_0=0;
	PCt_1=0;
	E=0;
	K_0=0;
	K_1=0;
	t_0=0;
	t_1=0;
	


	Q_angle=0.0008;  
	Q_gyro=0.003;
	R_angle=0.5;
	dt=0.01;	                  
	C_0 = 1;
	for(int i=0;i<4;i++)
		Pdot[i] =0;

	PP[0][0]= 1;
	PP[0][1]= 0;
	PP[1][0]= 0;
	PP[1][1]= 1;

}


Kalman::~Kalman(void)
{
}

void Kalman::Kalman_Filter(float Accel,float Gyro)	
{
	Angle+=(Gyro - Q_bias) * dt; //先验估计

	
	Pdot[0]=Q_angle - PP[0][1] - PP[1][0]; // Pk-先验估计误差协方差的微分

	Pdot[1]=- PP[1][1];
	Pdot[2]=- PP[1][1];
	Pdot[3]=Q_gyro;
	
	PP[0][0] += Pdot[0] * dt;   // Pk-先验估计误差协方差微分的积分
	PP[0][1] += Pdot[1] * dt;   // =先验估计误差协方差
	PP[1][0] += Pdot[2] * dt;
	PP[1][1] += Pdot[3] * dt;
		
	Angle_err = Accel - Angle;	//zk-先验估计
	
	PCt_0 = C_0 * PP[0][0];
	PCt_1 = C_0 * PP[1][0];
	
	E = R_angle + C_0 * PCt_0;
	
	K_0 = PCt_0 / E;
	K_1 = PCt_1 / E;
	
	t_0 = PCt_0;
	t_1 = C_0 * PP[0][1];

	PP[0][0] -= K_0 * t_0;		 
	PP[0][1] -= K_0 * t_1;
	PP[1][0] -= K_1 * t_0;
	PP[1][1] -= K_1 * t_1;
		
	Angle	+= K_0 * Angle_err;	 
	Q_bias	+= K_1 * Angle_err;	
	Gyro_y   = Gyro - Q_bias;	



}