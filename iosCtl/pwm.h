
int pwm_export(int channel_num);
int pwm_write_period(int channel_num, int period);
int pwm_write_duty_cycle(int channel_num, int duty_cycle);
int pwm_read_duty_cycle(int channel_num);
int pwm_write_polarity(int channel_num, char *polarity);
int pwm_write_enable(int channel_num, int enable);
int pwm_unexport(int channel_num);