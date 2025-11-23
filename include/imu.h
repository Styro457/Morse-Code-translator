// Initiate the IMU module using the provided sdk.
void init_imu();

/**
 * Main function for the IMU Task.
 * 
 * Measures the data from the IMU module using the provided sdk and applies noise filtering using Estimated Moving Average.
 * 
 * The gryoscope is used for real-time detection of tilting gestures up, down and left, right.
 * 
 * The accelerometer values are used for detecting the position of the device and playing a sound when the device is held
 * upright or downright for 2 seconds.
 */
void imu_task(void *pvParameters);